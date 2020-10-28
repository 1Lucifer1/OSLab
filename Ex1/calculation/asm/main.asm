section .data
msgl db 'please input two numbers and split with blank',0Ah,0h
huanhang db 0Ah, 0h
zero db '0', 0Ah , 0h
jinwei: db 0
one_len: dd 0
two_len: dd 0
add_one_len: dd 0
add_two_len: dd 0
res_len: dd 0
jishu_one: dd 0
jishu_two: dd 0
mul_jishu: dd 0
mul_len: dd 0
two_addr: dd 0

section .bss
add_result: resb 255
input: resb 255
mul_result: resb 255

section .text
    global _start   ;must be declare for using gcc

_start:
    ;print instruction
    mov eax, msgl
    call sprint

    call sinput
    call split
    call add_two_number
    call mul_two_number

    ;mov eax, input
    ;call sprint

    ;System Call to exit
    mov eax, 1
    mov ebx, 0
    int 80h

split:
    ;get the space
    mov edi, input

;find number1 len
find_loop:
    cmp byte[edi], ' '
    je found
    inc edi
    inc dword[one_len]
    jmp find_loop

found:
    ;sub dword[one_len], 1
    mov byte[edi], 0
    mov eax, input
    ;call sprint
    inc edi ;number2 start
    mov eax, edi
    ;call sprint
    mov edx, edi
;find number2 len
find_loop_two:
    cmp byte[edx], 0Ah
    je found_two
    inc edx
    inc dword[two_len]
    jmp find_loop_two
found_two:
    mov dword[two_addr], edi
    ;sub dword[two_len], 1
    ret

add_two_number:
    mov eax, input
    add eax, dword[one_len]
    mov esi, dword[one_len]
    mov dword[add_one_len], esi
    sub eax, 1
    mov ebx, edi
    add ebx, dword[two_len]
    mov esi, dword[two_len]
    mov dword[add_two_len], esi
    sub ebx, 1
    mov ecx, add_result
    add ecx, 254

    mov byte[ecx], 0
    dec ecx
    mov byte[ecx], 0Ah
    dec ecx

    ;call printabc
add_loop:
    mov edx, dword[add_one_len]
    sub edx, dword[jishu_one]
    cmp edx, 0
    ;cmp dword[one_len], dword[jishu_one]
    jg nextstep
    jmp add_finish
nextstep:
    mov edx, dword[add_two_len]
    sub edx, dword[jishu_two]
    cmp edx, 0
    ;call printabc
    ;cmp dword[two_len], dword[jishu_two]
    jng add_finish
    
    ;call printabc
    mov byte[ecx], 0
    cmp byte[jinwei], 1
    jne bujin
    add byte[ecx], 1
bujin:
    mov dl, byte[eax]
    sub dl, 48
    mov dh, byte[ebx]
    sub dh, 48
    add dl, dh
    add byte[ecx], dl
    
    cmp byte[ecx], 10
    jnl jin
    mov byte[jinwei], 0
    add byte[ecx], 48
    ;call printabc
    jmp pre_next
jin:
    mov byte[jinwei], 1
    sub byte[ecx], 10
    add byte[ecx], 48
    ;call printabc
    jmp pre_next
    ;mov byte[ecx], 48
    ;call printabc
;before next loop
pre_next:
    dec eax
    dec ebx
    dec ecx
    inc dword[res_len]
    inc dword[jishu_one]
    inc dword[jishu_two]
    jmp add_loop
    
add_finish:
    ;call printabc
    mov esi, dword[add_one_len]
    cmp esi, dword[add_two_len]
    je final_jinwei
    mov esi, dword[add_one_len]
    cmp esi, dword[add_two_len]
    jl two_big
    mov esi, dword[add_one_len]
    sub esi, dword[add_two_len]
;number one more longer loop
one_add_loop:
    cmp esi, 0
    je final_jinwei
    mov dl, byte[eax]
    mov byte[ecx], dl
    cmp byte[jinwei], 0
    je one_bujin
    add byte[ecx], 1
    cmp byte[ecx], 58
    jnl  one_jin
    jmp one_bujin
one_jin:
    mov byte[jinwei], 1
    sub byte[ecx], 10
    dec ecx
    dec esi
    dec eax
    inc dword[res_len]
    jmp one_add_loop
one_bujin:
    dec ecx
    dec esi
    dec eax
    inc dword[res_len]
    mov byte[jinwei], 0
    jmp one_add_loop
two_big:
    mov esi, dword[add_two_len]
    sub esi, dword[add_one_len]
;number two more longer loop
two_add_loop:
    cmp esi, 0
    je final_jinwei

    mov dl, byte[ebx]
    mov byte[ecx], dl

    cmp byte[jinwei], 0
    je two_bujin
    add byte[ecx], 1
    cmp byte[ecx], 58
    jnl  two_jin
    jmp two_bujin
two_jin:
    mov byte[jinwei], 1
    sub byte[ecx], 10
    dec ecx
    dec esi
    dec eax
    inc dword[res_len]
    jmp two_add_loop
two_bujin:
    dec ecx
    dec esi
    dec ebx
    inc dword[res_len]
    mov byte[jinwei], 0
    jmp two_add_loop
