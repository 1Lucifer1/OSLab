
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_process_sleep   equ 1
_NR_print_str       equ 2 
_NR_P	            equ 3
_NR_V               equ 4

INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	process_sleep
global	print_str
global	P
global	V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              process_sleep
; ====================================================================
process_sleep:
	push	ebx
	mov	ebx, [esp + 8]
	mov	eax, _NR_process_sleep
	int	INT_VECTOR_SYS_CALL
	pop	ebx
	ret

; ====================================================================
;                              print_str
; ====================================================================
print_str:
	push	ebx
	mov	ebx, [esp + 8]
	mov	eax, _NR_print_str
	int	INT_VECTOR_SYS_CALL
	pop	ebx
	ret

; ====================================================================
;                              P
; ====================================================================
P:
	push	ebx
	mov	ebx, [esp + 8]
	mov	eax, _NR_P
	int	INT_VECTOR_SYS_CALL
	pop	ebx
	ret

; ====================================================================
;                              V
; ====================================================================
V:
	push	ebx
	mov	ebx, [esp + 8]
	mov	eax, _NR_V
	int	INT_VECTOR_SYS_CALL
	pop	ebx
	ret


