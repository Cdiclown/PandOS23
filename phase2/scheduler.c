#include "scheduler.h"

/**
 * Funzione per effettuare il dispatching dei processi.
 */
void scheduler() {
    if (processCount <= 0) /* zero processi in esecuzione */
        HALT();
    
    currentProc = removeProcQ(&readyQueue);

    if (currentProc == NULL) {                  /* nessun processo ready     */
        if (softBlockCount > 0) {               /* processi in attesa di I/O */
            setSTATUS(ALLOFF | IECON | IMON);
            WAIT();
        }
        else                                    /* situazione di deadlock    */
            PANIC();
    }
    else {                                             /* esiste un processo ready */
        currentProc->p_s.status |= TEBITON;
        setTIMER((cpu_t)TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));  /* timer a 5 ms */
        getTimespan();                                 /* inizia un nuovo timespan */
        LDST(&currentProc->p_s);
    }
}