;equal and test jinwei
final_jinwei:
    cmp byte[jinwei], 0
    je res_print
    ;call printabc
    mov byte[ecx], 49
    dec ecx
    inc dword[res_len]
res_print:
    inc ecx
    ;add ebx, dword[add_two_len]
    ;mov eax, 4
    ;mov ebx, 1
    ;mov edx, dword[res_len]
    ;int 80h 
    push eax
    mov eax, ecx
    call sprint
    pop eax
    ret

mul_two_number:
    mov eax, input
    add eax, dword[one_len]
    sub eax, 1

    mov ebx, edi
    add ebx, dword[two_len]
    sub ebx, 1

    mov ecx, mul_result
    add ecx, 254

    mov byte[ecx], 0
    dec ecx
    mov byte[ecx], 0Ah
    dec ecx

    ;call printabc
    
    mov byte[jinwei], 0
    mov dword[res_len], 0
    mov dword[mul_jishu], 0
    
mul_loop:
    mov esi, dword[mul_jishu]
    cmp dword[two_len], esi
    je fin_mul

    mov eax, input
    add eax, dword[one_len]
    sub eax, 1

    mov ebx, edi
    add ebx, dword[two_len]
    sub ebx, 1

    mov ecx, mul_result
    add ecx, 252

    sub ebx, dword[mul_jishu]
    sub ecx, dword[mul_jishu]

    ;call printb
    
    mov dword[mul_len], 0
    mov dh, byte[ebx]
    sub dh, 48
num_mul_loop:
    mov esi, dword[mul_len]
    cmp esi, dword[one_len]
    jg fin_num_mul
    mov dl, 0
    cmp esi, dword[one_len]
    je mul_jinwei
    mov dl, byte[eax]
    sub dl, 48
    ;call printabc
    push eax
    mov al, dl
    mul dh
    mov dl, al
    pop eax
    ;call printabc
    ;call printb
mul_jinwei:
    add dl, byte[jinwei]
    add byte[ecx], dl
    mov byte[jinwei], 0
ten_loop:
    cmp byte[ecx], 10
    jl after_ten
    sub byte[ecx], 10
    inc byte[jinwei]
    jmp ten_loop
after_ten:
    cmp esi, dword[one_len]
    je fin_num_mul
    ;call printb
    ;push ecx
    ;mov ecx, jinwei
    ;call printb
    ;pop ecx
    inc dword[mul_len]
    dec eax
    dec ecx
    jmp num_mul_loop
fin_num_mul:
    inc dword[mul_jishu]
    jmp mul_loop
fin_mul:
    mov dl, byte[jinwei]
    add byte[ecx], dl
    mov byte[jinwei], 0
final_ten_loop:
    cmp byte[ecx], 0
    jne final_next
    inc ecx
    jmp after_ten_final
final_next:
    cmp byte[ecx], 10
    jl after_ten_final
    sub byte[ecx], 10
    inc byte[jinwei]
    jmp final_ten_loop
after_ten_final:
    cmp byte[jinwei], 0
    je before_add_loop
    inc ecx
    mov dl, byte[jinwei]
    add byte[ecx], dl
before_add_loop:
    mov eax, ecx
final_add_loop:
    cmp byte[eax], 0Ah
    je mul_res_print
    ;push eax
    add byte[eax], 48
    ;call sprint
    ;pop eax
    inc eax
    ;inc dword[res_len]
    jmp final_add_loop
mul_res_print:
    ;add ebx, dword[add_two_len]
    ;mov eax, 4
    ;mov ebx, 1
    ;mov edx, dword[res_len]
    ;int 80h
    ;inc ecx
    cmp byte[ecx], 48
    jne not_zero
    push eax
    mov eax, zero
    call sprint
    pop eax
    ret
not_zero:
    push eax
    mov eax, ecx
    call sprint
    pop eax   
    ret

;input string
sinput:
    ;读入string
    mov edx, 255
    mov ecx, input
    mov ebx, 0
    mov eax, 3
    int 80h
    ret
    
;calculate length
slen:
    push ebx
    mov ebx,eax

nextchar:
    cmp byte[eax],0
    jz finished
    inc eax ;累加
    jmp nextchar

finished:
    sub eax,ebx
    pop ebx
    ret

;print
sprint: ;put number in ecx
    ;push    eax
    push    edx
    push    ecx
    push    ebx
    push    eax
    call    slen
 
    mov     edx, eax
    pop     eax
 
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
 
    pop     ebx
    pop     ecx
    pop     edx
    ;pop     eax
    ret

printabc:
;print eax
    push eax
    call sprint
    mov eax, huanhang
    call sprint
    pop eax
;print  ebx
    push eax
    mov eax, ebx
    call sprint
    mov eax, huanhang
    call sprint
    pop eax
    
;print ecx
    push eax
    mov eax, ecx
    call sprint
    mov eax, huanhang
    call sprint
    pop eax

    ret
printb:
;output every bits
    push eax
    push ebx
    push edx
    add byte[ecx], 48
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    mov eax, huanhang
    call sprint
    sub byte[ecx], 48
    pop edx
    pop ebx
    pop eax
    ret