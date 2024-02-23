#ifndef NSD_H_INCLUDED
#define NSD_H_INCLUDED

#include "../lib/pandos_types.h"
#include "pcb.h"


/**
 * Inserisce i namespace contenuti in nsd_table[][]
 * dentro la lista in nsFree_h[] del tipo corretto.
 * Questo metodo viene invocato una volta sola durante
 * l'inizializzazione della struttura dati.
 */
void initNamespaces();


/**
 * Ritorna il namespace di tipo type associato
 * al processo *p (oppure NULL).
 */
nsd_t *getNamespace(pcb_t *p, int type);


/**
 * Associa al processo *p e a tutti i suoi figli
 * il namespace *ns.
 * Ritorna FALSE in caso di errore, TRUE altrimenti.
 */
int addNamespace(pcb_t *p, nsd_t *ns);


/**
 * Alloca un namespace di tipo type
 * dalla relativa lista in nsFree_h.
 */
nsd_t *allocNamespace(int type);


/**
 * Rimuove il namespace *ns dalla sua nsList_h
 * e lo inserisce nella lista nsFree_h corretta.
 */
void freeNamespace(nsd_t *ns);


#endif