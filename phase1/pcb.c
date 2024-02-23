#include "pcb.h"

static LIST_HEAD(pcbFree_h);
static pcb_t pcbFree_table[MAXPROC];


void initPcbs() {
    INIT_LIST_HEAD(&pcbFree_h);
    for (int i = 0; i < MAXPROC; i++) {
        pcbFree_table[i].p_pid = 0;
        list_add_tail(&pcbFree_table[i].p_list, &pcbFree_h);
    }
}


void freePcb(pcb_t *p) {
    p->p_pid = 0;
    list_add_tail(&p->p_list, &pcbFree_h);
}


pcb_t *allocPcb() {
    if (list_empty(&pcbFree_h)) // caso in cui non ci siano PCB liberi
        return NULL;

    pcb_t *p = list_first_entry(&pcbFree_h, pcb_t, p_list);

    // inizializzazione dei puntatori e liste di PCB
    list_del(&p->p_list);
    p->p_parent = NULL;
    INIT_LIST_HEAD(&p->p_child);
    INIT_LIST_HEAD(&p->p_sib);

    // inizializzazione dei campi di p_s e p_time
    p->p_s.entry_hi = 0;
    p->p_s.cause = 0;
    p->p_s.status = 0;
    p->p_s.pc_epc = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        p->p_s.gpr[i] = 0;
    p->p_s.hi = 0;
    p->p_s.lo = 0;
    p->p_time = 0;

    // inizializzazione dei puntatori a semaforo, namespace, supporto e pid
    p->p_semAdd = NULL;
    p->p_supportStruct = NULL;
    p->p_pid = (int)(p - &pcbFree_table[0]) + 1;
    for (int i = 0; i < NS_TYPE_MAX; i++)
        p->namespaces[i] = NULL;
    
    return p;
}


pcb_t *getPcb(int pid) {
    if (pid < 1 || pid > MAXPROC)
        return NULL;
    
    pcb_t *p = &pcbFree_table[pid-1];
    return p->p_pid == pid ? p : NULL;
}


int isPcbActive(pcb_t *p) {
    if (p == NULL)
        return 0;

    return p->p_pid != 0;
}


void mkEmptyProcQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}


int emptyProcQ(struct list_head *head) {
    return list_empty(head);
}


void insertProcQ(struct list_head *head, pcb_t *p) {
    list_add_tail(&p->p_list, head);
}


pcb_t *headProcQ(struct list_head *head) {
    if (list_empty(head))
        return NULL;

    return list_first_entry(head, pcb_t, p_list);
}


pcb_t *removeProcQ(struct list_head *head) {
    pcb_t *p = headProcQ(head);
    if (p != NULL)
        list_del(&p->p_list);

    return p;
}


pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
    pcb_t *iter = NULL;

    // ciclo per trovare p nella lista head
    list_for_each_entry(iter, head, p_list) {
        if (iter == p) {
            list_del(&p->p_list);
            return p;
        }
    }

    return NULL;
}


int emptyChild(pcb_t *p) {
    return list_empty(&p->p_child);
}


void insertChild(pcb_t *prnt, pcb_t *p) {
    list_add_tail(&p->p_sib, &prnt->p_child);
    p->p_parent = prnt;
}


pcb_t *removeChild(pcb_t *p) {
    if (list_empty(&p->p_child))
        return NULL;
    
    /* preleva il primo PCB da p->p_child,
       inizializza p_parent e lo ritorna */
    pcb_t *child = list_entry(list_next(&p->p_child), pcb_t, p_sib);
    list_del(&child->p_sib);
    child->p_parent = NULL;
    return child;
}


pcb_t *outChild(pcb_t *p) {
    if (p->p_parent == NULL)
        return NULL;

    pcb_t *iter = NULL;
    
    // ciclo per trovare p tra i figli di p->p_parent
    list_for_each_entry(iter, &p->p_parent->p_child, p_sib) {
        if (iter == p) {
            p->p_parent = NULL;
            list_del(&p->p_sib);
            return p;
        }
    }

    return NULL;
}