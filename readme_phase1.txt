Per compilare il test della fase 1:
    $ make
I file compilati vengono inseriti nella directory bin/ (se non esiste viene creata).

Per rimuovere bin/ e tutti i file .o:
    $ make clean

=======================

Nel nostro sviluppo della phase 1 di Panda+ non abbiamo compiuto scelte implementative
a nostro avviso particolarmente eterodosse, riteniamo che per comprendere il funzionamento
del codice siano sufficienti i commenti all'interno dello stesso.

Le uniche particolarità degne di essere segnalate:
    * ash.c implementa tre ulteriori funzioni statiche non presenti nelle specifiche
      del progetto, per manipolare in maniera più ordinata i semafori: __allocSemd,
      __scanHash e __ifEmptyDeallocSemd. Le funzioni sono descritte nei commenti al
      codice. Queste funzioni sono utilizzate solamente all'interno di ash.c.
    * In nsd.c, nsFree_h e nsList_h sono array di liste di lunghezza NS_TYPE_MAX, ossia
      esiste una lista per ogni tipo di NSD. Le specifiche della phase 1 richiedono
      solamente l'implementazione del tipo NS_PID, il ché rende l'utilizzo dell'array
      inutile rispetto alla semplice dichiarazione di nsFree_h e nsList_h come list_head,
      ma abbiamo comunque optato per questo approccio per far sì che il nostro codice sia
      eventualmente utilizzabile con specifiche che richiedano più tipi di NSD.

Il nostro makefile è adattato da quello disponibile in un esempio su VirtualSquare:
    https://wiki.virtualsquare.org/education/tutorials/umps/examples/hello-world.tar.gz