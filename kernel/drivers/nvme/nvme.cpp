#include <drivers/nvme/nvme.hpp>
#include <sched/smp.hpp>
#include <int/apic.hpp>
#include <acpi/madt.hpp>
#include <mm/vmm.hpp>
#include <fs/dev.hpp>
#include <map.hpp>
#include <memutils.hpp>
#include <int/idt.hpp>
#include <utility>

namespace nvme {

ssize_t msd::raw_read(size_t off, size_t cnt, void *buf) {
    smp::cpu *local = smp::core_local();
    controller *cntl = local->nvme_io_queue->parent_controller;

    ns *active_namespace = cntl->namespace_list[partition_index];

    auto alignment = off & (active_namespace->lba_size - 1);

    size_t lba_start = off / active_namespace->lba_size;
    size_t lba_cnt = div_roundup(cnt, active_namespace->lba_size) + (((alignment + cnt) > active_namespace->lba_size) ? 1 : 0);

    uint8_t *lba_buffer = reinterpret_cast<uint8_t*>(pmm::alloc(div_roundup(lba_cnt * active_namespace->lba_size, vmm::page_size)) + vmm::high_vma); 

    active_namespace->rw_lba(lba_buffer, lba_start, lba_cnt, 0);
    size_t lba_offset = off % active_namespace->lba_size;
    memcpy8(reinterpret_cast<uint8_t*>(buf), lba_buffer + lba_offset, cnt);

    pmm::free(reinterpret_cast<size_t>(lba_buffer) - vmm::high_vma, div_roundup(lba_cnt * active_namespace->lba_size, vmm::page_size));

    return cnt;
}

ssize_t msd::raw_write(size_t off, size_t cnt, void *buf) {
    smp::cpu *local = smp::core_local();
    controller *cntl = local->nvme_io_queue->parent_controller;

    ns *active_namespace = cntl->namespace_list[partition_index];

    auto alignment = off & (active_namespace->lba_size - 1);

    size_t lba_start = off / active_namespace->lba_size;
    size_t lba_cnt = div_roundup(cnt, active_namespace->lba_size) + (((alignment + cnt) > active_namespace->lba_size) ? 1 : 0);

    uint8_t *lba_buffer = reinterpret_cast<uint8_t*>(pmm::alloc(div_roundup(lba_cnt * active_namespace->lba_size, vmm::page_size)) + vmm::high_vma); 

    active_namespace->rw_lba(lba_buffer, lba_start, lba_cnt, 0);
    size_t lba_offset = off % active_namespace->lba_size;
    memcpy8(lba_buffer + lba_offset, reinterpret_cast<uint8_t*>(buf), cnt);
    active_namespace->rw_lba(lba_buffer, lba_start, lba_cnt, 1);
    pmm::free(reinterpret_cast<size_t>(lba_buffer) - vmm::high_vma, div_roundup(lba_cnt * active_namespace->lba_size, vmm::page_size));

    return cnt;
}

int ns::rw_lba(void *buf, size_t start, size_t cnt, bool rw) {
    cmd new_command {};

    new_command.opcode = rw ? 1 : 2;

    smp::cpu *local = smp::core_local();

    new_command.cid = local->nvme_io_queue->cid_bitmap.alloc();
    if(new_command.cid == 0xffff)
        return -1;

    new_command.rw.prp1 = (size_t)buf - vmm::high_vma; 

    uint64_t *prp_list = (uint64_t*)(pmm::calloc(div_roundup(max_prps * parent_controller->queue_entries * sizeof(uint64_t), vmm::page_size)) + vmm::high_vma);

    if((cnt * lba_size) > vmm::page_size) {
        if((cnt * lba_size) > (vmm::page_size * 2)) {
            size_t prp_cnt = (cnt - 1) * lba_size / vmm::page_size;
            if(prp_cnt > max_prps) {
                print("[NVME] Max prps exceeded\n");
                return -1;
            }

            auto align = (size_t)buf & (vmm::page_size - 1);

            for(size_t i = 0; i < prp_cnt; i++) {
                prp_list[i + new_command.cid * max_prps] = ((size_t)buf - vmm::high_vma - align) + vmm::page_size + i * vmm::page_size;
            }

            new_command.rw.prp2 = (size_t)&prp_list[new_command.cid * max_prps] - vmm::high_vma;
        } else {
            new_command.rw.prp2 = (size_t)buf + vmm::page_size - vmm::high_vma;
        }
    }

    new_command.opcode = rw ? 1 : 2;
    new_command.rw.nsid = nsid;
    new_command.rw.slba = start;
    new_command.rw.length = cnt - 1;
    new_command.rw.prp1 = (size_t)buf - vmm::high_vma;

    if(local->nvme_io_queue->send_cmd_and_poll(new_command, true) == 0xffff)
        return -1;

    return 0;
}

queue_pair::queue_pair(controller *parent_controller, size_t vector, size_t irq, size_t queue_type) : vector(vector), queue_type(queue_type), parent_controller(parent_controller) {
    entry_cnt = parent_controller->queue_entries;

    qid = parent_controller->qid_bitmap.alloc();

    if(qid == -1)
        return;

    submission_queue = (cmd*)(pmm::alloc(div_roundup(entry_cnt * sizeof(cmd), vmm::page_size)) + vmm::high_vma);
    completion_queue = (completion*)(pmm::alloc(div_roundup(entry_cnt * sizeof(completion), vmm::page_size)) + vmm::high_vma);

    auto submission_offset = vmm::page_size + 2 * qid * (4 << parent_controller->strides);
    submission_doorbell = (volatile uint32_t*)((size_t)parent_controller->regs + submission_offset);

    auto completion_offset = vmm::page_size + ((2 * qid + 1) * (4 << parent_controller->strides));
    completion_doorbell = (volatile uint32_t*)((size_t)parent_controller->regs + completion_offset);

    cid_bitmap = lib::bitmap(entry_cnt);

    parent_controller->queue_pair_list.push(this);

    if(queue_type & queue_admin)
        return;

    if(parent_controller->admin_queue == NULL)
        return;

    cmd create_cq_command {};

    create_cq_command.opcode = opcode_create_cq;
    create_cq_command.create_cq.prp1 = (size_t)completion_queue - vmm::high_vma;
    create_cq_command.create_cq.cqid = qid;
    create_cq_command.create_cq.qsize = entry_cnt - 1;
    create_cq_command.create_cq.irq_vector = irq;
    create_cq_command.create_cq.cq_flags = (1 << 0) | (1 << 1);

    if(parent_controller->admin_queue->send_cmd_and_poll(create_cq_command) == 0xffff) {
        print("[NVME] Unable to create IO completion queue\n");
        return;
    }

    cmd create_sq_command {};

    create_sq_command.opcode = opcode_create_sq;
    create_sq_command.create_sq.prp1 = (size_t)submission_queue - vmm::high_vma;
    create_sq_command.create_sq.sqid = qid;
    create_sq_command.create_sq.cqid = qid;
    create_sq_command.create_sq.qsize = entry_cnt - 1;
    create_sq_command.create_sq.sq_flags = (1 << 0) | (2 << 1);

    if(parent_controller->admin_queue->send_cmd_and_poll(create_sq_command) == 0xffff) {
        print("[NVME] Unable to create IO submission queue\n");
        return;
    }

    print("[NVME] Created IO queue pair with QID {}\n", qid);
}

queue_command *queue_pair::send_cmd(cmd submission, bool cid_override) {
    auto cid = [&, this]() -> uint16_t {
        if(cid_override)
            return submission.cid;
       return cid_bitmap.alloc();
    } ();

    if(cid == 0xffff)
        return NULL;

    pending_commands[cid] = new queue_command(this, cid);
    submission.cid = cid;

    submission_queue[sq_tail] = submission;
    if(++sq_tail == parent_controller->queue_entries)
        sq_tail = 0;

    *(submission_doorbell) = sq_tail;

    return pending_commands[cid];
}

uint16_t queue_pair::send_cmd_and_poll(cmd submission, bool cid_override) {
    const auto queue_cmd = send_cmd(submission, cid_override);
    if(queue_cmd == NULL) 
        return -1;

    for(;;) {
        if(queue_cmd->has_responded)
            break;
    }

    cid_bitmap.free(queue_cmd->cid);
    pending_commands.remove(queue_cmd->cid);

    return queue_cmd->completion_entry.status;
}

ns::ns(controller *parent_controller, namespace_id *identity, size_t nsid) : nsid(nsid), identity(identity), parent_controller(parent_controller) {
    lba_cnt = identity->nsze;
    lba_size = 1 << identity->lbaf_list[(unsigned)identity->flbas & 0b11111].ds;
}

void controller::irq(size_t vector) {
    queue_pair *queue = [&, this]() -> queue_pair* {
        for(size_t i = 0; i < queue_pair_list.size(); i++) {
            if(queue_pair_list[i]->vector == vector) {
                return std::move(queue_pair_list[i]);
            }
        }

        return NULL;
    } ();
    
    if(queue == NULL)
        return;

    //print("[NVME] MSIX RESPONDING TO CID {x} on interrupt vector {}\n", (uint16_t)queue->completion_queue[queue->cq_head].cid, vector);

    if(queue->completion_queue[queue->cq_head].status >> 1) {
        print("[NVME] Command Error: Status {x}\n", (uint16_t)queue->completion_queue[queue->cq_head].status);
        return;
    }

    for(size_t i = 0; i < queue->pending_commands.size(); i++) {
        if(queue->pending_commands[i]->cid == queue->completion_queue[queue->cq_head].cid) {
            queue->pending_commands[i]->has_responded = true;
            queue->pending_commands[i]->completion_entry = ((completion*)queue->completion_queue)[queue->cq_head];
            break;
        }
    }

    if(++queue->cq_head == queue->entry_cnt) {
        queue->cq_head = 0;
    }

    *queue->completion_doorbell = queue->cq_head;
}

int controller::get_namespace(namespace_id *identity, int nsid) {
    cmd new_command {};

    new_command.opcode = opcode_identify;
    new_command.identify.cns = 0;
    new_command.identify.nsid = nsid;
    new_command.identify.prp1 = (size_t)identity - vmm::high_vma;
    new_command.identify.prp2 = 0;

    if(admin_queue->send_cmd_and_poll(new_command) == 0xffff)
        return -1;

    return 0;
}

controller::controller(pci::device pci_device) {
    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 0);

