#include "exceptions.h"

#define EXC_CODE             CAUSE_GET_EXCCODE(SAVED_STATE->cause)    /* codice dell'eccezione                */
#define SYSCALL_CODE         ((int)SAVED_STATE->reg_a0)               /* codice della syscall                 */
#define GET_ARG1(type, name) type name = (type)SAVED_STATE->reg_a1    /* registri con parametri della syscall */
#define GET_ARG2(type, name) type name = (type)SAVED_STATE->reg_a2    // (crea una variabile di tipo type chiamata name e gli assegna il valore dell'argomento)
#define GET_ARG3(type, name) type name = (type)SAVED_STATE->reg_a3
#define SYSCALL_RETURN(val)  SAVED_STATE->reg_v0 = val                /* registro di ritorno della syscall    */


/**
 * Uccide *p e tutta la sua progenie.
 * Rimuove i processi uccisi da tutte le code in cui si trovano,
 * aggiorna i contatori e dealloca il PCB.
 */
static void __killProcess(pcb_t *p) {
    processCount--;
    outChild(p);

    if (p->p_semAdd != NULL) {
        if (isSoftBlocked(p))
            softBlockCount--;
        outBlocked(p);
    }

    outProcQ(&readyQueue, p);

    // uccisione ricorsiva dei figli
    pcb_t *iter;
    while ((iter = removeChild(p)) != NULL)
        __killProcess(iter);
    
    freePcb(p);
}


/**
 * Handler per il passup or die.
 */
static void __passUpOrDieHandler(int excType) {
    if (currentProc->p_supportStruct == NULL) {    /* die    */
        __killProcess(currentProc);
        scheduler();
    }
    else {                                         /* passup */
        currentProc->p_supportStruct->sup_exceptState[excType] = *SAVED_STATE;
        context_t newCxt = currentProc->p_supportStruct->sup_exceptContext[excType];
        LDCXT(newCxt.stackPtr, newCxt.status, newCxt.pc);
    }
}


/**
 * Syscall 1
 */
static void __createProcess() {
    GET_ARG1(state_t *, statep);
    GET_ARG2(support_t *, supportp);
    GET_ARG3(nsd_t *, ns);

    pcb_t *newProc = allocPcb();
    if (newProc == NULL) {       /* nessun PCB libero */
        SYSCALL_RETURN(-1);
        return;
    }

    processCount++;

    newProc->p_s = *statep;
    newProc->p_supportStruct = supportp;

    if (ns == NULL)
        addNamespace(newProc, getNamespace(currentProc, NS_PID));   /* se NULL, eredita il ns del padre */
    else
        addNamespace(newProc, ns);

    insertChild(currentProc, newProc);
    insertProcQ(&readyQueue, newProc);

    SYSCALL_RETURN(newProc->p_pid);
}


/**
 * Syscall 2
 */
static void __termProcess() {
    GET_ARG1(int, pid);

    if (pid == 0) {
        __killProcess(currentProc);
        scheduler();
    }
    else {
        pcb_t *victim = getPcb(pid);
        if (victim != NULL)
            __killProcess(victim);
        if (!isPcbActive(currentProc))
            scheduler();
    }
}


/**
 * Syscall 3
 */
static void __passeren() {
    GET_ARG1(int *, semaddr);

    P(semaddr);
}


/**
 * Syscall 4
 */
static void __verhogen() {
    GET_ARG1(int *, semaddr);

    V(semaddr);
}


/**
 * Syscall 5
 */
static void __doIo() {
    GET_ARG1(int *, cmdAddr);
    GET_ARG2(int *, cmdValues);

    cmdAddr[COMMAND] = cmdValues[COMMAND];          /* copia cmdValues in cmdAddr */

    if ((int)cmdAddr < DEV_REG_ADDR(TERMINT, 0)) {  /* per i device non terminali */
        cmdAddr[DATA0] = cmdValues[DATA0];
        cmdAddr[DATA1] = cmdValues[DATA1];
    }

    int *semaddr = getDeviceSem((int)cmdAddr);

    softBlockCount++;
    P(semaddr);

    /* Il valore di ritorno in reg_v0 verrà assegnato dal device interrupt.
       Tale interrupt copierà anche il nuovo stato di cmdAddr in cmdValues. */
}


