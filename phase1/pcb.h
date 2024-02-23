#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

#include "../lib/pandos_types.h"


/**
 * Inizializza pcbFree_h in modo che questa contenga tutti
 * gli elementi di pcbFree_table[]. Viene chiamato solo
 * una volta in fase di inizializzazione della struttura.
 */
void initPcbs();


/**
 * Inserisce *p in pcbFree_h.
 */
void freePcb(pcb_t *p);


/**
 * NULL se pcbFree_h è vuota, altrimenti rimuove un PCB
 * da pcbFree_h, inizializza tutti i suoi campi con NULL o 0
 * e ritorna tale PCB.
 */
pcb_t *allocPcb();


/**
 * Ritorna il PCB che ha il PID specificato.
 * Se tale PCB non esiste o è stato deallocato, ritorna NULL.
 */
pcb_t *getPcb(int pid);


/**
 * TRUE se *p è un PCB attualmente attivo, FALSE altrimenti.
 */
int isPcbActive(pcb_t *p);


/**
 * Inizializza *head come lista di PCB vuota.
 */
void mkEmptyProcQ(struct list_head *head);


/**
 * TRUE se *head è vuota, FALSE altrimenti.
 */
int emptyProcQ(struct list_head *head);


/**
 * Inserisce *p in *head.
 */
void insertProcQ(struct list_head *head, pcb_t *p);


/**
 * Ritorna l'elemento di testa in *head senza rimuoverlo,
 * oppure NULL se la coda è vuota.
 */
pcb_t *headProcQ(struct list_head *head);


/**
 * Rimuove il primo elemento da *head.
 * Ritorna NULL se la coda è vuota, altrimenti
 * ritorna il puntatore all'elemento rimosso dalla lista.
 */
pcb_t *removeProcQ(struct list_head *head);


/**
 * Rimuove *p dalla process queue puntata da head.
 * Se *p non è presente nella coda, ritorna NULL.
 */
pcb_t *outProcQ(struct list_head *head, pcb_t *p);


/**
 * TRUE se *p non ha figli, FALSE altrimenti.
 */
int emptyChild(pcb_t *p);


/**
 * Inserisce *p nei figli di *prnt.
 */
void insertChild(pcb_t *prnt, pcb_t *p);


/**
 * Rimuove e ritorna il PCB in testa alla lista dei figli di *p.
 * Se *p non ha figli, restituisce NULL.
 */
pcb_t *removeChild(pcb_t *p);


/**
 * Rimuove *p dalla lista dei figli del suo padre.
 * Se *p non ha un padre, ritorna NULL.
 * Altrimenti restituisce l'elemento rimosso (cioè p stesso).
 */
pcb_t *outChild(pcb_t *p);


#endif