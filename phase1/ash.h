#ifndef ASH_H_INCLUDED
#define ASH_H_INCLUDED

#include "../lib/pandos_types.h"
#include "../lib/hashtable.h"
#include "pcb.h"


/**
 * Inserisce *p nella coda dei processi bloccati
 * associata al SEMD con chiave semAdd.
 * Se il semaforo corrispondente non è presente in semd_h,
 * alloca un nuovo SEMD da quelli in semdFree_h e lo inserisce
 * in semd_h, settando i campi in maniera opportuna
 * (i.e. key e s_procQ).
 * Se non si può allocare un nuovo SEMD (i.e. quando
 * semdFree_h è vuota) ritorna TRUE. In tutti gli
 * altri casi ritorna FALSE.
 */
int insertBlocked(int *semAdd, pcb_t *p);


/**
 * Ritorna il primo PCB della coda dei processi bloccati
 * (s_procQ) associata al SEMD di semd_h con chiave semAdd.
 * Se tale descrittore non esiste in semd_h, ritorna NULL.
 * Altrimenti, restituisce l'elemento rimosso.
 * Se la coda dei processi bloccati per il semaforo diventa
 * vuota, rimuove il descrittore corrispondente da semd_h e
 * lo inserisce in semdFree_h.
 */
pcb_t *removeBlocked(int *semAdd);


/**
 * Rimuove *p dalla coda del semaforo su cui è bloccato
 * (indicato da p->p_semAdd). Se *p non compare in tale
 * coda, allora restituisce NULL (condizione di errore).
 * Altrimenti restituisce *p stesso.
 * Se la coda dei processi bloccati per il semaforo diventa
 * vuota, rimuove il descrittore corrispondente da semd_h e
 * lo inserisce in semdFree_h.
 */
pcb_t *outBlocked(pcb_t *p);


/**
 * Restituisce SENZA RIMUOVERLO il puntatore al PCB che si
 * trova in testa alla coda dei processi associata al SEMD
 * con chiave semAdd. Ritorna NULL se il SEMD non compare
 * in semd_h oppure se compare ma la sua coda dei processi
 * è vuota.
 */
pcb_t *headBlocked(int *semAdd);


/**
 * Inizializza semdFree_h in modo che contenga tutti gli
 * elementi di semd_table[]. Questo metodo viene invocato una
 * volta sola durante l'inizializzazione della struttura dati.
 */
void initASH();


#endif