/**
 * Syscall 6
 */
static void __getTime() {
    updateCurrentProcTime();
    SYSCALL_RETURN(currentProc->p_time);
}


/**
 * Syscall 7
 */
static void __clockWait() {
    softBlockCount++;
    P(&itSemaphore);
}


/**
 * Syscall 8
 */
static void __getSupportPtr() {
    SYSCALL_RETURN((memaddr)currentProc->p_supportStruct);
}


/**
 * Syscall 9
 */
static void __getProcessId() {
    GET_ARG1(int, parent);

    if (parent == 0)                          /* PID del chiamante           */
        SYSCALL_RETURN(currentProc->p_pid);
    else if (currentProc->p_parent == NULL || /* caso di errore              */
             getNamespace(currentProc, NS_PID) != getNamespace(currentProc->p_parent, NS_PID))
        SYSCALL_RETURN(0);
    else                                      /* PID del padre del chiamante */
        SYSCALL_RETURN(currentProc->p_parent->p_pid);
}


/**
 * Syscall 10
 */
static void __getChildren() {
    GET_ARG1(int *, children);
    GET_ARG2(int, size);

    if (emptyChild(currentProc)) {
        SYSCALL_RETURN(0);
        return;
    }

    int childNum = 0;
    pcb_t *iter = NULL;

    list_for_each_entry(iter, &currentProc->p_child, p_sib) {
        if (getNamespace(iter, NS_PID) == getNamespace(currentProc, NS_PID)) {
            if (childNum < size)
                children[childNum] = iter->p_pid;
            childNum++;
        }
    }

    SYSCALL_RETURN(childNum);
}


/**
 * Seleziona l'handler corretto per ogni tipo di syscall.
 * Se il chiamante è in user mode o richiede una syscall user, invoca l'handler del passup or die.
 */
static void __syscallHandler() {
    SAVED_STATE->pc_epc += WORDLEN;                         /* incrementa il PC per saltare l'istruzione di syscall   */

    if (SYSCALL_CODE > 10 || SYSCALL_CODE < 1)              /* syscall non-kernel                                     */
        __passUpOrDieHandler(GENERALEXCEPT);
    else if ((SAVED_STATE->status & USERPON) != 0) {        /* caso in cui il processo è in user mode (viene ucciso)  */
        SAVED_STATE->cause &= ~CAUSE_EXCCODE_MASK;          /* rimuove la precedente causa dell'eccezione             */
        SAVED_STATE->cause |= EXC_RI << CAUSE_EXCCODE_BIT;  /* assegna la causa "reserved instruction" (eccezione 10) */
        __passUpOrDieHandler(GENERALEXCEPT);
    }
    else {
        switch (SYSCALL_CODE) {
            case CREATEPROCESS: __createProcess(); break;
            case TERMPROCESS:   __termProcess();   break;
            case PASSEREN:      __passeren();      break;
            case VERHOGEN:      __verhogen();      break;
            case DOIO:          __doIo();          break;
            case GETTIME:       __getTime();       break;
            case CLOCKWAIT:     __clockWait();     break;
            case GETSUPPORTPTR: __getSupportPtr(); break;
            case GETPROCESSID:  __getProcessId();  break;
            case GETCHILDREN:   __getChildren();   break;
        }

        LDST(SAVED_STATE);
    }
}


/**
 * Seleziona l'handler corretto per ogni tipo di eccezione.
 */
void exceptionHandler() {
    updateCurrentProcTime();

    switch (EXC_CODE) {
        case EXC_INT:              interruptHandler();                  break;    /* interrupt         */
        case EXC_MOD ... EXC_TLBS: __passUpOrDieHandler(PGFAULTEXCEPT); break;    /* TLB exception     */
        case EXC_SYS:              __syscallHandler();                  break;    /* syscall           */
        default:                   __passUpOrDieHandler(GENERALEXCEPT); break;    /* general exception */
    }
}