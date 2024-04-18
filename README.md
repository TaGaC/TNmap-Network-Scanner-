# Projet : Conception d’un scanner réseau simple

## Travail à réaliser

En partant du couple client-serveur TCP du TP, développer un scanner réseau simple configurable à distance via un gestionnaire :

- **Contrôle distant :** Le gestionnaire permet de configurer à distance un scanner réseau simple (similaire à nmap) qui réalise la cartographie d’un réseau local, les résultats d’analyse du scanner sont transmis en retour au gestionnaire (qui les affiche).
- **Scan horizontal :** Le scanner est capable de scanner un réseau/sous-réseau local pour déterminer les machines qui sont opérationnelles (en supposant que le service ping soit bien activé sur les machines).
- **Scan vertical :** Le scanner est capable de déterminer les ports qui sont ouverts sur une machine donnée à partir de son adresse IP.

Il vous est demandé de développer par vous-même le scanner réseau, et de ne pas simplement réutiliser des commandes systèmes (comme la commande ping). Attention : veillez à bien réaliser les tests dans un environnement où vous y êtes autorisé (salles TP). Ceux qui avancent vite peuvent implanter plusieurs techniques de balayage de ports.

## TNmap : Notre scanner réseau

### Utilisation

Notre explorateur de réseaux TNmap permet à un utilisateur sur un PC d'exécuter des tests réseau (ping, scan d'adresses IP d'un réseau, scan d'un port ou de tous les ports d'une adresse IP) sur un serveur distant à l'aide d'une connexion TCP. Une fois la connexion TCP établie, l'utilisateur saisit les commandes à envoyer au serveur pour l'exécution des tests.

### Compilation

Pour compiler le programme, exécutez les commandes suivantes dans votre terminal :

```bash
make TNmap_client
make TNmap_serveur

```

###  Lancement du serveur

Lancez le serveur à l'aide de la commande suivante :

```bash
sudo ./TNmap_serveur
```


###  Lancement du client

Ensuite, lancez le client avec la commande suivante en remplaçant ip_du_serveur par l'adresse IP du serveur distant :

```bash
sudo ./TNmap_client
```

## Remarques

- Le préfixe `sudo` est utilisé pour écrire à l'aide de sockets raw.
- Des erreurs de segmentation peuvent s'afficher lors de l'exécution. Si c'est le cas, relancez une ou deux fois l'exécution pour que le programme démarre correctement. Nous n'avons pas trouvé la source de ces erreurs qui n'interviennent pas en permanence.

## Fonctionnalités

Une fois la connexion établie, un message "Bienvenue dans TNmap!" s'affichera sur le terminal du client, vous laissant le choix entre différentes options de notre gestionnaire. Vous pourrez ainsi :

- Pinger une adresse précise.
- Identifier l'ensemble des adresses IP actives sur un réseau donné en entrée avec son masque en /x.
- Effectuer un scan d'un port d'une adresse IP ou sélectionner une plage de ports à scanner.

Lors de l'exécution des scans, des informations sur l'avancement des scans sont affichées sur le terminal du serveur. Le résultat final est affiché sur le terminal du client. N'hésitez pas à comparer les informations contenues dans les deux terminaux.

### Réalisation

Ce travail a été réalisé par Thomas JEANJACQUOT et Amandine LAPIQUE-FAVRE.
