#include <drivers/hda/hda.hpp>
#include <drivers/hpet.hpp>
#include <int/idt.hpp>

namespace hda {

static lib::string widget_types[] = {
    "Audio Output",
    "Audio Input",
    "Audio Mixer",
    "Audio Selector",
    "Pin Complex",
    "Power Widget",
    "Volume Knob Widget",
    "Beep Generator Widget"
};

static struct {
    size_t freq;
    size_t base;
    size_t mult;
    size_t div;
} format_list[] = {
    { .freq = 8000, .base = 48, .mult = 1, .div = 6 },
    { .freq = 9600, .base = 48, .mult = 1, .div = 5 },
    { .freq = 11025, .base = 44, .mult = 1, .div = 4 },
    { .freq = 16000, .base = 48, .mult = 1, .div = 3 },
    { .freq = 22050, .base = 44, .mult = 1, .div = 2 },
    { .freq = 32000, .base = 48, .mult = 2, .div = 3 },
    { .freq = 44100, .base = 44, .mult = 1, .div = 1 },
    { .freq = 48000, .base = 48, .mult = 1, .div = 1 },
    { .freq = 88200, .base = 44, .mult = 2, .div = 1 },
    { .freq = 96000, .base = 48, .mult = 2, .div = 1 },
    { .freq = 176400, .base = 44, .mult = 4, .div = 1 },
    { .freq = 192000, .base = 48, .mult = 4, .div = 1 }
};

controller::_corb::_corb(controller *parent, size_t size) : size(size), parent(parent) {
    phys_base_addr = pmm::alloc(div_roundup(size * sizeof(uint32_t), vmm::page_size));
    base = (volatile uint32_t*)(phys_base_addr + vmm::high_vma);

    print("[HDA] CORB: disabled\n");

    if(parent->regs->corbctl & (1 << 1)) { // disable dma
        parent->regs->corbctl = parent->regs->corbctl & ~(1 << 1); 
    }

    parent->regs->corblbase = phys_base_addr & 0xffffffff;
    parent->regs->corbhbase = phys_base_addr >> 32 & 0xffffffff;
}

controller::_rirb::_rirb(controller *parent, size_t size) : size(size), head(0), parent(parent) {
    phys_base_addr = pmm::alloc(div_roundup(size * sizeof(uint64_t), vmm::page_size));
    base = (volatile uint32_t*)(phys_base_addr + vmm::high_vma);

    print("[HDA} RIRB: disabled\n");

    if(parent->regs->rirbctl & (1 << 1)) { // disable dma
        parent->regs->rirbctl = parent->regs->rirbctl & ~(1 << 1); 
    }

    parent->regs->rirblbase = phys_base_addr & 0xffffffff;
    parent->regs->rirbhbase = phys_base_addr >> 32 & 0xffffffff;
}

size_t widget::connection_list_entry(size_t index) {
    return parent_codec->send_cmd(nid, 0xf02 << 8 | index);
}

size_t widget::connection_list_size() {
    return parent_codec->verb_get_parameter(nid, codec::parameter_connection_list_len);
}

widget::widget(function_group *parent_group, size_t nid) : parent_group(parent_group), nid(nid) {
    parent_codec = parent_group->parent_codec;

    cap = parent_codec->verb_get_parameter(nid, codec::parameter_audio_widget_cap);

    if(cap >> 1 & 1) {
        input_amp_cap = parent_codec->verb_get_parameter(nid, codec::parameter_input_amp_cap);
    }

    if(cap >> 2 & 1) {
        output_amp_cap = parent_codec->verb_get_parameter(nid, codec::parameter_output_amp_cap);
    }

    type = cap >> 20 & 0b1111;

    if(type >= 8) {
        print("[NID {}] Vendor defined audio widget\n", nid);
        return;
    }

    print("[NID {}] Audio widget detected: {}\n", nid, widget_types[type]);

    pin_cap = parent_codec->verb_get_parameter(nid, codec::parameter_pin_caps);
    volume_knob_cap = parent_codec->verb_get_parameter(nid, codec::parameter_volume_knob_cap);
    config_default = parent_codec->send_cmd(nid, 0xf1c << 8);

    if(cap & (1 << 8)) { // connection list
        auto size = connection_list_size();

        auto length = size & 0x7f;

        if(size & (1 << 8)) { // long form
            for(size_t i = 0; i < length; i += 2) {
                auto entry = connection_list_entry(i);

                for(size_t j = 0; i < length && j < 2; i++, j++) {
                    auto connection = entry >> (j * 8) & 0xffff;
                    connection_list.push(connection);
                }
            } 
        } else {
            for(size_t i = 0; i < length;) {
                auto entry = connection_list_entry(i); 

                for(size_t j = 0; i < length && j < 4; i++, j++) {
                    auto connection = entry >> (j * 8) & 0xff;
                    connection_list.push(connection);
                }
            }
        }

        print("[NID {}] Connection List:\n", nid);

        for(size_t i = 0; i < connection_list.size(); i++) {
            print("[NID {}]    Connection entry: {}\n", nid, connection_list[i]);
        }
    }
}

function_group::function_group(codec *parent_codec, size_t nid) : parent_codec(parent_codec), nid(nid) {
    auto res = parent_codec->verb_get_parameter(nid, codec::parameter_function_group_type);

    unsol_capable = res >> 8 & 1;
    node_type = res & 0xff;

    switch(node_type) {
        case 1:
            print("[NID {}] audio function group detected\n", nid);
            break;
        case 2:
            print("[NID {}] vendor defined modem function group\n", nid);
            return;
        default:
            print("[NID {}] vendor defined function group\n", nid);
            return;
    }

    res = parent_codec->verb_get_parameter(nid, codec::parameter_subordinate_node_cnt);

    node_cnt = res & 0xff;
    starting_node = res >> 16 & 0xff;

    print("[NID {}] Subordindate nodes: {} -> {}\n", nid, starting_node, starting_node + node_cnt);

    for(size_t i = starting_node; i < (starting_node + node_cnt); i++) {
        widgets[i] = new widget(this, i);
    }
}

void codec::parse() {
    auto res = verb_get_parameter(0, parameter_vendor_id);

    device_id = res & 0xffff;
    vendor_id = res >> 16 & 0xffff;

    print("[CODEC] vid: {x}\n", vendor_id);
    print("[CODEC] did: {x}\n", device_id);

    res = verb_get_parameter(0, parameter_revision_id);

    version_major = res >> 20 & 0b1111;
    version_minor = res >> 16 & 0b1111;

    print("[CODEC] {x}:{x}\n", version_major, version_minor);

    res = verb_get_parameter(0, parameter_subordinate_node_cnt);

    starting_node = res >> 16 & 0xff;
    node_cnt = res & 0xff;

    print("[CODEC] Nodes: {} -> {}\n", starting_node, node_cnt);

    for(size_t i = starting_node; i < (starting_node + node_cnt); i++) {
        function_groups[i] = new function_group(this, i);
    }
}

void controller::irq() {
    if(!(regs->rirbsts & ((1 << 0) | (1 << 2)))) // check for a response interrupt
        return;

    if(regs->rirbsts & (1 << 2))
        regs->rirbsts = regs->rirbsts | (1 << 2); // clear override interrupt

    regs->rirbsts = (1 << 0) | (1 << 2);

    while(rirb->head != regs->rirbwp) {
        rirb->head = (rirb->head + 1) % rirb->size;

        uint64_t response = rirb->base[2 * rirb->head];
        uint64_t response_extended = rirb->base[2 * rirb->head + 1];

        if(response_extended & (1 << 4)) { // uusolicited response
            continue;
        }

        uint32_t codec_index = response_extended & 0b1111;

        codecs[codec_index]->has_responded = true;
        codecs[codec_index]->codec_response = response;
    }
}

uint32_t codec::send_cmd(uint8_t node, uint32_t cmd) {
    uint32_t verb = (codec_index & 15 << 28) | (node << 20) | (cmd & 0xfffff);

    has_responded = false;

    auto index = (parent->regs->corbwp + 1) % parent->corb->size;
    parent->corb->base[index] = verb;
    parent->regs->corbwp = index;

    while(!has_responded) {
        asm ("pause");
    }

    return codec_response;
}

widget *codec::find_widget(uint8_t node_id) {
    for(size_t i = 0; i < function_groups.size(); i++) { 
        function_group *group = function_groups[function_groups.get_tag(i)];
        for(size_t j = 0; j < group->widgets.size(); i++) {
            widget *wid = group->widgets[group->widgets.get_tag(j)];
            if(wid->nid == node_id) {
                return wid;
            }
        }
    }

    return NULL;
}

void stream::reset_stream() {
    stream_desc->ctl = stream_desc->ctl & ~(1 << 1); // clear dma
    stream_desc->ctl = stream_desc->ctl | 1;
    while(stream_desc->ctl & 1);
}

void stream::push(uint8_t *buf, size_t size) {
    if(entry_index == entry_cnt)
        return;

    buf_desc[entry_index].addr = (size_t)buf - vmm::high_vma;
    buf_desc[entry_index].length = size;

    total_size += size;
    entry_index++;
}

void stream::start() {
    stream_desc->cbl = total_size;
    stream_desc->ctl = stream_desc->ctl | (1 << 1);
}

stream::stream(controller *parent_controller, size_t sample_rate, size_t sample_size, size_t channels, size_t codec_index, size_t pin, size_t dac, size_t sid, size_t entry_cnt) :
    pin(pin),
    dac(dac),
    sid(sid),
    codec_index(codec_index),
    sample_rate(sample_rate),
    sample_size(sample_size),
    channels(channels),
    entry_cnt(entry_cnt),
    entry_index(0),
    total_size(0),
    parent_controller(parent_controller) {
    stream_desc = parent_controller->alloc_out_desc();

    reset_stream();

    if(stream_desc == NULL)
        return;

    stream_desc->ctl_high = stream_desc->ctl_high | (sid << 4) | (1 << 2); // preferred trafic and setting stream id

    size_t format = (channels - 1) & 0b1111;

    for(size_t i = 0; i < lengthof(format_list); i++) {
        if(format_list[i].freq == sample_rate) {
            format |= (((format_list[i].base == 48) ? 0 : 1)  & 1) << 14;
            format |= (format_list[i].mult - 1) << 11;
            format |= (format_list[i].div - 1) << 8;
            break;
        }
    }

    if(!format)
        return;

    switch(sample_size) {
        case 8: format |= 0b000 << 4; break;
        case 16: format |= 0b001 << 4; break;
        case 20: format |= 0b010 << 4; break;
        case 24: format |= 0b011 << 4; break; 
        case 32: format |= 0b100 << 4; break;
        default: 
            print("[HDA] Unknown sample size {}\n", sample_size);
    }

    stream_desc->fmt = format;

    buf_desc = (volatile buffer_descriptor*)(pmm::alloc(div_roundup(entry_cnt * sizeof(buffer_descriptor), vmm::page_size)) + vmm::high_vma);

    stream_desc->bdpl = ((size_t)buf_desc - vmm::high_vma) & 0xffffffff;
    stream_desc->bdph = ((size_t)buf_desc - vmm::high_vma) >> 32 & 0xffffffff;
    stream_desc->lvi = entry_cnt - 1;
    stream_desc->cbl = 0;

    parent_controller->codecs[codec_index]->send_cmd(dac, (0x2 << 16) | format); 
    parent_controller->codecs[codec_index]->send_cmd(dac, (0x706 << 8) | (sid << 4 | 2));

    widget *dac_widget = parent_controller->codecs[codec_index]->find_widget(dac);

    auto gain = dac_widget->output_amp_cap >> 8 & 0x7f;
    parent_controller->codecs[codec_index]->send_cmd(dac, (0x3 << 16) | (1 << 15) | (1 << 13) | (1 << 12) | gain);

    parent_controller->codecs[codec_index]->send_cmd(pin, (0x707 << 8) | (1 << 7)); // enable pin out
    parent_controller->codecs[codec_index]->send_cmd(pin, (0x70c << 8) | (1 << 1));
}

controller::controller(pci::device pci_device) {
    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 0);