    regs = reinterpret_cast<volatile registers*>(bar.base + vmm::high_vma);

    print("{x}\n", (size_t)regs);
    print("{} and {}\n", (unsigned)regs->vs, (unsigned)regs->cap);

    auto major_version = (regs->vs >> 16) & 0xffff;
    auto minor_version = (regs->vs >> 8) & 0xff;
    auto tertiary_version = regs->vs & 0xff;

    print("[NVME] Version Detected {}:{}:{}\n", major_version, minor_version, tertiary_version);

    switch(regs->vs) {
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

    if((regs->cap & (1ull << 37)) == 0) {
        print("[NVME] NVME Command Set Not Supported\n");
        return;
    }

    max_page_size = pow(2, 12 + (regs->cap >> 52 & 0b1111));
    min_page_size = pow(2, 12 + (regs->cap >> 48 & 0b1111));
    page_size = min_page_size;

    uint32_t cc = regs->cc;

    if(regs->cc & (1 << 0)) {
        regs->cc = regs->cc & ~(1 << 0); // disabled controller
    }

    while((regs->csts & (1 << 0))) { }

    int vec = x86::alloc_vec([](::regs *regs_cur, void *ptr) {
        ((controller*)ptr)->irq(regs_cur->isr_number);
    }, this);

    pci_device.set_interrupt(vec);

    queue_entries = regs->cap & 0xffff;
    strides = regs->cap >> 32 & 0xf;

    qid_bitmap = lib::bitmap(queue_entries);

    admin_queue = new queue_pair(this, vec, 0, 0);

    regs->aqa = (queue_entries - 1) << 16 | (queue_entries - 1);
    regs->asq = (size_t)admin_queue->submission_queue - vmm::high_vma;
    regs->acq = (size_t)admin_queue->completion_queue - vmm::high_vma;

    cc |= (0 << 4) | // nvme command set
          (0 << 11) | // ams = round robin
          (0 << 14) | // no notifications
          (6 << 16) | // io submission queue size 16 bytes 
          (4 << 20) | // io completion queue size 64 bytes
          (1 << 0); // enable

    regs->cc = cc;

    for(;;) {
        if(regs->csts & (1 << 0)) { // CSDT.RDY
            break;
        } else if(regs->csts & (1 << 1)) {  // CSDT.CFS
            print("[NVME] Controller Fatal Status\n");
            return;
        }
    }

    print("[NVME] Controller Restarted\n");

    ctrl_id = [&, this]() -> controller_id* {
        controller_id *ret = (controller_id*)(pmm::alloc(div_roundup(sizeof(controller_id), vmm::page_size)) + vmm::high_vma);

        cmd new_command {};

        new_command.opcode = opcode_identify;
        new_command.identify.cns = 1;
        new_command.identify.prp1 = (size_t)ret - vmm::high_vma;

        if(admin_queue->send_cmd_and_poll(new_command) == 0xffff) {
            return NULL;
        }

        return ret;
    } ();

    if(ctrl_id == NULL) {
        print("[NVME] Unable to get controller id\n");
        return;
    }

    print("[NVME] PCI vendor ID and subsystem vendor ID: {}/{}\n", (unsigned)ctrl_id->vid, (unsigned)ctrl_id->ssvid);
    print("[NVME] Model Number: {}\n", lib::string((const char*)ctrl_id->mn));
    print("[NVME] Serial Number: {}\n", lib::string((const char*)ctrl_id->sn));
    print("[NVME] Firmware Version: {}\n", lib::string((const char*)ctrl_id->fr));
    print("[NVME] IEEE OUI Identifier: {}:{}:{}\n", (unsigned)ctrl_id->ieee[0], (unsigned)ctrl_id->ieee[1], (unsigned)ctrl_id->ieee[2]);
    print("[NVME] Namespace max: {}\n", (unsigned)ctrl_id->nn);

    const auto nsid_list = [&, this]() -> uint32_t* {
        cmd new_command {};

        uint32_t *nsid_list = (uint32_t*)(pmm::alloc(align_up(ctrl_id->nn * sizeof(uint32_t), 0x1000)) + vmm::high_vma);

        new_command.opcode = opcode_identify;
        new_command.identify.cns = 2;
        new_command.identify.prp1 = (size_t)nsid_list - vmm::high_vma;

        if(admin_queue->send_cmd_and_poll(new_command) == 0xffff) {
            return NULL;
        }

        return nsid_list;
    } ();

    if(nsid_list == NULL) {
        print("[NVME] Unable to get nsid list\n");
        return;
    }

    for(size_t i = 0; i < ctrl_id->nn; i++) {
        if(nsid_list[i]) {
            namespace_id *namespace_identity = (namespace_id*)(pmm::alloc(1) + vmm::high_vma);

            if(get_namespace(namespace_identity, nsid_list[i]) == -1) {
                print("[NVME] Unable to get namespace identity struct for NSID {}\n", nsid_list[i]);
                return;
            }

            namespace_list.push(new ns(this, namespace_identity, nsid_list[i]));

            auto *identity = namespace_list.last()->identity;

            namespace_list.last()->max_prps = [&, this]() {
                auto lba_shift = identity->lbaf_list[identity->flbas & 0xf].ds;

                auto shift = 12 + (regs->cap >> 48 & 0xf);
                auto max_transfer_shift = (ctrl_id->mdts) ? shift + ctrl_id->mdts : 20;
                auto max_lbas = 1 << (max_transfer_shift - lba_shift);

                return (max_lbas * (1 << lba_shift)) / vmm::page_size;
            } ();

            print("[NVME] Namespace ID {}\n", nsid_list[i]);
            print("[NVME] \tLBA cnt: {x}\n", namespace_list.last()->lba_cnt);
            print("[NVME] \tLBA size: {x}\n", namespace_list.last()->lba_size);
            print("[NVME] \tMax PRPs: {x}\n", namespace_list.last()->max_prps);
        }
    }

    auto status = [&, this](size_t queue_cnt) {
        cmd new_command {};

        new_command.opcode = opcode_set_features;
        new_command.features.fid = 7; // set queue size
        new_command.features.dword11 = (queue_cnt - 1) | ((queue_cnt - 1) << 16);

        if(admin_queue->send_cmd_and_poll(new_command) == 0xffff) {
            return -1;
        }

        return 0;
    } (smp::cpus.size() * 2);

    if(status == -1) {
        print("[NVME] Unable to set queue cnt\n");
        return;
    }

    for(size_t i = 0; i < smp::cpus.size(); i++) {
        vec = x86::alloc_vec([](::regs *regs_cur, void *ptr) {
            ((controller*)ptr)->irq(regs_cur->isr_number);
        }, this);

        pci_device.set_interrupt(vec);

        smp::cpus[i]->nvme_io_queue = new queue_pair(this, vec, i + 1, 0);
    }

    for(size_t i = 0; i < namespace_list.size(); i++) {
        msd *new_msd = new msd;

        new_msd->device_prefix = lib::string("nvme") + controller_list.size() + lib::string("n") + i + lib::string("p");
        new_msd->device_index = controller_list.size();
        new_msd->partition_index = i;
        new_msd->sector_size = namespace_list[i]->lba_size;
        new_msd->sector_cnt = namespace_list[i]->lba_cnt;
        new_msd->partition_cnt = 0;

        dev::scan_partitions(new_msd);
    }

    controller_list.push(this);
}

}
