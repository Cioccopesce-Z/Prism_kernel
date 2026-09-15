// file: gdt.h
#ifndef GDT_H
#define GDT_H

// Un descrittore della GDT occupa sempre 8 byte: formato imposto dal
// processore, identico nello spirito alla voce della IDT che hai gia'
// in idt.h. La suddivisione della base in tre pezzi separati e' la
// stessa scomodita' storica gia' vista li': va presa cosi' com'e'.
typedef struct {
    unsigned short limite_parte_bassa;
    unsigned short base_parte_bassa;
    unsigned char  base_parte_media;
    unsigned char  byte_di_accesso;
    unsigned char  limite_alta_e_flag;
    unsigned char  base_parte_alta;
} __attribute__((packed)) voce_della_gdt;

typedef struct {
    unsigned short dimensione_tabella_meno_uno;
    unsigned int   indirizzo_tabella;
} __attribute__((packed)) puntatore_alla_gdt;

// Struttura della TSS. La dimensione e la posizione di ogni campo sono
// fissate dal processore; qui verranno effettivamente usati solo ss0
// ed esp0 (vedi spiegazione sopra), ma gli altri campi devono comunque
// esistere ed essere al loro posto esatto, altrimenti il processore
// leggerebbe dati a caso quando consulta la struttura.
typedef struct {
    unsigned int collegamento_task_precedente;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax, ecx, edx, ebx;
    unsigned int esp, ebp, esi, edi;
    unsigned int es, cs, ss, ds, fs, gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short base_mappa_io;
} __attribute__((packed)) task_state_segment;

void gdt_inizializza(void);
extern void ricarica_segmenti_dopo_gdt(void);
extern void salta_in_ring_3();


// Da richiamare ogni volta che si vuole cambiare lo stack di ring 0
// usato per gestire le interruzioni che arrivano da ring 3. Utile fin
// da subito con un solo programma; indispensabile piu' avanti quando
// ogni processo avra' il proprio stack kernel dedicato.
void tss_imposta_stack_kernel(unsigned int indirizzo_esp0);
void salta_a_programma_in_ring_3_a_indirizzo(unsigned int idx);

#endif