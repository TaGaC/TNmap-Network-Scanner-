#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scan_horizontal.h"
#include "scanport.h"



// Fonction qui permet de scanner le réseau
void scanReseau(const char *ip){
    // On sépare l'ip de la plage
    char *ip_copie = strdup(ip);
    char *ip_reseau = strtok(ip_copie, "/");
    char *plage = strtok(NULL, "/");

    // On récupère le masque
    int masque = atoi(plage);
    // On lance ping sur chaque ip

}




int main(int argc, char *argv[]){
    // On affiche le menu 
    // On demande à l'utilisateur de choisir une option parmis : 
    // 1. Faire un scan du réseau
    // 2. Faire un scan des ports d'une machine dont on nous donne l'info 

    printf("Que voulez vous faire ?\nPour faire un scan du réseau, tapez 1\nPour faire un scan des ports d'une machine, tapez 2\n");

    // On récupère le choix de l'utilisateur
    int choix = 0;
    scanf("%d", &choix);

    // On appelle la fonction correspondante
    if(choix == 1){
        printf("Scan du réseau\n");
        printf("Entrez l'adresse IP du réseaux à scanner: xxx.xxx.xxx.xxx/n \n");
        char reseau[16];
        scanf("%s", reseau);
        int result = ping(reseau, 10);
        if (result == -1) {
            printf("Erreur lors du ping\n");
        } else if (result == 0) {
            printf("Hôte injoignable\n");
        } else {
            printf("Hôte joignable\n");
        }

    }else if(choix == 2){
        printf("Entrez l'adresse IP de la machine à scanner au format: xxx.xxx.xxx.xxx \n");
        char ip[16];
        scanf("%s", ip);
        int choix2 = 0;
        printf("Voulez vous scanner un port ou une plage de ports ?\nPour scanner un port, tapez 1\nPour scanner une plage de ports que vous choississez, tapez 2\nPour scanner une plage de ports de 1 à 1024, tapez 3\n");
        scanf("%d", &choix2);
        if(choix2 == 1){
            printf("Entrez le port à scanner\n");
            int port;
            scanf("%d", &port);
            scan(ip, port);
        }else if(choix2 == 2){
            printf("Entrez le port de début\n");
            int debut;
            scanf("%d", &debut);
            printf("Entrez le port de fin\n");
            int fin;
            scanf("%d", &fin);
            scan_ports(ip, debut, fin);
        }else if(choix2 == 3){
            scan_ports(ip, 1, 1024);
        }else{
            printf("Choix invalide\n");
        }
    }else{
        printf("Choix invalide\n");
    }

    // On demande à l'utilisateur s'il veut continuer
    printf("Voulez vous continuer ?\nPour continuer, tapez 1\nPour quitter, tapez 2\n");

    // Si oui, on recommence
    int continuer = 0;
    scanf("%d", &continuer);
    if(continuer == 1){
        main(argc, argv);
    }
    // Si non, on quitte
    else{
        return 0;
    }
}
