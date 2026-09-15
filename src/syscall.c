// file: chiamate_di_sistema.c
#include "syscall.h"
#include "idt.h"
#include "stdf.h"
#include "delay.h"

#define CHIAMATA_STAMPA_CARATTERE 1
#define CHIAMATA_ATTENDI_MILLISECONDI 2

// Eseguita per ogni "int 0x30". Riceve il numero della chiamata e fino
// a tre argomenti, cosi' come li ha messi nei registri il programma in
// ring 3 prima dell'interruzione. Il valore restituito arriva al
// programma chiamante dentro EAX, come una normale funzione C.
unsigned int gestore_chiamata_di_sistema(unsigned int numero_chiamata, unsigned int argomento_1, unsigned int argomento_2, unsigned int argomento_3){
    switch(numero_chiamata){
        case CHIAMATA_STAMPA_CARATTERE:
            print((char) argomento_1);
            return 0;

        case CHIAMATA_ATTENDI_MILLISECONDI:
            waaait(argomento_1);
            return 0;

        default:
            return (unsigned int) -1; // numero di chiamata non riconosciuto
    }
}

void chiamate_di_sistema_inizializza(void){
    // 0xEE = presente, DPL 3, interrupt gate a 32 bit. La differenza
    // rispetto a 0x8E (usato per tastiera e timer) e' solo nei due bit
    // del DPL: qui sono 11 invece di 00. Questo e' l'UNICO motivo per
    // cui un programma in ring 3 puo' eseguire "int 0x30" senza
    // causare un errore di protezione: il processore confronta il
    // proprio livello di privilegio attuale con il DPL scritto in
    // questa voce, e nega l'accesso se il chiamante ha un privilegio
    // insufficiente. Con la tastiera (DPL 0) un programma in ring 3
    // che provasse "int 0x21" otterrebbe un errore, giustamente: le
    // interruzioni hardware non devono poter essere simulate a piacere
    // da codice utente.
    idt_imposta_voce(NUMERO_INTERRUZIONE_CHIAMATA_DI_SISTEMA, (unsigned int) ingresso_chiamata_di_sistema, 0x08, 0xEE);
}