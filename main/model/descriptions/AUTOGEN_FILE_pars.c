const char *pars_lingue[2][2] = {
    {"Italiano", "Italian", },
    {"Inglese", "English", },
};

const char *pars_loghi[6][2] = {
    {"Nessuno", "None", },
    {"MS", "MS", },
    {"Lavenda", "Lavenda", },
    {"Rotondi", "Rotondi", },
    {"Schulthess", "Schulthess", },
    {"Hoover", "Hoover", },
};

const char *pars_gettoniera[9][2] = {
    {"Nessuno", "None", },
    {"1 gettoniera NA", "1 Coint count. NO", },
    {"1 gettoniera NC", "1 Coin count. NC", },
    {"2 gettoniere NA", "2 Coin count. NO", },
    {"2 gettoniere NC", "2 Coin count. NC", },
    {"Gettoniera digitale", "Digital coin count.", },
    {"G. dig. a linea singola", "D. c.c. single line", },
    {"Cassa NA", "Cash Desk NA", },
    {"Cassa NC", "Cash Desk NC", },
};

const char *pars_accelerometro[4][2] = {
    {"Disabilitato", "Disabled", },
    {"1 velocita'", "1 speed", },
    {"2 velocita', soglie delta", "2 speed, delta thresh.", },
    {"3 velocita', soglie HI", "3 speed, HI thresh.", },
};

const char *pars_diretto_indiretto[2][2] = {
    {"Diretto", "Direct", },
    {"Indiretto", "Indirect", },
};

const char *pars_livello_accesso[4][2] = {
    {"Utente", "User", },
    {"Tecnico", "Technician", },
    {"Distributore", "Distributor", },
    {"Costruttore", "Constructor", },
};

const char *pars_ausiliari[8][2] = {
    {"Sblocco pagamento", "Payment unlock", },
    {"Standby saponi", "Detergent Standby", },
    {"Macchina occupata", "Machine busy", },
    {"Lampeggiante", "Blink", },
    {"H2O recupero", "H2O recycle", },
    {"Pompa ricircolo", "Recycle pump", },
    {"Scarico recupero", "Drain recovery", },
    {"Riscald. indir.", "Indirect heating", },
};

const char *pars_scala_accelerometro[6][2] = {
    {"0-2g con offset", "0-2g with offset", },
    {"1-4g con offset", "1-4g with offset", },
    {"2-8g con offset", "2-8g with offset", },
    {"3-2g no offset", "3-2g no offset", },
    {"4-4g no offset", "4-4g no offset", },
    {"5-8g no offset", "5-8g no offset", },
};

const char *pars_tipo_pagamento[5][2] = {
    {"Nessuno", "None", },
    {"Prezzo", "Price", },
    {"Prezzo e rimanente", "Price and remainder", },
    {"Prezzo e credito", "Price and credit", },
    {"Rimanente e credito", "Remainder and credit", },
};

const char *pars_tipo_livello[3][2] = {
    {"Centimetri", "Centimeters", },
    {"1 contalitri", "1 liter counter", },
    {"2 contalitri", "2 liter counter", },
};

const char *pars_na_nc[2][2] = {
    {"Normalmente aperto", "Normally open", },
    {"Normalmente chiuso", "Normally closed", },
};

const char *pars_rgb[8][2] = {
    {"Spento", "Off", },
    {"Blu", "Blue", },
    {"Verde", "Green", },
    {"Azzurro", "Cyan", },
    {"Rosso", "Red", },
    {"Viola", "Purple", },
    {"Giallo", "Yellow", },
    {"Bianco", "White", },
};

const char *pars_abilitazione[2][2] = {
    {"Disabilitato", "Disabled", },
    {"Abilitato", "Enabled", },
};

const char *pars_tipo_serratura[4][2] = {
    {"Nuova/3/livello", "New/1/level", },
    {"Bobina/1/livello", "Reel/1/level", },
    {"Nuova/3/no livello", "New/3/no level", },
    {"Bobina/1/no livello", "Reel/1/no level", },
};

const char *pars_tipo_inverter[2][2] = {
    {"Avanti/indietro", "Forward/backward", },
    {"Marcia/direzione", "Run/direction", },
};

const char *pars_nc_na[2][2] = {
    {"Normalmente chiuso", "Normally closed", },
    {"Normalmente aperto", "Normally open", },
};

const char *pars_tempo_attivo[5][2] = {
    {"Subito", "Immediate", },
    {"Dopo il livello", "After the level", },
    {"Dopo la temperatura", "After the temperature", },
    {"Dopo liv. e temp.", "After lev. and temp.", },
    {"Con vel. di riempimento", "With fill speed", },
};

const char *pars_richiesta_pagamento[4][2] = {
    {"Inserire gettone", "Insert token", },
    {"Inserire moneta", "Insert coin", },
    {"Pagamento cassa", "Pay at the desk", },
    {"Pagamento importo", "Pay the required amount", },
};

