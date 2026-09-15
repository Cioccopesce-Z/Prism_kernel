#include "kernel.h"

#include "delay.h"
#include "idt.h"
#include "shell.h"
#include "loc.h"
#include "gdt.h"
#include "mem.h"
#include "mappa_memoria.h"
#include "stdf.h"

/* memoria vera e propria: allocata da begin(), dichiarata qui perche'
   e' il file-main che decide dimensioni e vive per tutta l'esecuzione. */
__uint8_t *memory;
size_t memory_cursor = 0;

/* configurazione della struttura: modificabile qui prima di begin(). */
size_t byte_for_lenght_of_the_lenght_of_the_scope = 1;
size_t byte_for_scope_code = 1;

size_t byte_for_scope = 0;
size_t byte_for_dim = 1;
size_t byte_for_vleng = 1;
size_t byte_for_method_lenght = 1;

size_t exit_code;

int kernel_main(int exit_call) <%

    if(exit_call){
        return exit_code;
    }
    clear_screen(' ');
    
    void *stack_kernel_per_ring3 = alloc(4096);
    tss_imposta_stack_kernel((unsigned int) stack_kernel_per_ring3 + 4096);

    idt_inizializza();
    temporizzatore_inizializza();
    mappa_memoria_inizializza();
    gdt_inizializza();

    present();

    stampa_regioni_utilizzabili();
    shell_inizializza();
    
    
    while(1) {
        shell_ciclo_principale();
        __asm__ volatile ("hlt");
    }
%>
