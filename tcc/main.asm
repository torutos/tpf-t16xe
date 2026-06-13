section .code
    org 0x0400
    CALL main
    HLT

mul:
    ; AX = AX * BX (простое умножение сложением)
    PUSH CX
    MOV CX, AX
    MOV AX, #0
mul_loop:
    CMP BX, #0
    JEQ mul_done
    ADD AX, CX
    DEC BX
    JMP mul_loop
mul_done:
    POP CX
    RET

print_int:
    ; вывод числа из AX (0-999)
    PUSH BX
    PUSH CX
    PUSH DX
    MOV BX, #100
    CALL print_digit
    MOV BX, #10
    CALL print_digit
    MOV BX, #1
    CALL print_digit
    POP DX
    POP CX
    POP BX
    RET

print_digit:
    ; выводит (AX / BX) % 10 как цифру
    MOV CX, #0
pd_loop:
    CMP AX, BX
    JCC pd_done
    SUB AX, BX
    INC CX
    JMP pd_loop
pd_done:
    PUSH AX
    MOV AX, CX
    ADD AX, #48
    STB [0xFFF2], AX
    POP AX
    RET

    main:
    MOV AX, #5
    PUSH AX
    MOV AX, #42
    POP BX
    ADD AX, BX
    CALL print_int
    MOV AX, #0
    RET