const char *pars_descriptions[167][2] = {
    {"PARAMETRO", "Parameter", },
    {"LINGUA", "LANGUAGE", },
    {"LOGO", "LOGO", },
    {"LIVELLO DI ACCESSO", "ACCESS LEVEL", },
    {"TEMPO TASTO PAUSA", "PAUSE BUTTON TIME", },
    {"TEMPO TASTO STOP", "STOP BUTTON TIME", },
    {"Durata", "Duration", },
    {"Tempo attivo", "Active time", },
    {"Moto in riempimento", "Fillup motion", },
    {"Velocita' riempimento", "Fillup speed", },
    {"Inversione riempimento", "Fillup reversal", },
    {"T. moto riempimento", "Fillup motion time", },
    {"T. pausa riempimento", "Fillup pause time", },
    {"Velocita' lavaggio", "Wash speed", },
    {"Inversione lavaggio", "Wash reversal", },
    {"T. moto lavaggio", "Wash motion time", },
    {"T. pausa lavaggio", "Wash pause time", },
    {"Riscaldamento", "Heating", },
    {"Temperatura", "Temperature", },
    {"Tipo riscaldamento", "Heating type", },
    {"Tipo gettoniera", "Payment type", },
    {"Valore impulso", "Pulse value", },
    {"Valore prezzo unico", "Absolute price value", },
    {"Prezzo unico", "Absolute price", },
    {"Cifre prezzo", "Price digits", },
    {"Cifre decimali prezzo", "Decimal price digits", },
    {"Visualizzazione prezzo", "Price display", },
    {"Richiesta pagamento", "Payment request", },
    {"Sblocco gettoniera", "Coin acc. unlock", },
    {"Inibizione allarmi", "Disable alarms", },
    {"Diametro cesto", "Basket diameter", },
    {"Profondita' cesto", "Basked depth", },
    {"Altezza trappola", "Trap height", },
    {"Sensore prossimita'", "Proximity sensor", },
    {"Numero raggi", "Radiuses", },
    {"Correzione contagiri", "Rounds corrector", },
    {"Scala accelerometro", "Accelerometer scale", },
    {"Soglia x acc.", "Acc. x limit", },
    {"Soglia y acc.", "Acc. y limit", },
    {"Soglia z acc.", "Acc. z limit", },
    {"Soglia x acc. (h)", "Acc. x limit (h)", },
    {"Soglia y acc. (h)", "Acc. y limit (h)", },
    {"Soglia z acc. (h)", "Acc. z limit (h)", },
    {"Giri accelerometro", "Accelerometer rounds", },
    {"Giri accelerometro 2", "Accelerometer rounds 2", },
    {"Delta accelerometro", "Delta accelerometer", },
    {"Tempo attesa acc.", "Acc. wait time", },
    {"Temperatura massima", "Maximum temperature", },
    {"Isteresi temperatura", "Temperature hysteresis", },
    {"Temperatura sicurezza", "Safety temperature", },
    {"Temperatura termodegradazione", "Thermal degradation temperature", },
    {"Tipo livello", "Level type", },
    {"Tempo isteresi livello", "Level hysteresis time", },
    {"Livello massimo", "Max level", },
    {"Livello sfioro", "Touch level", },
    {"Livello minimo scarico", "Minimum drain level", },
    {"Livello minimo riscaldamento", "Minimum heating level", },
    {"Litri massimi", "Maximum liters", },
    {"Litri minimi riscaldamento", "Minimum heating liters", },
    {"Impulsi litri", "Pulses per liters", },
    {"Tipo inverter", "Inverter type", },
    {"Velocita' servizio", "Service speed", },
    {"Velocita' minima lavaggio", "Minimum wash speed", },
    {"Velocita' massima lavaggio", "Maximum wash speed", },
    {"Cicli preparazione", "Preparation cycles", },
    {"T. marcia prep. rot.", "Run time rot. prep.", },
    {"T. sosta prep. rot.", "Pause time rot. prep.", },
    {"V. minima prep.", "Minimum prep. speed", },
    {"V. massima prep.", "Max prep. speed", },
    {"V. min. centrifuga 1", "Minimum cen. 1 speed", },
    {"V. max centrifuga 1", "Maximum cen. 1 speed", },
    {"V. min. centrifuga 2", "Minimum cen. 2 speed", },
    {"V. max centrifuga 2", "Maximum cen. 2 speed", },
    {"V. min. centrifuga 3", "Minimum cen. 3 speed", },
    {"V. max centrifuga 3", "Maximum cen. 3 speed", },
    {"T. minimo rampa", "Minium ramp time", },
    {"T. massimo rampa", "Maximum ramp time", },
    {"N. max sbilanciamenti", "N. max sbilanciamenti", },
    {"Min/sec", "Min/sec", },
    {"Tipo serratura", "Lock type", },
    {"Durata impulso serratura", "Lock pulse duration", },
    {"Controllo t. continuo", "Continous t. control", },
    {"Livello", "Level", },
    {"Ricircolo", "Recycling", },
    {"Acqua fredda", "Cold water", },
    {"Acqua calda", "Hot water", },
    {"Acqua depurata", "Purified water", },
    {"Sapone", "Detergent", },
    {"Tempo sapone 1", "Detergent time 1", },
    {"Ritardo sapone 1", "Detergent delay 1", },
    {"Tempo sapone 2", "Detergent time 2", },
    {"Ritardo sapone 2", "Detergent delay 2", },
    {"Tempo sapone 3", "Detergent time 3", },
    {"Ritardo sapone 3", "Detergent delay 3", },
    {"Tempo sapone 4", "Detergent time 4", },
    {"Ritardo sapone 4", "Detergent delay 4", },
    {"Tempo sapone 5", "Detergent time 5", },
    {"Ritardo sapone 5", "Detergent delay 5", },
    {"Tempo sapone 6", "Detergent time 6", },
    {"Ritardo sapone 6", "Detergent delay 6", },
    {"Tempo sapone 7", "Detergent time 7", },
    {"Ritardo sapone 7", "Detergent delay 7", },
    {"Tempo sapone 8", "Detergent time 8", },
    {"Ritardo sapone 8", "Detergent delay 8", },
    {"Tempo sapone 9", "Detergent time 9", },
    {"Ritardo sapone 9", "Detergent delay 9", },
    {"Tempo sapone 10", "Detergent time 10", },
    {"Ritardo sapone 10", "Detergent delay 10", },
    {"Movimento", "Motion", },
    {"Recupero", "Recovery", },
    {"Tempo preparazione", "Preparation time", },
    {"Preparation speed", "Preparation speed", },
    {"Tipo scarico", "Drain type", },
    {"Velocita' cen. 1", "Cen. speed 1", },
    {"Tempo rampa 1", "Ramp time 1", },
    {"Velocita' cen. 2", "Cen. speed 2", },
    {"Tempo rampa 2", "Ramp time 2", },
    {"Velocita' cen. 3", "Cen. speed 3", },
    {"Tempo rampa 3", "Ramp time 3", },
    {"Tempo frenata", "Stop time", },
    {"Tempo attesa", "Wait time", },
    {"Tempo avviso attesa on", "Tempo avviso attesa on", },
    {"Tempo avviso attesa off", "Tempo avviso attesa off", },
    {"Numero rampe", "Ramp number", },
    {"Tempo attivo sapone", "Detergent active time", },
    {"T. attesa cen. 1", "Cen. 1 wait time", },
    {"T. attesa cen. 2", "Cen. 2 wait time", },
    {"Numero saponi", "Detergent count", },
    {"Ritardo allarme livello", "Level alarm delay", },
    {"Ritardo allarme scarico", "Drain alarm delay", },
    {"Ritardo micro oblo", "Door contact delay", },
    {"Tempo precarica", "Preload time", },
    {"Tempo pulizia saponi", "Soap cleaning time", },
    {"Tempo carico saponi", "Soap load time", },
    {"Tempo colpo scarico", "Drain opening time", },
    {"Tempo minimo scarico", "Minimum drain time", },
    {"Tempo minimo frenata", "Minimum stop time", },
    {"Scarico recupero", "Drain recovery", },
    {"Livello carico ridotto", "Reduced load level", },
    {"Sapone carico ridotto", "Reduced load detergent", },
    {"Autoavvio", "Autostart", },
    {"Accelerometro", "Accelerometer", },
    {"Tempo scarico accelerometro", "Accelerometer drain time", },
    {"Tempo allarme temperatura", "Temperature alarm time", },
    {"Espansione IO", "IO expansion", },
    {"Esclusione sapone", "Detergent exclusion", },
    {"Macchina libera", "Machine free", },
    {"Tipo macchina libera", "Machine free signal", },
    {"Tipo IN ausiliario 1", "Auxiliary IN type 1", },
    {"Tipo OUT ausiliario 2", "Auxiliary OUT type 2", },
    {"Tipo OUT ausiliario 3", "Auxiliary OUT type 3", },
    {"Tipo OUT ausiliario 4", "Auxiliary OUT type 4", },
    {"Lavaggio programmato", "Scheduled cycle", },
    {"LED da fermo", "LED while stopped", },
    {"LED al lavoro", "LED while working", },
    {"LED in pausa", "LED in pause", },
    {"LED in attesa", "LED waiting", },
    {"LED con avviso", "LED with notification", },
    {"LED con allarme", "LED with alarmti", },
    {"Interf. al lavoro", "Work interf.", },
    {"Interf. da fermo", "Stopped interf.", },
    {"Interf. menu'", "Menu interface", },
    {"Velocita'", "Speed", },
    {"Max programmi utente", "Max user programs", },
    {"Ripetizione ciclo", "Cycle repetition", },
    {"Interf. saponi", "Detergent interf.", },
    {"Tempo scarico servizio", "Service drain time", },
};

const char *pars_macchina_libera[3][2] = {
    {"Non gestita", "Not managed", },
    {"Commuta allo start", "Changes on start", },
    {"Commuta al pagamento", "Changes on payment", },
};

const char *pars_visualizzazione[2][2] = {
    {"Self", "Self", },
    {"Laboratorio", "Laboratory", },
};

