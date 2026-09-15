// file: gdt.c
#include "gdt.h"
#include "loc.h"

#define NUMERO_DI_VOCI_NELLA_GDT 6

static voce_della_gdt tabella_gdt[NUMERO_DI_VOCI_NELLA_GDT];
static puntatore_alla_gdt puntatore_gdt;
static task_state_segment tss;

// La differenza rispetto a idt_imposta_voce e' che qui i parametri
// sono base e limite di un intervallo di memoria (o, per la TSS,
// indirizzo e dimensione di una struttura), non l'indirizzo di una
// funzione: il formato del descrittore GDT e' fatto per descrivere
// segmenti di memoria, quello IDT per descrivere gestori.
static void gdt_imposta_voce(int indice, unsigned int base, unsigned int limite, unsigned char byte_di_accesso, unsigned char flag)
{
    tabella_gdt[indice].limite_parte_bassa = limite & 0xFFFF;
    tabella_gdt[indice].limite_alta_e_flag = ((limite >> 16) & 0x0F) | (flag & 0xF0);
    tabella_gdt[indice].base_parte_bassa = base & 0xFFFF;
    tabella_gdt[indice].base_parte_media = (base >> 16) & 0xFF;
    tabella_gdt[indice].base_parte_alta = (base >> 24) & 0xFF;
    tabella_gdt[indice].byte_di_accesso = byte_di_accesso;
}

void gdt_inizializza(void)
{
    // Voce 0: descrittore nullo, obbligatorio in prima posizione,
    // vincolo hardware. Il processore lo tratta come "segmento non
    // valido" quando un selettore viene azzerato.
    gdt_imposta_voce(0, 0, 0, 0, 0);

    // Voci 1 e 2: codice e dati kernel, ring 0. Stessi valori gia'
    // usati in boot.asm: da qui in poi e' questa GDT, non quella del
    // bootloader, a restare attiva.
    gdt_imposta_voce(1, 0, 0xFFFFFFFF, 0x9A, 0xC0);
    gdt_imposta_voce(2, 0, 0xFFFFFFFF, 0x92, 0xC0);

    // Voci 3 e 4: codice e dati utente, ring 3. Stessa area di
    // memoria del kernel (tutti i 4 GB: senza paging attivo non c'e'
    // modo di restringerla), unica differenza il DPL nel byte di
    // accesso, 11 invece di 00.
    gdt_imposta_voce(3, 0, 0xFFFFFFFF, 0xFA, 0xC0);
    gdt_imposta_voce(4, 0, 0xFFFFFFFF, 0xF2, 0xC0);

    // Voce 5: la TSS. Qui base e limite non descrivono un'area
    // generica ma puntano esattamente alla struttura "tss" qui sopra.
    gdt_imposta_voce(5, (unsigned int) &tss, sizeof(tss) - 1, 0x89, 0x00);

    // Azzera la TSS prima di usarla: solo ss0 ed esp0 avranno un
    // valore significativo, ma partire da una struttura pulita evita
    // che campi non inizializzati contengano valori casuali.
    unsigned char *puntatore_byte = (unsigned char*) &tss;
    for(unsigned int indice = 0; indice < sizeof(tss); indice++){
        puntatore_byte[indice] = 0;
    }
    tss.ss0 = 0x10; // selettore dati kernel: lo stack che il processore usera' in ring 0

    puntatore_gdt.dimensione_tabella_meno_uno = sizeof(tabella_gdt) - 1;
    puntatore_gdt.indirizzo_tabella = (unsigned int) &tabella_gdt;

    __asm__ volatile ("lgdt %0" : : "m"(puntatore_gdt));

    // "lgdt" dice al processore dove si trova la tabella, ma non
    // ricarica da sola i registri di segmento gia' in uso: quello va
    // fatto esplicitamente, ed e' piu' sicuro farlo in assembly puro
    // (vedi sotto) che con assembly incorporato in C, per via del
    // salto lontano necessario per ricaricare CS.
    ricarica_segmenti_dopo_gdt();
}

void tss_imposta_stack_kernel(unsigned int indirizzo_esp0)
{
    tss.esp0 = indirizzo_esp0;
}

void salta_a_programma_in_ring_3_a_indirizzo(unsigned int idx)
{
    void *stack_utente = alloc(4096);
    salta_in_ring_3(idx, (unsigned int) stack_utente + 4096);
}

