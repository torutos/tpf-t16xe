print_char:
    STOR [0xFFF2], AX
    RET

print_str:
    LDB AX, [BX]
    STOR [0xFFF2], AX
    RET

print_string:
    PUSH AX
    MOV AX, #80   ; 'P'
    STB [0xFFF2], AX
    POP AX
    ; дальше обычный код
    LDB AX, [BX]
    CMP AX, #0
    JEQ print_string.done
    STOR [0xFFF2], AX
    INC BX
    JMP print_string
print_string.done:
    RET

strlen:
    MOV CX, #0
strlen.loop:
    LDB AX, [BX]        ; читаем один байт
    CMP AX, #0
    JEQ strlen.done
    INC CX
    INC BX
    JMP strlen.loop
strlen.done:
    RET
    
    memcpy:
    CMP DX, #0
    JEQ memcpy.done
memcpy.loop:
    LDB AX, [BX]
    STB [CX], AX
    INC BX
    INC CX
    DEC DX
    JNE memcpy.loop
memcpy.done:
    RET