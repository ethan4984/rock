#include <drivers/nvme/nvme.hpp>
#include <sched/smp.hpp>
#include <int/apic.hpp>
#include <acpi/madt.hpp>
#include <mm/vmm.hpp>

namespace nvme {

device::device(pci::device pci_device) : pci_device(pci_device), qid_cnt(0) {
    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 0);

    registers = reinterpret_cast<volatile bar_regs*>(bar.base + vmm::high_vma);

    auto major_version = (registers->vs >> 16) & 0xffff;
    auto minor_version = (registers->vs >> 8) & 0xff;
    auto tertiary_version = registers->vs & 0xff;

    print("[NVME] Version Detected {}:{}:{}\n", major_version, minor_version, tertiary_version);

    switch(registers->vs) {
        case version_1_0_0_id:
            break;
        case version_1_1_0_id:
            break;
        case version_1_2_0_id:
            break;
        case version_1_2_1_id:
            break;
        case version_1_3_0_id:
            break;
        case version_1_4_0_id:
            break;
        default:
            print("[NVME] Invalid Controller Version\n");
            return;
    }

    if(!(registers->cap & (1ull << 37))) {
        print("[NVME] NVME Command Set Not Supported\n");
        return;
    }

    if(!(registers->cap & (1ull << 44))) {
        print("[NVME] I/O Command Set Not Supprted\n");
        return;
    }

    max_page_size = pow(2, 12 + (registers->cap >> 52 & 0b1111));
    min_page_size = pow(2, 12 + (registers->cap >> 48 & 0b1111));
    page_size = min_page_size;

    uint32_t cc = registers->cc;

    if(registers->cc & (1 << 0)) {
        registers->cc = registers->cc & ~(1 << 0); // disabled controller
    }

    while((registers->csts & (1 << 0))) { }

    queue_entries = registers->cap & 0xffff; // max queue entries
    strides = (registers->cap >> 32) & 0xf; // max strides

    admin_queue = queue(queue_entries, qid_cnt++, registers, strides);

    registers->aqa = (queue_entries - 1) << 16 | (queue_entries - 1);
    registers->asq = reinterpret_cast<size_t>(admin_queue.submission_queue) - vmm::high_vma;
    registers->acq = reinterpret_cast<size_t>(admin_queue.completion_queue) - vmm::high_vma;

    cc |= (0 << 4) | // nvme command set
          (0 << 11) | // ams = round robin
          (0 << 14) | // no notifications
          (6 << 16) | // io submission queue size 16 bytes 
          (4 << 20) | // io completion queue size 64 bytes
          (1 << 0); // enable

    registers->cc = cc;

    for(;;) {
        if(registers->csts & (1 << 0)) { // CSDT.RDY
            break;
        } else if(registers->csts & (1 << 1)) {  // CSDT.CFS
            print("[NVME] Controller Fatal Status\n");
            return;
        }
    }

    print("[NVME] Controller Restarted\n");

    ctrl_id = reinterpret_cast<controller_id*>(pmm::alloc(align_up(sizeof(controller_id), vmm::page_size)) + vmm::high_vma);
    ns_id = reinterpret_cast<namespace_id*>(pmm::alloc(1) + vmm::high_vma);

    if(get_ctrl_id() == -1) {
        print("[NVME] Fatal Command Error\n");
        return;
    }

    print("[NVME] PCI vendor ID and subsystem vendor ID: {}/{}\n", (unsigned)ctrl_id->vid, (unsigned)ctrl_id->ssvid);
    print("[NVME] Model Number: {}\n", lib::string((const char*)ctrl_id->mn));
    print("[NVME] Serial Number: {}\n", lib::string((const char*)ctrl_id->sn));
    print("[NVME] Firmware Version: {}\n", lib::string((const char*)ctrl_id->fr));
    print("[NVME] IEEE OUI Identifier: {}:{}:{}\n", (unsigned)ctrl_id->ieee[0], (unsigned)ctrl_id->ieee[1], (unsigned)ctrl_id->ieee[2]);
    print("[NVME] Namespace max: {}\n", (unsigned)ctrl_id->nn);

    lib::vector<uint32_t> nsid_list = get_nsid_list();

    for(size_t i = 0; i < nsid_list.size(); i++) {
        if(get_namespace_id(nsid_list[i]) == -1) {
            print("[NVME] Fatal Command Error\n");
            return;
        }

        namespace_list.push(ns(*ns_id));

        print("[NVME] Namespace ID {}\n", nsid_list[i]);
        print("[NVME] \tLBA cnt: {x}\n", namespace_list[i].lba_cnt);
        print("[NVME] \tLBA size: {x}\n", namespace_list[i].lba_size);
    }

    auto status = [&](size_t queue_cnt) {
        command resize_command; 
        memset8(reinterpret_cast<uint8_t*>(&resize_command), 0, sizeof(command));

        resize_command.fet.opcode = opcode_set_features;
        resize_command.fet.fid = 0x7; // set size
        resize_command.fet.dword11 = (queue_cnt - 1) | ((queue_cnt - 1) << 16);

        return admin_queue.send_cmd(resize_command);
    } (smp::cpus.size() * 2);

    if(status) {
        print("[NVME] Failed command\n");
        return;
    }

    for(size_t i = 0; i < smp::cpus.size(); i++) {
        smp::cpus[i].nvme_io_queue = new queue;
        *smp::cpus[i].nvme_io_queue = queue(queue_entries, qid_cnt++, registers, strides);

        create_io_queue(*smp::cpus[i].nvme_io_queue);
    }
}