    regs = reinterpret_cast<volatile registers*>(bar.base + vmm::high_vma);

    regs->gctl = regs->gctl & ~(1 << 0); // disable controller
    while((regs->gctl & (1 << 0)) != 0);

    regs->gctl = regs->gctl | 1; // enable controller
    while((regs->gctl & (1 << 0)) == 0); 

    ksleep(1); // let the codecs get themselves in order

    print("[HDA] Version Detected {}:{}\n", regs->vmaj, regs->vmin);

    oss = regs->gcap >> 12 & 0b1111;
    iss= regs->gcap >> 8 & 0b1111;
    bss = regs->gcap >> 3 & 0b11111;
    nsdo = regs->gcap >> 1 & 0b11;

    print("[HDA] oss {x} iss {x} bss {x} nsdo {x}\n", oss, iss, bss, nsdo);

    auto is_64_bit = regs->gcap & 1;

    if(is_64_bit) {
        print("[HDA] 64 bit operation supported\n");
    } else {
        print("[HDA] 64 bit operation not supported\n");
        return;
    }

    regs->wakeen = 0;
    //regs->ssync = regs->ssync | 0x3fffffff;
    regs->rirbsts = regs->rirbsts | (1 << 0) | (1 << 2);

    auto calcuate_buffer_size = [&, this](size_t size) -> std::pair<int, int> { 
        if(size & (1 << 2)) {
            return { 256, corbsize_256 };
        } else if(size & (1 << 1)) {
            return { 16, corbsize_16 };
        } else if(size & (1 << 0)) {
            return { 2, corbsize_2 };
        } else {
            return { -1, -1 };
        }
    };

