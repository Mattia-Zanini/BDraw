# Introduzione

Il celebre problema della brachistocrona, ovvero la ricerca della curva che minimizza il tempo di percorrenza tra due punti sotto l'effetto della gravità, rappresenta una delle pietre miliari della storia della matematica. La sua soluzione, un arco di cicloide, fu scoperta oltre tre secoli fa, ma il fascino di questa sfida risiede ancora oggi non solo nel risultato, quanto nelle geniali intuizioni e negli strumenti matematici che la sua ricerca ha saputo generare.

L'idea del progetto nasce dalla volontà di far sperimentare in prima persona ciò che per i matematici del Seicento rappresentava una vera competizione intellettuale tra menti brillanti. L'obiettivo è duplice: da un lato rendere tangibile l'evoluzione degli strumenti computazionali, mostrando come sfide che impegnarono i geni di tutta Europa siano oggi accessibili a chiunque grazie ai computer; dall'altro offrire un'esperienza divertente dove si può cercare la curva ottimale per via intuitiva, "andando a occhio".

Il software trasforma così un problema puramente teorico in un'attività pratica e interattiva: l'utente può disegnare curve arbitrarie e confrontare direttamente il tempo ottenuto con quello della soluzione ottimale, apprezzando visivamente e quantitativamente le differenze tra un percorso generico e la curva ideale.

Per realizzare questo obiettivo, l'applicazione mette a disposizione un'area di disegno dove tracciare il percorso e osservare il moto di una pallina lungo la rotta scelta. La velocità di percorrenza è mostrata sia attraverso una simulazione fisica in tempo reale, sia mediante il calcolo teorico preciso del moto sulla spezzata disegnata. Le curve possono essere fornite sia graficamente (tramite trascinamento del mouse) sia numericamente (tramite equazioni matematiche) (DA IMPLEMENTARE FORSE ALLA FINE), alimentando lo stesso motore di calcolo sottostante.

### Storia della Brachistocrona

Il problema della brachistocrona, dalla parola greca composta *brákhistos* (βράχιστος, "il più breve") e *khrónos* (χρόνος, "tempo"), rappresenta una delle pietre miliari della storia della matematica e del calcolo delle variazioni.

#### La formulazione di Galileo (1638)

Il problema fu formulato per la prima volta da Galileo Galilei nel 1638. Nel suo studio sul moto dei corpi, Galileo si chiese quale fosse la curva lungo la quale un punto materiale, scorrendo senza attrito sotto l'azione della gravità costante, impiegasse il tempo minimo per percorrere la distanza tra due punti fissati. Per motivi estetici e per la mancanza di strumenti matematici appropriati, dato che il calcolo infinitesimale era ancora agli albori, Galileo congetturò erroneamente che la soluzione fosse un arco di circonferenza. 

Questa ipotesi, sebbene sbagliata, non era irragionevole: come mostrato da Buttazzo e Mintchev, l'errore di valutazione di Galileo era sorprendentemente contenuto. In particolare, considerando il percorso necessario per tornare alla stessa altezza iniziale ($H=0$), il tempo di percorrenza lungo la cicloide è circa il **96%** di quello impiegato lungo una semicirconferenza. Questo rende l'intuizione di Galileo molto vicina alla soluzione ottimale dal punto di vista numerico, nonostante l'errore concettuale sulla forma della curva.

#### La sfida di Johann Bernoulli (1696)

La soluzione corretta arrivò quasi sessant'anni dopo, quando nel giugno 1696 Johann Bernoulli lanciò una sfida pubblica ai matematici di tutta Europa, pubblicando il problema sulla rivista *Acta Eruditorum* di Lipsia. La risposta non si fece attendere: entro la fine del 1697, soluzioni furono proposte da alcuni dei più grandi studiosi dell'epoca:

- **Isaac Newton**, che secondo la tradizione avrebbe risolto il problema in una sola notte, dopo averlo ricevuto per posta
- **Gottfried Wilhelm Leibniz**, co-fondatore del calcolo infinitesimale
- **Jacob Bernoulli**, fratello di Johann, che introdusse un metodo più generale
- Lo stesso **Johann Bernoulli**, con un approccio basato sul principio di Fermat
- **Guillaume de l'Hôpital**, autore del celebre trattato sulle coniche

Ognuno di questi matematici utilizzò metodi diversi, ma il vero valore di questa competizione intellettuale non fu solo la risposta in sé. La brachistocrona è storicamente significativa perché **per risolvere questo problema furono sviluppati strumenti matematici che si sarebbero rivelati fondamentali in moltissimi altri contesti**. La necessità di trovare una curva che minimizzasse un funzionale (e non semplicemente un numero) richiese un salto concettuale che portò alla nascita del calcolo delle variazioni.

#### La soluzione: un arco di cicloide

La soluzione si rivelò essere un **arco di cicloide**, la curva descritta da un punto fisso su una circonferenza che rotola senza strisciare lungo una retta. La soluzione in forma parametrica è:

$$
\begin{cases}
x(t) = c(t - \sin t) \\
y(t) = c(1 - \cos t)
\end{cases}
\qquad t \in [0, \tau]
$$

