#ifndef HDA_HPP_
#define HDA_HPP_

#include <drivers/pci.hpp>
#include <cpu.hpp>
#include <map.hpp>

namespace hda {

constexpr auto corbsize_2 = 0b00;
constexpr auto corbsize_16 = 0b01;
constexpr auto corbsize_256 = 0b10;

struct [[gnu::packed]] registers {
    uint16_t gcap;
    uint8_t vmin;
    uint8_t vmaj;
    uint16_t outpay;
    uint16_t inpay;
    uint32_t gctl;
    uint16_t wakeen;
    uint16_t wakests;
    uint16_t gsts;
    uint8_t reserved0[6];
    uint16_t outstrmpay;
    uint16_t instrmpay;
    uint8_t reserved1[4];
    uint32_t intctl;
    uint32_t insts;
    uint8_t reserved2[8];
    uint32_t walclk;
    uint32_t old_ssync;
    uint32_t ssync;
    uint8_t reserved3[4];
    uint32_t corblbase;
    uint32_t corbhbase;
    uint16_t corbwp;
    uint16_t corbrp;
    uint8_t corbctl;
    uint8_t corbsts;
    uint8_t corbsize;
    uint8_t reserved4;
    uint32_t rirblbase;
    uint32_t rirbhbase;
    uint16_t rirbwp;
    uint16_t rirbcnt;
    uint8_t rirbctl;
    uint8_t rirbsts;
    uint8_t rirbsize;
    uint8_t reserved5;
    uint32_t icoi;
    uint32_t icii;
    uint16_t icis;
    uint8_t reserved6[6];
    uint32_t dpiblbase;
    uint32_t bpibubase;
    uint8_t reserved7[8]; 
};

class controller;
class function_group;
class widget;
class stream; 

class codec {
public:
    codec(controller *parent, size_t codec_index) : parent(parent), has_responded(false), codec_index(codec_index) { }
    codec() = default;

    uint32_t send_cmd(uint8_t node, uint32_t cmd);
    widget *find_widget(uint8_t node_id); 
    void parse();

    static constexpr size_t parameter_vendor_id = 0;
    static constexpr size_t parameter_revision_id = 2;
    static constexpr size_t parameter_subordinate_node_cnt = 4;
    static constexpr size_t parameter_function_group_type = 5;
    static constexpr size_t parameter_audio_function_group_cap = 8;
    static constexpr size_t parameter_audio_widget_cap = 9;
    static constexpr size_t parameter_sample_size = 0xa;
    static constexpr size_t parameter_stream_formats= 0xb; 
    static constexpr size_t parameter_pin_caps = 0xc;
    static constexpr size_t parameter_input_amp_cap = 0xd;
    static constexpr size_t parameter_output_amp_cap = 0x12;
    static constexpr size_t parameter_connection_list_len = 0xe;
    static constexpr size_t parameter_supported_power_states = 0xf;
    static constexpr size_t parameter_processing_cap = 0x10;
    static constexpr size_t parameter_gpi_cnt = 0x11;
    static constexpr size_t parameter_volume_knob_cap = 0x13;
    static constexpr size_t parameter_hdmi_lpcm_cad = 0x20;

    uint32_t verb_get_parameter(uint8_t node, uint8_t parameter) {
        return send_cmd(node, (0xf00 << 8) | parameter);
    }

    friend controller;
private:
    controller *parent; 

    lib::map<size_t, function_group*> function_groups;

    size_t vendor_id;
    size_t device_id;
    size_t version_major;
    size_t version_minor; 
    size_t starting_node;
    size_t node_cnt;

    volatile bool has_responded;
    uint32_t codec_response;

    size_t codec_index;
    size_t lock;
};

class function_group {
public:
    function_group(codec *parent_codec, size_t nid);
    function_group() = default;

    lib::map<size_t, widget*> widgets;

    friend widget;
private:
    size_t node_type;
    size_t unsol_capable;

    size_t node_cnt;
    size_t starting_node;

