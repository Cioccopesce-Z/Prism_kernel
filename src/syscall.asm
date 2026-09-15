[BITS 32]
global ingresso_chiamata_di_sistema
extern gestore_chiamata_di_sistema

; Punto di ingresso registrato nella IDT per l'interruzione software
; delle syscall. A differenza dei gestori scritti con
; __attribute__((interrupt)), qui serve accesso diretto ai registri
; generali: per questo e' scritto a mano.
ingresso_chiamata_di_sistema:
    pusha                 ; salva EAX, ECX, EDX, EBX, ESP originale, EBP, ESI, EDI

    push edx               ; terzo argomento per il gestore in C
    push ecx               ; secondo argomento
    push ebx               ; primo argomento
    push eax               ; numero della chiamata di sistema

    call gestore_chiamata_di_sistema
    add esp, 16             ; rimuove i quattro argomenti appena impilati

    mov [esp + 28], eax      ; sovrascrive l'EAX salvato da pusha con il
                              ; valore restituito dal gestore, cosi' popa
                              ; lo ripristinera' come valore di ritorno
                              ; per il programma in ring 3

    popa
    iret                      ; torna al programma in ring 3, ripristinando
                               ; anche il suo stack originale   