#include <sound.h>
#include <port.h>
#include <stdint.h>
#include <shitio.h>
#include <interrupt.h>
#include <scheduler.h>

using namespace standardout;

int note_n[7] = { 220, 246, 262, 294, 329, 349, 392 }; /* natural note frequency */
char note_c[7] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
int note_s[7] = { 233, 0, 277, 311, 0, 370, 415 }; /* sharp note frequency */

uint32_t song[9] = { 'c', false, 15, 'd', false, 15, 'd', true, 15 };

/* 1 = note, 2 = sharp or not, 3 = time */

void play_note(const char note_x, bool sharp = false)
{
    int frequency;
    for(int i = 0; i < 7; i++) {
        if(note_x == note_c[i] && sharp)
            frequency = note_s[i];
        if(note_x == note_c[i] && !sharp)
            frequency = note_n[i];
    }

    uint32_t div;
    uint8_t old_div;

    div = 1193180 / frequency;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)(div));
    outb(0x42, (uint8_t)(div >> 8));

    old_div = inb(0x61);
    if(old_div != (old_div | 3))
        outb(0x61, old_div | 3);
}

void play_song(uint32_t notes[], uint32_t size)
{
    for(int i = 0; i < size; i += 3) {
        play_note(notes[i], notes[i+1]);
        nanosleep(notes[i+2]);
    }
}

void stop_sound()
{
    uint8_t old_div = inb(0x61) & 0xfc;
    outb(0x61, old_div);
}