    codec *parent_codec;
    size_t nid;
};

class widget {
public:
    widget(function_group *parent_group, size_t nid);
    widget() = default;

    size_t connection_list_entry(size_t index);
    size_t connection_list_size();

    lib::vector<size_t> connection_list;

    size_t type;

    friend codec;
    friend stream;
private:
    size_t cap;
    size_t pin_cap;
    size_t input_amp_cap;
    size_t output_amp_cap;
    size_t volume_knob_cap;
    size_t config_default;

    function_group *parent_group;
    codec *parent_codec;
    size_t nid;
};

struct [[gnu::packed]] stream_descriptor {
    uint16_t ctl;
    uint8_t ctl_high;
    uint8_t sts;
    uint32_t lpib;
    uint32_t cbl;
    uint16_t lvi;
    uint16_t reserved;
    uint16_t fifod;
    uint16_t fmt;
    uint32_t reserved0;
    uint32_t bdpl;
    uint32_t bdph;
};

static_assert(sizeof(stream_descriptor) == 0x20);

struct [[gnu::packed]] buffer_descriptor {
    uint64_t addr;
    uint32_t length;
    uint32_t flags;
};

class stream {
public:
    stream(controller *parent_controller, size_t sample_rate, size_t sample_size, size_t channels, size_t codec, size_t pin, size_t dac, size_t sid, size_t entry_cnt);
    stream() = default;

    void reset_stream();
    void start();
    void push(uint8_t *buf, size_t size);

    size_t pin;
    size_t dac;
    size_t sid;
    size_t codec_index;
private:
    size_t sample_rate;
    size_t sample_size;
    size_t channels;
    size_t entry_cnt;

    size_t entry_index;
    size_t total_size;

    controller *parent_controller;

    volatile stream_descriptor *stream_desc;
    volatile buffer_descriptor *buf_desc;
};

class controller {
public: 
    controller(pci::device pci_device);
    controller() = default;

    volatile stream_descriptor *alloc_out_desc() {
        for(size_t i = 0; i < oss; i++) {
            if(bm_test(out_bitmap, i) == 0) {
                bm_set(out_bitmap, i);
                return &out_desc[i];
            }
        }
        return NULL;
    }

    volatile stream_descriptor *alloc_bi_desc() {
        for(size_t i = 0; i < bss; i++) {
            if(bm_test(bi_bitmap, i) == 0) {
                bm_set(bi_bitmap, i);
                return &bi_desc[i];
            }
            return NULL;
        }
    }

    volatile stream_descriptor *alloc_in_desc() {
        for(size_t i = 0; i < iss; i++) {
            if(bm_test(in_bitmap, i) == 0) {
                bm_set(in_bitmap, i);
                return &in_desc[i];
            }
            return NULL;
        }
    }

    volatile registers *regs;

    friend codec;
    friend stream;
private:
    pci::device pci_device;
    pci::bar bar;

    lib::map<size_t, codec*> codecs;

    size_t oss;
    size_t iss;
    size_t bss;
    size_t nsdo;

    volatile stream_descriptor *in_desc;
    volatile stream_descriptor *out_desc;
    volatile stream_descriptor *bi_desc;
    
    uint8_t *in_bitmap;
    uint8_t *out_bitmap;
    uint8_t *bi_bitmap;

    struct _corb {
        _corb(controller *parent, size_t size);
        _corb() = default;

        size_t size;
        size_t phys_base_addr;
        volatile uint32_t *base;

        controller *parent;
    } *corb;

    struct _rirb {
        _rirb(controller *paent, size_t size);
        _rirb() = default;

        size_t size;
        size_t head;
        size_t phys_base_addr;
        volatile uint32_t *base;

        controller *parent;
    } *rirb;

    void irq();

    size_t lock;
};

struct device {
    controller *parent_controller;
    codec *parent_codec;
    size_t pin;
    size_t dac;
};

inline lib::vector<device> device_list;

}

#endif