int device::get_ctrl_id() { 
    command new_command; 
    memset8(reinterpret_cast<uint8_t*>(&new_command), 0, sizeof(command));

    new_command.ident.opcode = opcode_identify;
    new_command.ident.cns = 1;
    new_command.ident.prp1 = reinterpret_cast<size_t>(ctrl_id) - vmm::high_vma;
    
    uint16_t status = admin_queue.send_cmd(new_command);
    if(status)
        return -1;

    return 0;
}

int device::get_namespace_id(int num) {
    command new_command;
    memset8(reinterpret_cast<uint8_t*>(&new_command), 0, sizeof(command));

    new_command.ident.opcode = opcode_identify;
    new_command.ident.nsid = num;
    new_command.ident.cns = 0;
    new_command.ident.prp1 = reinterpret_cast<size_t>(ns_id) - vmm::high_vma;
    new_command.ident.prp2 = 0;

    uint16_t status = admin_queue.send_cmd(new_command);
    if(status)
        return -1;

    return 0;
}

lib::vector<uint32_t> device::get_nsid_list() {
    command new_command;
    memset8(reinterpret_cast<uint8_t*>(&new_command), 0, sizeof(command));

    uint32_t *namespace_list = reinterpret_cast<uint32_t*>(pmm::alloc(align_up(ctrl_id->nn * sizeof(uint32_t), 0x1000)) + vmm::high_vma);

    new_command.ident.opcode = opcode_identify;
    new_command.ident.cns = 2;
    new_command.ident.prp1 = reinterpret_cast<uint64_t>(namespace_list) - vmm::high_vma;

    admin_queue.send_cmd(new_command);

    lib::vector<uint32_t> ret; 

    for(size_t i = 0; i < ctrl_id->nn; i++) {
        if(namespace_list[i])
            ret.push(namespace_list[i]);
    }

    pmm::free(reinterpret_cast<size_t>(namespace_list) - vmm::high_vma, align_up(ctrl_id->nn * sizeof(uint32_t), 0x1000));

    return ret;
}

int device::create_io_queue(queue new_queue) {
    command create_cq_command;
    memset8(reinterpret_cast<uint8_t*>(&create_cq_command), 0, sizeof(command));

    create_cq_command.create_cq.opcode = opcode_create_cq;
    create_cq_command.create_cq.prp1 = reinterpret_cast<size_t>(new_queue.completion_queue) - vmm::high_vma;
    create_cq_command.create_cq.cqid = new_queue.qid;
    create_cq_command.create_cq.qsize = new_queue.entry_cnt - 1;
    create_cq_command.create_cq.cq_flags = (1 << 0);

    uint16_t status = admin_queue.send_cmd(create_cq_command);
    if(status)
        return -1;

    command create_sq_command;
    memset8(reinterpret_cast<uint8_t*>(&create_sq_command), 0, sizeof(command));

    create_sq_command.create_sq.opcode = opcode_create_sq;
    create_sq_command.create_sq.prp1 = reinterpret_cast<size_t>(new_queue.submission_queue) - vmm::high_vma;
    create_sq_command.create_sq.sqid = new_queue.qid;
    create_sq_command.create_sq.cqid = new_queue.qid;
    create_sq_command.create_sq.qsize = new_queue.entry_cnt - 1;
    create_sq_command.create_sq.sq_flags = (1 << 0) | (2 << 1);

    status = admin_queue.send_cmd(create_sq_command);
    if(status)
        return -1;

    return 0;
}

queue::queue(size_t entry_cnt, size_t qid, volatile bar_regs *registers, size_t strides) :  registers(registers),
                                                                                            entry_cnt(entry_cnt),
                                                                                            qid(qid),
                                                                                            strides(strides),
                                                                                            sq_head(0),
                                                                                            sq_tail(0),
                                                                                            cq_head(0),
                                                                                            cq_tail(0),
                                                                                            phase(1),
                                                                                            command_id(0) { 
    submission_queue = reinterpret_cast<command*>(pmm::calloc(div_roundup(entry_cnt * sizeof(command), vmm::page_size)) + vmm::high_vma);
    completion_queue = reinterpret_cast<completion*>(pmm::calloc(div_roundup(entry_cnt * sizeof(completion), vmm::page_size)) + vmm::high_vma);
    submission_doorbell = reinterpret_cast<uint32_t*>(reinterpret_cast<size_t>(registers) + vmm::page_size + (2 * qid * (4 << strides)));
    completion_doorbell = reinterpret_cast<uint32_t*>(reinterpret_cast<size_t>(registers) + vmm::page_size + ((2 * qid + 1) * (4 << strides)));
}

uint16_t queue::send_cmd(command cmd) {
    submission_queue[sq_tail] = cmd;
    if(++sq_tail == entry_cnt)
        sq_tail = 0;

    *(submission_doorbell) = sq_tail;

    for(;;) {
        if((completion_queue[cq_head].status & 0x1) == phase)
            break;
    }

    if((completion_queue[cq_head].status >> 1)) {
        print("[NVME] Command Error: Status {x}\n", (uint16_t)completion_queue[cq_head].status);
        return completion_queue[cq_head].status;
    }

    if(++cq_head == entry_cnt) {
        cq_head = 0;
        phase = !phase;
    }

    *(completion_doorbell) = cq_head;

    return 0;
}

ns::ns(namespace_id ns_id) : ns_id(ns_id) {
    lba_cnt = ns_id.nsze;
    lba_size = 1 << ns_id.lbaf_list[(unsigned)ns_id.flbas & 0b11111].ds;
}

}
