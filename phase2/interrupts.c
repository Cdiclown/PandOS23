#include "interrupts.h"

/**
 * Funzione per uscire dall'interrupt handler.
 */
static void __exitIntHandler() {
    getTimespan();                /* scarta il tempo di elaborazione dell'interrupt */

    if (currentProc != NULL)
        LDST(SAVED_STATE);
    else
        scheduler();
}


/**
 * Sblocca un PCB bloccato sulla coda del semaforo a cui si riferisce
 * il device indicato da devRegister. Scrive il contenuto di *devRegister
 * e dei registri successivi nell'area di memoria puntata dal parametro
 * cmdValues della syscall 5 effettuava dal relativo processo.
*/
static void __devReturn(unsigned int *devRegister, int isTerminal) {
    pcb_t *readyProc = removeBlocked(getDeviceSem((int)devRegister));

    if (isPcbActive(readyProc)) {
        softBlockCount--;

        if (devRegister[STATUS] == 0 || devRegister[STATUS] == 2)    /* valore di ritorno della syscall 5 */
            readyProc->p_s.reg_v0 = -1;
        else
            readyProc->p_s.reg_v0 = 0;

        int *cmdValues = (int *)readyProc->p_s.reg_a2;    /* riutilizza l'indirizzo fornito come parametro per la sys5 */
        cmdValues[STATUS]  = devRegister[STATUS];
        cmdValues[COMMAND] = devRegister[COMMAND];

        if (!isTerminal) {
            cmdValues[DATA0] = devRegister[DATA0];
            cmdValues[DATA1] = devRegister[DATA1];
        }

        insertProcQ(&readyQueue, readyProc);
    }

    devRegister[COMMAND] = ACK;    /* scrive l'ack sul device register */
}


/**
 * Handler per i PLT interrupt.
 * Assegna un valore grande al timer come acknowledgment
 * (verrà poi impostato correttamente dallo scheduler).
 */
static void __pltHandler() {
    setTIMER(2147483647);    /* ack */

    currentProc->p_s = *SAVED_STATE;

    if (isPcbActive(currentProc))
        insertProcQ(&readyQueue, currentProc);

    scheduler();
}


/**
 * Handler per gli interval timer interrupt.
 */
static void __itHandler() {
    pcb_t *iter;
    while ((iter = removeBlocked(&itSemaphore)) != NULL) {     /* sblocca tutti i processi bloccati su itSemaphore */
        if (isPcbActive(iter)) {
            insertProcQ(&readyQueue, iter);
            softBlockCount--;
        }
    }

    LDIT(PSECOND);      /* riattiva l'interval timer */
    __exitIntHandler();
}


/**
 * Handler per gli interrupt dei device I/O.
 * Gestisce un singolo device per ogni invocazione.
 */
static void __devHandler(int line) {
    unsigned int bitmap = *((unsigned int *)CDEV_BITMAP_ADDR(line));  /* indica i device con interrupt pending */
    unsigned int mask = 1;                                            /* utilizzata per iterare sui device     */

    for (int devNum = 0; devNum < N_INTERRUPT_LINES; devNum++) {
        if ((bitmap & mask) != 0) {
            if (line != IL_TERMINAL) {                                                        /* non terminale */
                dtpreg_t *devRegister = (dtpreg_t *)DEV_REG_ADDR(line, devNum);
                __devReturn(&devRegister->status, 0);
            }
            else {                                                                                /* terminale */
                termreg_t *devRegister = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, devNum);
                if (devRegister->transm_status > READY && devRegister->transm_status != BUSY)      /* transmit */
                    __devReturn(&devRegister->transm_status, 1);
                else if (devRegister->recv_status > READY && devRegister->recv_status != BUSY)      /* receive */
                    __devReturn(&devRegister->recv_status, 1);
            }
            __exitIntHandler();
        }

        mask <<= 1;
    }
}


/**
 * Seleziona l'handler corretto per ogni tipo di interrupt.
 */
void interruptHandler() {
    unsigned int ip = SAVED_STATE->cause & CAUSE_IP_MASK;   /* causa dell'interrupt */

    if      ((ip & LOCALTIMERINT)  != 0)
        __pltHandler();
    else if ((ip & TIMERINTERRUPT) != 0)
        __itHandler();
    else if ((ip & DISKINTERRUPT)  != 0)
        __devHandler(IL_DISK);
    else if ((ip & FLASHINTERRUPT) != 0)
        __devHandler(IL_FLASH);
    else if ((ip & NETINTERRUPT)   != 0)
        __devHandler(IL_ETHERNET);
    else if ((ip & PRINTINTERRUPT) != 0)
        __devHandler(IL_PRINTER);
    else if ((ip & TERMINTERRUPT)  != 0)
        __devHandler(IL_TERMINAL);
    else
        __exitIntHandler();
}