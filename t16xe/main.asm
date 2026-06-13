section .code
    org 0x0400
    CALL main
    HLT

inner:
    MOV AX, #89    ; 'Y'
    STB [0xFFF2], AX
    RET

outer:
    CALL inner
    RET

main:
    CALL outer
    MOV AX, #78    ; 'N'
    STB [0xFFF2], AX
    RET