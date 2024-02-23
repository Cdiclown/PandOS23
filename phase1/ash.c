#include "ash.h"

static semd_t semd_table[MAXPROC];
static LIST_HEAD(semdFree_h);
static DEFINE_HASHTABLE(semd_h, 5);     // grandezza 5 = 32 slot


/**
 * Alloca un nuovo semaforo dalla lista semdFree_h,
 * assegnandogli la chiave semAdd, e lo inserisce in semd_h.
 * Se non è possibile allocare un semaforo ritorna NULL,
 * altrimenti ritorna il semaforo appena allocato.
 */
static inline semd_t *__allocSemd(int *semAdd) {
    if (list_empty(&semdFree_h))
        return NULL;
    semd_t *sem = list_first_entry(&semdFree_h, semd_t, s_freelink);

    // inizializzazione dei campi di *sem
    list_del(&sem->s_freelink);
    sem->s_key = semAdd;
    INIT_LIST_HEAD(&sem->s_procq);
    hash_add(semd_h, &sem->s_link, (u32)semAdd);    // aggiunge *sem alla hashtable associandogli la chiave semAdd

    return sem;
}


/**
 * Cerca all'interno della hashtable semd_h
 * il semaforo al quale è associata la chiave semAdd
 * e se presente lo restituisce senza rimuoverlo,
 * altrimenti ritorna NULL.
 */
static inline semd_t *__scanHash(int *semAdd) {
    semd_t *iter = NULL;
    hash_for_each_possible(semd_h, iter, s_link, (u32)semAdd) {
        if (iter->s_key == semAdd)
            return iter;
    }
    return NULL;
}


/**
 * Dealloca il semaforo *sem, ma solo se la sua lista
 * di processi bloccati è vuota.
 */
static inline void __ifEmptyDeallocSemd(semd_t *sem) {
    if (list_empty(&sem->s_procq)) {
        hash_del(&sem->s_link);
        list_add_tail(&sem->s_freelink, &semdFree_h);
    }
}


int insertBlocked(int *semAdd, pcb_t *p) {
    semd_t *sem = __scanHash(semAdd);
    if (sem == NULL) {                  // se non esiste semaforo con chiave semAdd:
        if (list_empty(&semdFree_h))    // nel caso in cui semdFree_h è vuota, ritorna true
            return true;
        sem = __allocSemd(semAdd);      // altrimenti alloca un semaforo nuovo
    }

    insertProcQ(&sem->s_procq, p);
    p->p_semAdd = semAdd;
    return false;
}


pcb_t *removeBlocked(int *semAdd) {
    semd_t *sem = __scanHash(semAdd);
    if (sem == NULL)
        return NULL;

    pcb_t *p = removeProcQ(&sem->s_procq);
    p->p_semAdd = NULL;
    __ifEmptyDeallocSemd(sem);          // se la coda è vuota dealloca il semaforo
    return p;
}


pcb_t *outBlocked(pcb_t *p) {
    semd_t *sem = __scanHash(p->p_semAdd);
    if (sem == NULL)
        return NULL;

    pcb_t *ret = outProcQ(&sem->s_procq, p);
    p->p_semAdd = NULL;
    __ifEmptyDeallocSemd(sem);
    return ret;
}


pcb_t *headBlocked(int *semAdd) {
    semd_t *sem = __scanHash(semAdd);
    if (sem == NULL)
        return NULL;

    pcb_t *p = headProcQ(&sem->s_procq);
    __ifEmptyDeallocSemd(sem);
    return p;
}


void initASH() {
    INIT_LIST_HEAD(&semdFree_h);
    for (int i = 0; i < MAXPROC; i++) {
        list_add_tail(&semd_table[i].s_freelink, &semdFree_h);
    }
}