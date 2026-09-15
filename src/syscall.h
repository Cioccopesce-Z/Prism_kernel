// file: chiamate_di_sistema.h
#ifndef SYSCALL_H
#define SYSCALL_H

extern void ingresso_chiamata_di_sistema(void);

// Numero dell'interruzione software dedicata. 0x30 e' una scelta
// libera (Linux storicamente usa 0x80): l'unico vincolo e' non
// entrare in conflitto con le eccezioni del processore (0-31) o con
// le interruzioni hardware gia' rimappate (0x20-0x2F).
#define NUMERO_INTERRUZIONE_CHIAMATA_DI_SISTEMA 0x30

void chiamate_di_sistema_inizializza(void);

#endif