    auto corbszcap = regs->corbsize >> 4 & 0b1111;

    auto corb_size = calcuate_buffer_size(corbszcap);

    if(corb_size.first == -1) {
        print("[HDA] No valid corbsize found\n");
        return;
    }

    corb = new _corb(this, corb_size.first);

    regs->corbsize = (regs->corbsize & ~(0b11)) | corb_size.second;

    print("[HDA] CORB: {} entries\n", corb_size.first);

    regs->corbrp = regs->corbrp | 1 << 15; // corbrp reset and clear
    while((regs->corbrp & (1 << 15)) == 0);

    regs->corbrp = regs->corbrp & ~(1 << 15); // verify everything has been cleared
    while(regs->corbrp & (1 << 15));

    regs->corbwp = regs->corbwp & ~(0xff); // clear corbwp
    regs->corbctl = regs->corbctl | (1 << 1); // enable corb dma

    print("[HDA] CORB: started\n");

    auto rirbsize = regs->rirbsize >> 4 & 0b1111;
    auto rirb_size = calcuate_buffer_size(rirbsize);

    print("[HDA] RIRB: {} entries\n", rirb_size.first);

    rirb = new _rirb(this, rirb_size.first);

    regs->rirbsize = (regs->rirbsize & ~(0b11)) | rirb_size.second;

