.text

.global _start

_start:
	mov $3, %rax
	mov $1, %rdi
	mov $lol, %rsi
	mov $9, %rdx
	syscall

	mov $14, %rax
	xor %rdi, %rdi
	syscall

lol:
	.ascii  "gamering\n"
