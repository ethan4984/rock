#ifndef NVME_HPP_
#define NVME_HPP_
 
#include <drivers/pci.hpp>
#include <cpu.hpp>
#include <bitmap.hpp>
#include <vector.hpp> 
#include <fs/dev.hpp> 
 
namespace nvme {
 
constexpr size_t version_1_0_0_id = (1 << 8 | 0) << 8 | 0;
constexpr size_t version_1_1_0_id = (1 << 8 | 1) << 8 | 0;
constexpr size_t version_1_2_0_id = (1 << 8 | 2) << 8 | 0;
constexpr size_t version_1_2_1_id = (1 << 8 | 2) << 8 | 1;
constexpr size_t version_1_3_0_id = (1 << 8 | 3) << 8 | 0;
constexpr size_t version_1_4_0_id = (1 << 8 | 4) << 8 | 0;

constexpr size_t queue_admin = (1 << 0);
constexpr size_t queue_interrupt = (1 << 1);

constexpr size_t async_block_size = 0x2000;

struct [[gnu::packed]] registers {
    uint64_t cap;   
    uint32_t vs;  
    uint32_t intms;
    uint32_t intmc;
    uint32_t cc; 
    uint32_t rsvd1; 
    uint32_t csts;
    uint32_t rsvd2;
    uint32_t aqa; 
    uint64_t asq; 
    uint64_t acq;
};
 
constexpr size_t opcode_del_sq = 0x0;
constexpr size_t opcode_create_sq = 0x1;
constexpr size_t opcode_delete_cq = 0x4;
constexpr size_t opcode_create_cq = 0x5;
constexpr size_t opcode_identify = 0x6;
constexpr size_t opcode_abort = 0x8;
constexpr size_t opcode_set_features = 0x9;
constexpr size_t opcode_get_features = 0xa;
constexpr size_t opcode_namespace_management = 0xd;
constexpr size_t opcode_format_cmd = 0x80;

namespace command {

struct [[gnu::packed]] create_cq {
    uint32_t rsvd1[5];
    uint64_t prp1;
    uint64_t rsvd8;
    uint16_t cqid;
    uint16_t qsize;
    uint16_t cq_flags;
    uint16_t irq_vector;
    uint32_t rsvd12[4];
};

struct [[gnu::packed]] create_sq {
    uint32_t rsvd1[5];
    uint64_t prp1;
    uint64_t rsvd8;
    uint16_t sqid;
    uint16_t qsize;
    uint16_t sq_flags;
    uint16_t cqid;
    uint32_t rsvd12[4];
};

struct [[gnu::packed]] delete_queue {
    uint32_t rsvd1[9];
    uint16_t qid;
    uint16_t rsvd10;
    uint32_t rsvd11[5];
};

struct [[gnu::packed]] abort {
    uint32_t rsvd1[9];
    uint16_t sqid;
    uint16_t cid;
    uint32_t rsvd11[5];
};

struct [[gnu::packed]] features {
    uint32_t nsid;
    uint64_t rsvd2[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t fid;
    uint32_t dword11;
    uint32_t rsvd12[4];
};

struct [[gnu::packed]] identify {
    uint32_t nsid;
    uint64_t rsvd2[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cns;
    uint32_t rsvd11[5];
};

struct [[gnu::packed]] rw {
    uint32_t nsid;
    uint64_t rsvd2;
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint64_t slba;
    uint16_t length;
    uint16_t control;
    uint32_t dsmgmt;
    uint32_t reftag;
    uint16_t apptag;
    uint16_t appmask;
};

}

struct cmd {
    uint8_t opcode;
    uint8_t flags;
    uint16_t cid;

    union {
        command::create_cq create_cq;
        command::create_sq create_sq;
        command::delete_queue delete_queue;
        command::abort abort;
        command::features features;
        command::identify identify;
        command::rw rw;
    };
};

struct [[gnu::packed]] completion {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
};

struct [[gnu::packed]] power_state_id {
    uint16_t max_power;
    uint8_t rsvd2;
    uint8_t flags;
    uint32_t entry_lat;
    uint32_t exit_lat;
    uint8_t read_tput;
    uint8_t read_lat;
    uint8_t write_tput;
    uint8_t write_lat;
    uint16_t idle_power;
    uint8_t idle_scale;
    uint8_t rsvd19;
    uint16_t active_power;
    uint8_t active_work_scale;
    uint8_t rsvd23[9];
};

struct [[gnu::packed]] controller_id {
    uint16_t vid;
    uint16_t ssvid;
    char sn[20];
    char mn[40];
    char fr[8];
    uint8_t rab;
    uint8_t ieee[3];
    uint8_t mic;
    uint8_t mdts;
    uint16_t cntlid;
    uint32_t ver;
    uint8_t rsvd84[172];
    uint16_t oacs;
    uint8_t acl;
    uint8_t aerl;
    uint8_t frmw;
    uint8_t lpa;
    uint8_t elpe;
    uint8_t npss;
    uint8_t avscc;
    uint8_t apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint8_t rsvd270[242];
    uint8_t sqes;
    uint8_t cqes;
    uint8_t rsvd514[2];
    uint32_t nn;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t fna;
    uint8_t vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t nvscc;
    uint8_t rsvd531;
    uint16_t acwu;
    uint8_t rsvd534[2];
    uint32_t sgls;
    uint8_t rsvd540[1508];
    power_state_id psd[32];
    uint8_t vs[1024];
};

struct [[gnu::packed]] lbaf {
    uint16_t ms;
    uint8_t ds;
    uint8_t rp;
};

struct [[gnu::packed]] namespace_id {
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;
    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;
    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;
    uint8_t fpi;
    uint8_t rsvd33;
    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;
    uint16_t rsvd46;
    uint64_t nvmcap[2];
    uint8_t rsvd64[40];
    uint8_t nguid[16];
    uint8_t eui64[8];
    lbaf lbaf_list[16];
    uint8_t rsvd192[192];
    uint8_t vs[3712];
};

class controller;
class queue_pair;

struct msd : dev::msd {
    ssize_t raw_read(size_t off, size_t cnt, void *buf);
    ssize_t raw_write(size_t off, size_t cnt, void *buf);
};

struct queue_command {
    queue_command(queue_pair *parent_queue, uint16_t cid) : has_responded(false), cid(cid), completion_entry({}), parent_queue(parent_queue) { }
    queue_command() = default;

    volatile bool has_responded;
    uint16_t cid;
    completion completion_entry;
    queue_pair *parent_queue;
};

struct ns {
    ns(controller *parent_controller, namespace_id *identity, size_t nsid);
    ns() = default;

    int rw_lba(void *buf, size_t start, size_t cnt, bool rw);

    size_t lba_cnt;
    size_t lba_size;
    size_t max_prps;
    size_t nsid;
    
    namespace_id *identity;
    controller *parent_controller;
};

class queue_pair {
public:
    queue_pair(controller *parent_controller, size_t vector, size_t irq, size_t queue_type);
    queue_pair() = default;

    queue_command *send_cmd(cmd submission, bool cid_override = false);
    uint16_t send_cmd_and_poll(cmd submission, bool cid_override = false);

    friend controller;
    friend msd;
    friend ns;
private:
    cmd *submission_queue;
    volatile completion *completion_queue;

    volatile uint32_t *submission_doorbell;
    volatile uint32_t *completion_doorbell;

    size_t qid;
    size_t entry_cnt;
    size_t sq_head;
    size_t sq_tail;
    size_t cq_head;
    size_t cq_tail;
    size_t phase;
    size_t vector;
    size_t queue_type;

    lib::map<size_t, queue_command*> pending_commands;
    lib::bitmap cid_bitmap;
 
    controller *parent_controller;
};
 
class controller {
public:
    controller(pci::device pci_device);
    controller() = default;

    lib::vector<queue_pair*> queue_pair_list;
    lib::vector<ns*> namespace_list;
    int get_namespace(namespace_id *identity, int nsid);

    friend queue_pair;
    friend ns; 
private:
    pci::device pci_device;
    pci::bar bar;

    volatile registers *regs;

    void irq(size_t vector);

    size_t queue_entries;
    size_t max_page_size;
    size_t min_page_size;
    size_t page_size;
    size_t max_transfer_shift;
    size_t max_prps;
    size_t strides;

    queue_pair *admin_queue;

    lib::bitmap qid_bitmap;
    controller_id *ctrl_id;
};

inline lib::vector<controller*> controller_list;
 
} 
 
#endif 
