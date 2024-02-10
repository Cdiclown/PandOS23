Per compilare il test della fase 2:
    $ make
I file compilati vengono inseriti nella directory bin/ (se non esiste viene creata).

Per rimuovere bin/ e tutti i file .o:
    $ make clean

===========================

Organizzazione dei file:
    initial
        Contiene il main, più funzioni ausiliarie per
        facilitare la gestione dei PCB e variabili
        globali.
    scheduler
        Contiene la funzione per lo scheduling, invocata
        ogni volta che si renda necessario cambiare il PCB
        in esecuzione.
    exceptions
        Contiene l'exception handler e funzioni per
        gestire le syscall.
    interrupts
        Contiene l'interrupt handler e funzioni per
        gestire gli interrupt.

Sono disponibili descrizioni più approfondite delle funzioni nei commenti al codice.

Gestione dei PID
    Ad ogni PCB viene univocamente assegnato un PID, che viene
    mantenuto invariato durante ogni allocazione del PCB.
    Tale PID corrisponde alla posizione del PCB dentro
    l'array pcbFree_table incrementata di 1, e.g. il
    PCB pcbFree_table[4] ha PID 5. Quando viene deallocato
    il PID diventa 0.

Il nostro makefile è adattato da quello disponibile in un esempio su VirtualSquare:
    https://wiki.virtualsquare.org/education/tutorials/umps/examples/hello-world.tar.gz