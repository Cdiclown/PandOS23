#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include <umps3/umps/libumps.h>
#include "../phase1/pcb.h"
#include "../phase1/ash.h"
#include "../phase1/nsd.h"
#include "exceptions.h"
#include "interrupts.h"
#include "scheduler.h"


#define SAVED_STATE    ((state_t *)BIOSDATAPAGE)  /* stato del processore al momento di un'eccezione          */
#define IO_DEVICES_NUM (DEVICECNT + DEVPERINT)    /* numero totale di device + altre 8 linee per il terminale */

extern pcb_t *currentProc;
extern int processCount;
extern int softBlockCount;
extern struct list_head readyQueue;
extern int itSemaphore;
extern int deviceSemaphore[IO_DEVICES_NUM];

void memcpy(void *dest, const void *src, unsigned int len);
cpu_t getTimespan();
void updateCurrentProcTime();
bool isSoftBlocked(pcb_t *p);
void P(int *semaddr);
void V(int *semaddr);
int *getDeviceSem(int cmdAddr);

#endif