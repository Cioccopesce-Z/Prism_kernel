# ================================================================
# Makefile del kernel
# ================================================================
#
# Compila boot.asm (bootloader), kernel.asm (punto di ingresso) e
# TUTTI i file .c dentro ./src, senza bisogno di elencarli a mano.
#
# In più: se un file .c include un header .h, e quell'header viene
# modificato, il file .c che lo include viene ricompilato in automatico
# alla prossima "make" (non serve fare "make clean" ogni volta che
# tocchi un .h). I file .h di per se' non vengono "compilati": in C
# un header non produce mai un proprio file oggetto, viene solo
# incollato nel .c che lo include tramite #include; qui viene solo
# tracciato come dipendenza, per sapere quando ricompilare.
#
# Aggiungere un nuovo file .c o .h dentro ./src non richiede NESSUNA
# modifica a questo Makefile.

# Elenco di tutti i sorgenti C dentro ./src. Ricalcolato ogni volta
# che lanci "make" cercando davvero sul disco, non è testo fisso:
# per questo un nuovo file .c viene trovato da solo.
SOURCES_C = $(wildcard ./src/*.c)

# Percorso del file .o corrispondente a ciascun sorgente C
# (./src/nome.c diventa ./build/nome.o).
OBJECTS_C = $(patsubst ./src/%.c, ./build/%.o, $(SOURCES_C))

# Percorso del file .d (dipendenze) corrispondente a ciascun
# sorgente C. E' un file di testo generato in automatico dal
# compilatore (vedi il flag -MMD piu' sotto), che elenca da quali
# header dipende ciascun .c: e' quello che rende possibile il
# rebuild automatico quando cambi solo un header.
DEPENDS_C = $(patsubst ./src/%.c, ./build/%.d, $(SOURCES_C))

# Oggetto del punto di ingresso in assembly. Resta indicato a mano,
# a differenza dei file C, perche' e' unico e ha un ruolo speciale:
# deve essere il primo codice eseguito dal kernel.
OBJECT_ENTRY_ASM = ./build/kernel.asm.o
OBJECT_PRIVILEGE_ASM = ./build/cambio_di_privilegio.asm.o
OBJECT_SYSCALL_ASM = ./build/syscall.asm.o


# Oggetto finale, risultato dell'unione di tutti gli oggetti sopra.
OBJECT_KERNEL_COMPLETE = ./build/completeKernel.o

# Opzioni di compilazione del C.
#   -g               informazioni di debug	
#   -ffreestanding   nessuna assunzione sulla presenza di un sistema
#                    operativo sotto (niente libc implicita)
#   -nostdlib        non collegare le librerie standard C
#   -nostartfiles    non usare i file di avvio standard (li sostituisce
#                    kernel.asm)
#   -nodefaultlibs   non collegare nessuna libreria di default
#   -Wall            mostra tutti gli avvisi del compilatore
#   -O0              nessuna ottimizzazione, piu' semplice da debuggare
#   -Iinc            cerca gli header anche dentro ./inc
COMPILER_FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -mgeneral-regs-only -msoft-float

# Regola predefinita: quella eseguita da "make" o "make all" senza
# argomenti.
all: ./bin/os.bin

# ----------------------------------------------------------------
# Regola "pattern": insegna a make come ottenere un qualsiasi file
# ./build/NOME.o a partire da ./src/NOME.c, senza una riga separata
# per ciascun file. Quando serve ./build/qualcosa.o e make trova
# ./src/qualcosa.c, applica questa ricetta automaticamente.
#
#   $<  = il primo prerequisito (qui: il file .c di partenza)
#   $@  = il file che questa regola sta generando (qui: il file .o)
#   | ./build  = prerequisito "order-only": assicura che la cartella
#                ./build esista prima di scrivere dentro, senza pero'
#                far scattare una ricompilazione ogni volta solo
#                perche' la cartella esiste gia'.
#   -MMD  = oltre al file .o, genera anche un file .d con l'elenco
#           degli header da cui questo .c dipende.
#   -MP   = dentro quel file .d, aggiunge anche una regola "vuota"
#           per ogni header elencato: evita un errore di make se in
#           futuro rinomini o cancelli un header senza aggiornare
#           subito tutti i file che lo includevano.
# ----------------------------------------------------------------
./build/%.o: ./src/%.c | ./build
	i686-elf-gcc -I./src $(COMPILER_FLAGS) -std=gnu99 -MMD -MP -c $< -o $@

# Assembla il punto di ingresso del kernel.
$(OBJECT_ENTRY_ASM): ./src/kernel.asm | ./build
	nasm -f elf -g ./src/kernel.asm -o $(OBJECT_ENTRY_ASM)

$(OBJECT_PRIVILEGE_ASM): ./src/cambio_di_privilegio.asm | ./build
	nasm -f elf -g ./src/cambio_di_privilegio.asm -o $(OBJECT_PRIVILEGE_ASM)
$(OBJECT_SYSCALL_ASM): ./src/syscall.asm | ./build
	nasm -f elf -g ./src/syscall.asm -o $(OBJECT_SYSCALL_ASM)


# Unisce il punto di ingresso in assembly con TUTTI gli oggetti C in
# un unico oggetto rilocabile. $(OBJECTS_C) contiene sempre l'elenco
# aggiornato, calcolato da $(wildcard ...) in cima al file.
$(OBJECT_KERNEL_COMPLETE): $(OBJECT_ENTRY_ASM) $(OBJECT_PRIVILEGE_ASM) $(OBJECT_SYSCALL_ASM) $(OBJECTS_C)
	i686-elf-ld -g -relocatable $(OBJECT_ENTRY_ASM) $(OBJECT_PRIVILEGE_ASM) $(OBJECT_SYSCALL_ASM) $(OBJECTS_C) -o $(OBJECT_KERNEL_COMPLETE)


# Produce il binario finale del kernel, usando lo script del linker
# per decidere indirizzi e ordine delle sezioni in memoria.
#   -lgcc  collega libgcc: la libreria interna del compilatore che
#          implementa operazioni che la CPU x86 a 32 bit non sa fare
#          in una singola istruzione, come dividere due numeri a 64
#          bit (__udivdi3, __umoddi3). Non e' legata a nessun sistema
#          operativo, quindi resta valida anche con -nostdlib.
./bin/kernel.bin: $(OBJECT_KERNEL_COMPLETE) | ./bin
	i686-elf-gcc $(COMPILER_FLAGS) -T ./src/linkerScript.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib $(OBJECT_KERNEL_COMPLETE) -lgcc


# Assembla il bootloader (settore di avvio, 512 byte).
./bin/boot.bin: ./src/boot.asm | ./bin
	nasm -f bin ./src/boot.asm -o ./bin/boot.bin

# Immagine finale del disco: bootloader + kernel + riempimento.
# Cancello prima os.bin esistente, altrimenti "dd ... >>" continuerebbe
# ad accodare contenuto a un file gia' esistente da build precedenti,
# facendolo crescere ogni volta invece di sovrascriverlo.
./bin/os.bin: ./bin/boot.bin ./bin/kernel.bin
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=8 >> ./bin/os.bin

# Crea le cartelle di lavoro se non esistono ancora.
./build ./bin:
	mkdir -p $@

# Include i file .d generati durante la compilazione, se esistono.
# Il "-" davanti a "include" dice a make di non lamentarsi se questi
# file non esistono ancora (per esempio al primissimo "make", prima
# che sia mai stato compilato nulla).
-include $(DEPENDS_C)

clean:
	rm -f ./bin/boot.bin
	rm -f ./bin/kernel.bin
	rm -f ./bin/os.bin
	rm -f ./build/*.o
	rm -f ./build/*.d