dove la costante $c$ e il parametro $\tau$ sono determinati dalle condizioni al contorno.

#### Nascita del calcolo delle variazioni

Il problema della brachistocrona è probabilmente il primo problema di calcolo delle variazioni in dimensione infinita. La formulazione matematica richiede di minimizzare il funzionale:

$$
T(u) = \frac{1}{\sqrt{2g}} \int_{x_1}^{x_2} \sqrt{\frac{1 + |u'(x)|^2}{u(x)}} \, dx
$$

che rappresenta il tempo totale di percorrenza. Dall'equazione di Eulero-Lagrange (nella forma integrata di DuBois-Reymond) si ottiene:

$$
(1 + |u'(x)|^2) \, u(x) = 2c
$$

Questa equazione differenziale non lineare portò alla soluzione cicloidale. Il tempo di percorrenza lungo la cicloide risulta circa il 16% inferiore rispetto al percorso rettilineo tra gli stessi due punti.

L'importanza storica di questo risultato va ben oltre la risposta specifica: le tecniche sviluppate per la brachistocrona gettarono le basi per intere nuove branche della matematica applicata, con applicazioni che spaziano dalla fisica teorica all'ingegneria, dalla robotica alla teoria del controllo ottimo.

#### Generalizzazioni

Il concetto di brachistocrona è stato esteso ben oltre il caso del campo gravitazionale costante. Nel lavoro di Buttazzo e Mintchev, il problema è stato generalizzato a campi di gravità newtoniani, incluso il celebre "tunnel brachistocrono" attraverso la Terra: un ipotetico treno che viaggiasse senza attrito sotto l'effetto della sola gravità tra due città impiegherebbe soltanto circa 11 minuti lungo la curva ottimale, contro i 42 minuti del percorso rettilineo, raggiungendo velocità superiori a 7000 km/h.

## Requisiti Funzionali

### Input e Disegno

- L'utente deve poter disegnare una curva a mano libera sull'area di disegno tramite il trascinamento del mouse
- Il sistema deve permettere l'inserimento di curve tramite equazioni matematiche, oltre all'input grafico (DA IMPLEMENTARE FORSE ALLA FINE)
- Il sistema deve poter campionare correttamente la posizione del mouse sull'area di disegno
- Il sistema deve garantire la monotonia crescente della curva rispetto all'asse X, rimuovendo i punti che violano questo vincolo
- Il sistema deve unire tutte le posizioni campionate per formare una curva continua
- Il sistema deve permettere di pulire l'area di disegno per creare una nuova curva

### Curve Predefinite e Calcolo Teorico

- Il sistema deve poter generare percorsi predefiniti: linea retta e cicloide
- Il sistema deve calcolare il tempo teorico di percorrenza sulla spezzata disegnata applicando la formula di Galileo:

$$
T = \sum_{i=1}^{n} \frac{\sqrt{\Delta x_i^2 + \Delta y_i^2}}{\sqrt{2g \cdot y_{\text{media},i}}}
$$

### Simulazione Fisica

- Il sistema deve eseguire una simulazione fisica della pallina che percorre la curva sotto l'effetto della gravità
- La velocità della pallina deve variare lungo il percorso in base alla quota, secondo il principio di conservazione dell'energia
- Il sistema deve misurare e visualizzare il tempo effettivo di percorrenza durante la simulazione
- Il sistema deve permettere di ripetere la simulazione sulla stessa curva

### Visualizzazione e Confronto

- Il sistema deve visualizzare il confronto tra tempo teorico e tempo effettivo di percorrenza
- Il sistema deve loggare informazioni di debug su console (parametri fisici, stato della simulazione)

## Requisiti Non Funzionali

### Prestazioni, Responsività e Logging

- Il loop di simulazione deve avvenire a una frequenza di ~60 FPS (circa 16ms per frame) gestendo correttamente il delta time (tempo virtuale) per permettere alla finestra dell'applicazione di rimanere responsiva
- Il sistema deve fornire informazioni di debug su console tramite la libreria `spdlog` configurata adeguatamente

### Calcolo e Simulazione Fisica

- Il tempo teorico è calcolato in modo esatto sulla spezzata: poiché la curva è costante a tratti su ogni segmento, la formula di Galileo fornisce il valore preciso senza approssimazioni numeriche
- La posizione della pallina in ciascun frame deve essere determinata collegando il tempo fisico reale con il fluire dei frame, garantendo coerenza fisica nella visualizzazione
- Il percorso disegnato deve essere sufficientemente lungo per rendere apprezzabile la differenza visiva e temporale tra le diverse curve confrontate

### Elaborazione e Rendering

- Per garantire la monotonia crescente della curva viene eseguito un postprocessing sui punti campionati al momento del rilascio del mouse (terminazione del disegno)
- La curva disegnata deve apparire continua all'utente, unendo tutte le posizioni campionate dal mouse
- La pallina deve rimanere visivamente sopra la curva senza compenetrazioni grafiche, calcolando la normale della superficie nel punto di contatto

## Requisiti di Sistema

- **Linguaggio**: C++17
- **Framework UI**: Qt 6.10
- **Calcolo Numerico**: Armadillo
- **Logging**: spdlog
- **Build System**: CMake 3.16+
