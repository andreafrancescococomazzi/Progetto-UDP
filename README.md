# Progetto-UDP
Il progetto consiste nella realizzazione di un'applicazione UDP client-server, in cui il server funge da generatore di password in base alle richieste ricevute dal client. Una volta ricevuto un datagramma, il server registra l'indirizzo e la porta del mittente, mostrando un messaggio di log sullo standard output.

Il client, tramite input da tastiera, specifica il tipo di password desiderata e la lunghezza, utilizzando un protocollo semplice definito attraverso caratteri identificativi:
n: password numerica (solo cifre)
a: password alfabetica (solo lettere minuscole)
m: password mista (lettere minuscole e numeri)
s: password sicura (lettere maiuscole, minuscole, numeri e simboli)
u: password sicura senza caratteri ambigui
h: visualizza il menu di aiuto
Ad esempio, inserendo n 8 viene richiesta al server una password numerica di 8 caratteri. Il server elabora la richiesta, genera la password utilizzando le funzioni dedicate e la restituisce al client tramite UDP.
Il sistema rimane attivo fino all'inserimento del comando q, che provoca la chiusura del client. Il server resta sempre in esecuzione e può gestire più richieste in sequenza.

Questo progetto ha permesso di approfondire i concetti di programmazione di rete in C, gestione delle socket UDP, protocolli applicativi e sicurezza nella generazione delle password, con attenzione all'affidabilità e alla portabilità del codice tra diversi sistemi operativi.
