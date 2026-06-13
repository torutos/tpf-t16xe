; kbd.asm — драйвер клавиатуры T16XE

; ========================================
; Данные буфера клавиатуры
; ========================================



; ========================================
; Обработчик INT 0x09
; ========================================
keyboard_handler:
    MOV AX, #88
    STB [0xFFF2], AX
    IRET

; ========================================
; getchar — получить символ (блокирующий)
; Выход: AX = символ
; ========================================

; ========================================
; kbhit — проверить, есть ли символ (не блокирует)
; Выход: flagZ = 1 если буфер пуст
;        AX = символ если есть
; ========================================
kbhit:
    PUSH BX
    PUSH CX
    
    CLI
    MOV BX, kbd_head
    LOAD CX, [BX]
    MOV BX, kbd_tail
    LOAD AX, [BX]
    STI
    
    CMP CX, AX
    JEQ kbhit.empty
    
    MOV BX, kbd_buffer
    ADD BX, AX
    LDB AX, [BX]
    POP CX
    POP BX
    RET
kbhit.empty:
    POP CX
    POP BX
    RET
    
getchar:
    PUSH BX
    PUSH CX
    PUSH DX
    
getchar.wait:
    CLI
    MOV BX, kbd_head
    LOAD CX, [BX]
    MOV BX, kbd_tail
    LOAD DX, [BX]
    STI
    
    CMP CX, DX
    JEQ getchar.wait
    
    MOV BX, kbd_buffer
    ADD BX, DX
    LDB AX, [BX]
    
    CLI
    INC DX
    AND DX, #0x0F
    MOV BX, kbd_tail
    STOR [BX], DX
    STI
    
    POP DX
    POP CX
    POP BX
    RET