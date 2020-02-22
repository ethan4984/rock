global start_paging
global load_paging

start_paging:
	push ebp
	mov ebp, esp
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	mov esp, ebp
	pop ebp
	ret

load_paging:
	push ebp
	mov ebp, esp
	mov cr3, eax
	mov eax, [esp + 8]
	mov esp, ebp
	pop ebp
	ret




