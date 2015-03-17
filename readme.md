## About
Emerald è un tema minimale realizzato per Jekyll. Lo scopo principale di Emerald è quello di fornire un tema chiaro e pulito per chi desidera un blog pronto all’uso e che punti ad essere facilmente fruibile in primo luogo dai dispositivi mobili.

![Emerald](/img/Emerald01.png "Emerald")

### Setup
Emerald può essere installato semplicemente [scaricando dal repository](https://github.com/KingFelix/emerald/archive/master.zip) la cartella .zip che lo contiene.
Una volta estratto il contenuto dalla cartella nella directory prescelta, è sufficiente lanciare ``jekyll serve`` da terminale per visualizzarlo in locale all'indirizzo ``0.0.0.0:4000/emerald/``.

In alternativa è possibile eseguire un fork del repository direttamente da Github e utilizzare Github Pages come hosting. Utilizzando questa opzione sarà sufficiente modificare il valore di ``baseurl`` all'interno del file ``_config.yml``, sostituendolo con la directory nel quale si desidera installare Emerald o semplicemente lasciare il simbolo "/" (lo slash), qual ora si desiderasse installare Emerald nella root del proprio progetto.

Come ultima cosa, per saperne di più su Jekyll il miglior punto di partenza è la [documentazione sul sito ufficiale](http://jekyllrb.com)!

### Baseurl
Emerald è stato pensato per essere utilizzato principalmente attraverso Github, in special modo all'interno di [project site](https://pages.github.com/). Per questo motivo sono stati inseriti diversi tag ``{{ site.baseurl }}`` all'interno del sito. Se si desidera utilizzare Emerald, ad esempio, come sistema di blog all'interno di un sito personale ad esempio, è sufficiente modificare la voce "baseurl" nel file ``config.yml`, sostituendo il valore "/emerald" con, ad esempio "/blog", o qualsiasi cosa si desideri.

### Typografia
Per i contenuti e per mantenere il ritmo verticale, è stata applicata una la scala tipografica come scala modulare e un valore di 24px come baseline. Per mantenere il giusto ritmo verticale, occorre inserire immagini e altri contenuti, tenendo come riferimento per l'altezza un 24 o un multiplo di 24.

## Author

### Jacopo Rabolini

- Web site: [www.jacoporabolini.com](http://www.jacoporabolini.com)
- Google+: [+JacopoRabolini](https://plus.google.com/u/0/+JacopoRabolini/posts)

## License
Emerald is released under [MIT License]({{ site.baseurl }}/license.html).
