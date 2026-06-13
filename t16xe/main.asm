section .code
    org 0x0400
    CALL main
    HLT

print_digit:
    MOV CX, #0
pd_loop:
    CMP AX, BX
    JCC pd_done
    SUB AX, BX
    INC CX
    JMP pd_loop
pd_done:
    MOV AX, CX
    ADD AX, #48
    STB [0xFFF2], AX
    RET

main:
    MOV AX, #47
    MOV BX, #10
    CALL print_digit
    MOV AX, #88       ; 'X' — отладка
    STB [0xFFF2], AX
    RET