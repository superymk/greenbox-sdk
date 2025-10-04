.code _text

win_HALT PROC
    ud2
ret
win_HALT ENDP

Intel_VMCALL_ret4 PROC 
; fn --> rcx
; ret_cx --> rdx
; ret_dx --> r8
; ret_si --> r9
; ret_di --> stack

    push rbp
    mov rbp, rsp

    push rsi
    push rdi
    push rbx        ; backup non-volatile registers, see https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170#parameter-passing

    push rdx
    push r8
    push r9

    mov rax, rcx    ; move fn to rax

    mov rbx, rdx    
    mov rcx, [rbx]  ; move ret_cx to rcx

    mov rbx, r8
    mov rdx, [rbx]  ; move ret_dx to rdx

    mov rbx, r9
    mov rsi, [rbx]  ; move ret_si to rsi

    mov rbx, [rbp + 30h]
    mov rdi, [rbx]  ; move ret_di to rdi

    
    vmcall

    mov rbx, [rbp + 30h]    ; Why needs an offset of 0x30? See Windows x64 calling convention: Stack frame at https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/windows-x64-calling-convention-stack-frame
    mov [rbx], rdi  ; move rdi to ret_di

    pop r9
    mov rbx, r9
    mov [rbx], rsi  ; move rsi to ret_si

    pop r8
    mov rbx, r8
    mov [rbx], rdx  ; move rdx to ret_dx

    pop rdx
    mov rbx, rdx
    mov [rbx], rcx  ; move rcx to ret_cx

    pop rbx
    pop rdi
    pop rsi

    pop rbp
ret
Intel_VMCALL_ret4 ENDP


END