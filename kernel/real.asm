global smp_tramp_begin
global smp_tramp_end

smp_tramp_begin:

incbin 'bin/smp_asm.bin'

smp_tramp_end:
