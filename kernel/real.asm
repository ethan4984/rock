global smp_core_init_begin
global smp_core_init_end

smp_core_init_begin:

incbin 'bin/smp_asm.bin'

smp_core_init_end:
