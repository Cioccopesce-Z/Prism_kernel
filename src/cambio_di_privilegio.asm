[BITS 32]

global ricarica_segmenti_dopo_gdt
global salta_in_ring_3

; Da chiamare subito dopo "lgdt" dal lato C. Il motivo per cui questo
; pezzo resta in assembly e non in C incorporato: CS non si puo'
; ricaricare con un normale "mov" (il processore lo impedisce, vincolo
; hardware), l'unico modo e' un salto. E deve essere un salto "lontano"
; (con selettore esplicito), perche' altrimenti il processore
; continuerebbe a eseguire le istruzioni gia' pre-lette secondo la
; vecchia GDT invece che secondo quella nuova appena caricata.
ricarica_segmenti_dopo_gdt:
    jmp 0x08:.dopo_il_salto
.dopo_il_salto:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ax, 0x28       ; selettore della TSS: indice 5 della GDT, 5*8 = 0x28
    ltr ax              ; "ltr" e' l'istruzione dedicata a caricare la TSS,
                         ; diversa da un mov perche' la TSS non e' un
                         ; segmento di dati qualsiasi

    ret

; salta_in_ring_3(indirizzo_punto_di_ingresso, indirizzo_stack_utente)
; Argomenti secondo la convenzione cdecl di GCC: si leggono da
; [esp+4] e [esp+8], perche' in [esp+0] c'e' l'indirizzo di ritorno di
; questa chiamata, che qui pero' non verra' mai usato: la funzione non
; torna con una "ret" normale.
salta_in_ring_3:
    mov eax, [esp+4]    ; punto di ingresso del programma
    mov ebx, [esp+8]    ; cima dello stack che il programma usera'

    ; "iret" si aspetta questi cinque valori impilati in un ordine
    ; preciso imposto dall'hardware: EIP, CS, EFLAGS, ESP, SS (letti
    ; dal basso verso l'alto della pila, quindi impilati qui in ordine
    ; inverso). Il fatto che ripristini anche SS ed ESP in un solo
    ; passo, e non solo EIP come una "ret" normale, e' cio' che rende
    ; possibile cambiare stack durante lo stesso cambio di privilegio:
    ; e' l'unico modo per farlo atomicamente.
    push 0x23           ; selettore dati utente, indice 4, RPL 3 (0x20 | 3)
    push ebx
    pushf
    push 0x1B            ; selettore codice utente, indice 3, RPL 3 (0x18 | 3)
    push eax

    ; Siccome il CS impilato ha RPL 3, il processore abbassa il
    ; proprio privilegio a ring 3 nello stesso identico istante in cui
    ; esegue questo salto: non c'e' un momento intermedio in cui sia
    ; "un po' ring 0 e un po' ring 3".
    iret