    regs->rirbwp = regs->rirbwp | (1 << 15); // rirbwp reset and clear
    regs->rirbcnt = 1;
    regs->rirbctl = regs->rirbctl | (1 << 1) | (1 << 0); // enable dma and enable interrupts

    print("[HDA] RIRB: started\n");

    int vec = x86::alloc_vec([](::regs*, void *ptr) {
        ((controller*)ptr)->irq();
    }, this);
    pci_device.set_msi(vec);

    regs->gctl = regs->gctl | (1 << 8);
    regs->intctl = regs->intctl | (1u << 31) | (1 << 30);

    auto wakests = regs->wakests & 0x7FFF;
    for(size_t i = 0; i < 15; i++) {
        if(wakests & (1 << i)) {
            codecs[i] = new codec(this, i);
            codecs[i]->parse();
        }
    }

    in_desc = (volatile stream_descriptor*)(bar.base + vmm::high_vma + sizeof(registers));
    out_desc = (volatile stream_descriptor*)(bar.base + vmm::high_vma + sizeof(registers) + iss * sizeof(stream_descriptor));
    bi_desc = (volatile stream_descriptor*)(bar.base + vmm::high_vma + sizeof(registers) + iss * sizeof(stream_descriptor) + oss * sizeof(stream_descriptor));

    in_bitmap = (uint8_t*)kmm::calloc(div_roundup(iss, 8));
    out_bitmap = (uint8_t*)kmm::calloc(div_roundup(oss, 8));
    bi_bitmap = (uint8_t*)kmm::calloc(div_roundup(bss, 8));

/*    stream new_stream(this, 44100, 16, 2, 0, 3, 2, 0, 256);
    vfs::mount("/dev/nvme0n0p0-0", "/");
    fs::fd fd("/tarkus.pcm", 0, 0);
    uint8_t *data = (uint8_t*)(pmm::alloc(div_roundup(0x1000000, vmm::page_size)) + vmm::high_vma);
    fd.read(data, 0x1000000);
    new_stream.push(data, 0x1000000);
    new_stream.start();*/
}

}
