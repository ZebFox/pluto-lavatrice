 # Pluto 

Firmware per la scheda Pluto per il controllo della scheda lavatrice quadro.

## Compilazione

Il firmware dovrebbe essere compilato con i seguenti strumenti:
 - ESP-IDF v4.4.x (testato con v4.4.6)

Per generare delle nuove traduzioni modificare i file csv sotto `assets/translations` e invocare il comando `scons intl`.

## Struttura del Progetto

Il progetto e' strutturato secondo il paradigma Model-View-Controller (o almeno la mia interpretazione).

Il Model contiene tutti i dati dell'applicazione in un'unica `struct`; non ha dipendenze e gestisce soltanto i dati.

La View gestisce tutto cio' che e' correlato al display: e' divisa in pagine che si impilano durante l'esecuzione.
Ogni pagina e' costituita da callback che ne descrivono il ciclo di vita (creazione, apertura, gestione degli eventi, chiusura e distruzione) durante il quale possono mostrare informazioni sullo schermo, modificare il modello o inviare messaggi al Controller in risposta a interazioni dell'utente.

Il Controller gestisce tutto il resto; la colla tra i componenti e l'interazione con l'hardware.

# TODO
    
    - Funzionalita' lavaggio programmato
    - Pagina eventi
    - Durante la frenata far comparire un popup che indichi quanto tempo manca o a che velocita' si e' arrivati
