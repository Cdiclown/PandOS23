#include "initial.h"

pcb_t *currentProc;                  /* puntatore al processo corrente      */
int processCount;                    /* contatore dei procesi vivi          */
int softBlockCount;                  /* contatore dei processi soft-blocked */
struct list_head readyQueue;         /* lista di processi in attesa         */
int itSemaphore;                     /* semaforo per l'interval timer       */
int deviceSemaphore[IO_DEVICES_NUM]; /* semafori per i device I/O           */

extern void test();                  /* la funzione di test da invocare     */
extern void uTLB_RefillHandler();    /* dummy handler per TLB-Refill        */


/**
 * Implementazione di memcpy.
 */
void memcpy(void *dest, const void *src, unsigned int len) {
    char *d = (char *)dest;
    const char *s = (char *)src;

    for (unsigned int i = 0; i < len; i++)
        d[i] = s[i];
}


/**
 * Ritorna il tempo trascorso tra la sua precedente invocazione e quella attuale.
 */
cpu_t getTimespan() {
    static cpu_t spanStart = 0;
    cpu_t timespan;

    STCK(timespan);
    timespan -= spanStart;
    STCK(spanStart);
    return timespan;
}


/**
 * Incrementa il tempo di esecuzione del currentProc tramite getTimespan().
 * Utilizzata per tenere aggiornato il tempo di esecuzione dei processi.
 */
void updateCurrentProcTime() {
    if (currentProc != NULL)
        currentProc->p_time += getTimespan();
}


/**
 * TRUE se *p è bloccato su un semaforo di I/O, FALSE altrimenti.
 */
bool isSoftBlocked(pcb_t *p) {
    if (p == NULL)
        return false;
    
    if (p->p_semAdd == &itSemaphore)
        return true;
    
    for (int i = 0; i < IO_DEVICES_NUM; i++) {
        if (p->p_semAdd == &deviceSemaphore[i])
            return true;
    }
    
    return false;
}


/**
 * Effettua una P sul semaforo con chiave semaddr, eventualmente bloccando il currentProc.
 */
void P(int *semaddr) {
    if (*semaddr == 0) {                            /* semaforo bloccato                        */
        currentProc->p_s = *SAVED_STATE;
        if (insertBlocked(semaddr, currentProc))
            PANIC();                                /* PANIC se la ASH ha esaurito i semafori   */
        updateCurrentProcTime();
        scheduler();
    }
    else if (headBlocked(semaddr) != NULL) {        /* semaforo libero ma con processi bloccati */
        pcb_t *readyProc = removeBlocked(semaddr);
        if (isPcbActive(readyProc))
            insertProcQ(&readyQueue, readyProc);
    }
    else                                            /* semaforo libero senza processi           */
        *semaddr = 0;
}


/**
 * Effettua una V sul semaforo con chiave semaddr, eventualmente bloccando il currentProc.
 */
void V(int *semaddr) {
    if (*semaddr == 1) {
        currentProc->p_s = *SAVED_STATE;
        if (insertBlocked(semaddr, currentProc))
            PANIC();
        updateCurrentProcTime();
        scheduler();
    }
    else if (headBlocked(semaddr) != NULL) {
        pcb_t *readyProc = removeBlocked(semaddr);
        if (isPcbActive(readyProc))
            insertProcQ(&readyQueue, readyProc);
    }
    else
        *semaddr = 1;
}


/**
 * Prende in input l'indirizzo del registro di un device I/O,
 * ritorna la chiave del suo semaforo.
 */
int *getDeviceSem(int cmdAddr) {
    if (cmdAddr < DEV_REG_START || cmdAddr >= DEV_REG_END)
        return NULL;

    // trova l'indice del device in base all'offset rispetto all'indirizzo base
    int semIndex = (cmdAddr - DEV_REG_START) / DEV_REG_SIZE;

    /* I terminali hanno device register di dimensione dimezzata
       quindi se l'offset non è multiplo di 16 è un terminale transmitter.
       Terminali receiver: indici 32-39, terminali transmitter: indici 40-48. */
    if (cmdAddr >= DEV_REG_ADDR(7, 0) &&
        (cmdAddr - DEV_REG_START) % DEV_REG_SIZE >= DEV_REG_SIZE / 2)
        semIndex += N_DEV_PER_IL;
    
    return &deviceSemaphore[semIndex];
}


void main() {
    // inizializzazione delle strutture della phase 1
    initPcbs();
    initASH();
    initNamespaces();

    // inizializzazione delle variabili globali
    mkEmptyProcQ(&readyQueue);
    currentProc    = NULL;
    processCount   = 0;
    softBlockCount = 0;
    itSemaphore    = 0;
    for (int i = 0; i < IO_DEVICES_NUM; i++)
        deviceSemaphore[i] = 0;
    
    // riempimento del passup vector
    passupvector_t *puVector = (passupvector_t *)PASSUPVECTOR;
    puVector->tlb_refill_handler  = (memaddr)uTLB_RefillHandler;
    puVector->tlb_refill_stackPtr = (memaddr)KERNELSTACK;
    puVector->exception_handler   = (memaddr)exceptionHandler;
    puVector->exception_stackPtr  = (memaddr)KERNELSTACK;

    // setting dell'interval timer a 100 ms
    // LDIT richiede un valore in microsecondi, 100000 μs = 100 ms
    LDIT(PSECOND);

    // istanziazione processo iniziale
    pcb_t *firstProc = allocPcb();
    addNamespace(firstProc, NULL);
    firstProc->p_s.status = ALLOFF | IEPON | IMON | TEBITON;    /* kernel mode, ogni interrupt attivo, PLT attivo */
    firstProc->p_s.pc_epc = (memaddr)test;
    firstProc->p_s.reg_t9 = (memaddr)test;
    RAMTOP(firstProc->p_s.reg_sp);

    // caricamento processo iniziale
    insertProcQ(&readyQueue, firstProc);
    processCount++;
    scheduler();
}