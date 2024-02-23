#include "nsd.h"

/* NS_PID = 0, NS_TYPE_MAX = 1, MAXPROC = 20 */

static nsd_t nsd_table[NS_TYPE_MAX][MAXPROC];   // array bidimensionale contenente MAXPROC namespace per ogni tipo di namespace
static struct list_head nsFree_h[NS_TYPE_MAX];  // array di liste di namespace liberi
static struct list_head nsList_h[NS_TYPE_MAX];  // array di liste di namespace attivi


void initNamespaces() {
    for (int i = 0; i < NS_TYPE_MAX; i++) {     // inizializzazione delle liste di namespace (nel nostro caso esistono solo le liste nsFree_h[NS_PID] e nsList_h[NS_PID])
        INIT_LIST_HEAD(&nsFree_h[i]);
        INIT_LIST_HEAD(&nsList_h[i]);

        for (int j = 0; j < MAXPROC; j++) {     // inizializzazione di tutti i namespace contenuti in nsd_table[i]
            nsd_table[i][j].n_type = i;
            list_add_tail(&nsd_table[i][j].n_link, &nsFree_h[i]);
        }
    }
}


nsd_t *getNamespace(pcb_t *p, int type) {
    return p->namespaces[type];
}


int addNamespace(pcb_t *p, nsd_t *ns) {
    if (p == NULL)
        return true;
    
    if (ns == NULL) {
        ns = allocNamespace(NS_PID);
        if (ns == NULL)
            return false;
    }

    p->namespaces[ns->n_type] = ns;

    pcb_t *child;
    list_for_each_entry(child, &p->p_child, p_sib)  // assegna ricorsivamente lo stesso namespace a tutti i figli di *p
        addNamespace(child, ns);
    return true;
}


nsd_t *allocNamespace(int type) {
    if (list_empty(&nsFree_h[type]))                // caso in cui i namespace liberi del tipo richiesto sono finiti
        return NULL;

    nsd_t *ns = list_first_entry(&nsFree_h[type], nsd_t, n_link);
    list_move_tail(&ns->n_link, &nsList_h[type]);   // sposta *ns da nsFree_h[type] a nsList_h[type]
    return ns;
}


void freeNamespace(nsd_t *ns) {
    list_move_tail(&ns->n_link, &nsFree_h[ns->n_type]);     // sposta *ns in nsFree_h[ns->n_type]
}