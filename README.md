## Projet : conception d’un scanner réseau simple 

# Travail à réaliser

En partant du couple client-serveur TCP du TP, développer un scanner réseau simple configurable
à distance via un gestionnaire :
- Contrôle distant : le gestionnaire permet de configurer à distance un scanner réseau
simple (similaire à nmap) qui réalise la cartographie d’un réseau local, les résultats
d’analyse du scanner sont transmis en retour au gestionnaire (qui les affiche),
- Scan horizontal : le scanner est capable de scanner un réseau/sous-réseau local pour
déterminer les machines qui sont opérationnelles (en supposant que le service ping
soit bien activé sur les machines),
- Scan vertical : le scanner est capable de déterminer les ports qui sont ouverts sur une
machine donnée à partir de son adresse IP.

Il vous est demandé de développer par vous-même le scanner réseau, et de ne pas
simplement réutiliser des commandes systèmes (comme la commande ping).   
Attention : veillez à bien réaliser les tests dans un environnement où vous y êtes autorisé (salles TP).
Ceux qui avancent vite peuvent implanter plusieurs techniques de balayage de ports.


https://nmap.org/docs.html   
https://nmap.org/man/fr/man-port-scanning-techniques.html

# Notre scanner

Nous avons réalisé un scanner répondant aux attentes du sujet. 
Afin d'utiliser notre scanner TNmap il faut lancer la commande : $sudo ./TNmap
