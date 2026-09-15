// file: src/shell.c
#include "shell.h"
#include "stdf.h"
#include "gdt.h"
#include "type_conv.h"

static char buffer_riga[LUNGHEZZA_MASSIMA_COMANDO];

// "volatile" per lo stesso motivo di contatore_millisecondi in
// delay.c: queste variabili vengono scritte dentro un gestore di
// interruzione (la tastiera) e lette nel ciclo principale del
// kernel. Senza volatile il compilatore potrebbe tenerle in un
// registro e non accorgersi che sono cambiate "da sole".
static volatile int lunghezza_riga_corrente = 0;
static volatile int comando_pronto = 0;

typedef void (*funzione_comando)(char *argomenti);

struct voce_comando
{
    char *nome;
    funzione_comando esegui;
    char *descrizione;
};

// Dichiarazioni anticipate: servono perche' tabella_comandi qui sotto
// referenzia queste funzioni prima che il compilatore ne veda il
// corpo completo, piu' in fondo al file.
static void comando_aiuto(char *argomenti);
static void comando_pulisci_schermo(char *argomenti);
static void comando_sconosciuto(char *nome_digitato);
static void stampa_prompt(void);
static void echo(char *stringa);
static void nano(char *nome_file);
static char *estrai_nome_comando(char *riga, char **argomenti);
static int nomi_uguali(const char *a, const char *b);
static void wrapper_regione_utilizzabile(char *place);
static void exec_run(char *starting_idx_of_program);

// Tabella dei comandi disponibili. Aggiungere un comando nuovo vuol
// dire: scrivere la funzione che lo esegue, aggiungere una riga qui.
// Nessun altro punto del file va toccato.
static const struct voce_comando tabella_comandi[] = {
    { "help",  comando_aiuto,           "elenca i comandi disponibili" },
    { "clear", comando_pulisci_schermo, "pulisce lo schermo" },
    { "echo", echo, "stampa a schermo la stringa successiva al comando"},
    { "nano", nano, "crea un file e crea una variabile puntatore al nome dopo nano"},
    { "freemem", wrapper_regione_utilizzabile,"stampa le regioni disponibili di memoria con idx iniziale e dim"},
    { "run", exec_run,"execute the program at index in hex after the run command"}
};

#define NUMERO_COMANDI (sizeof(tabella_comandi) / sizeof(tabella_comandi[0]))

static void exec_run(char *starting_idx_of_program)
{
    fprint("\nstarting program at address %x\n",hex_str_to_n(starting_idx_of_program));
    salta_a_programma_in_ring_3_a_indirizzo(hex_str_to_n(starting_idx_of_program));
}

static void wrapper_regione_utilizzabile(char *place)
{
    stampa_regioni_utilizzabili();
}

static void stampa_prompt(void)
{
    stampa_stringa("prism:#-()> ");
}

static void nano(char *nome_file)
{

}

static void echo(char *stringa)
{
    stampa_stringa(stringa);
    print('\n');    
}


void shell_inizializza(void)
{
    lunghezza_riga_corrente = 0;
    comando_pronto = 0;
    stampa_prompt();
}


void shell_gestisci_carattere(char carattere)
{
    if(carattere == '\n')
    {
        print('\n');
        buffer_riga[lunghezza_riga_corrente] = '\0';
        comando_pronto = 1;
        return;
    }

    // Backspace: toglie l'ultimo carattere sia dal buffer sia dallo
    // schermo. Senza questo controllo, premere Backspace scriverebbe
    // il suo stesso codice nel buffer come se fosse una lettera
    // normale, invece di cancellare quella precedente.
    if(carattere == '\b')
    {
        if(lunghezza_riga_corrente > 0)
        {
            lunghezza_riga_corrente = lunghezza_riga_corrente - 1;
            sposta_cursore_indietro();
        }
        return;
    }

    if(lunghezza_riga_corrente < LUNGHEZZA_MASSIMA_COMANDO - 1)
    {
        buffer_riga[lunghezza_riga_corrente] = carattere;
        lunghezza_riga_corrente++;
        print(carattere);
    }
}


// Divide "nomecomando resto della riga" al primo spazio, modificando
// buffer_riga sul posto (inserisce un '\0' al posto dello spazio).
// *argomenti punta a cio' che resta dopo lo spazio, o alla stringa
// vuota se non c'erano argomenti. Stesso concetto di strtok in C
// standard, riscritto perche' qui strtok non e' disponibile.
static char *estrai_nome_comando(char *riga, char **argomenti)
{
    char *cursore = riga;
    while(*cursore != '\0' && *cursore != ' ')
    {
        cursore++;
    }

    if(*cursore == ' ')
    {
        *cursore = '\0';
        *argomenti = cursore + 1;
    }
    else
    {
        *argomenti = cursore; // vuota: cursore e' gia' sul terminatore
    }

    return riga;
}


// Confronto tra stringhe: non e' disponibile strcmp in ambiente
// freestanding senza libreria standard, quindi la riscriviamo. Stessa
// logica esatta di strcmp: avanza finche' i caratteri combaciano, si
// ferma al primo carattere diverso o al termine di una delle due.
static int nomi_uguali(const char *a, const char *b)
{
    while(*a != '\0' && *b != '\0')
    {
        if(*a != *b)
        {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b; // vero solo se finiscono entrambe nello stesso punto
}


void shell_ciclo_principale(void)
{
    if(!comando_pronto)
    {
        return;
    }

    char *argomenti;
    char *nome_comando = estrai_nome_comando(buffer_riga, &argomenti);

    if(nome_comando[0] != '\0')
    {
        int trovato = 0;
        for(unsigned int indice = 0; indice < NUMERO_COMANDI; indice++)
        {
            if(nomi_uguali(tabella_comandi[indice].nome, nome_comando))
            {
                //esegue
                tabella_comandi[indice].esegui(argomenti);
                trovato = 1;
                break;
            }
        }

        if(!trovato)
        {
            comando_sconosciuto(nome_comando);
        }
    }

    lunghezza_riga_corrente = 0;
    comando_pronto = 0;
    stampa_prompt();
}


static void comando_aiuto(char *argomenti)
{
    for(unsigned int indice = 0; indice < NUMERO_COMANDI; indice++)
    {
        stampa_stringa(tabella_comandi[indice].nome);
        stampa_stringa(" - ");
        stampa_stringa(tabella_comandi[indice].descrizione);
        print('\n');
    }
}


static void comando_pulisci_schermo(char *argomenti)
{
    clear_screen(' ');
}


static void comando_sconosciuto(char *nome_digitato)
{
    stampa_stringa("comando sconosciuto: ");
    stampa_stringa(nome_digitato);
    print('\n');
}