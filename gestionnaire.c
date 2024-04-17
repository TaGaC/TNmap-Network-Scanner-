#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scan_horizontal.h"
#include "scanport.h"

void afficher_menu() {
    printf("\nQue voulez-vous faire ?\n");
    printf("    1. Ping une adresse IP\n");
    printf("    2. Scanner toutes les adresses IP d'un réseau\n");
    printf("    3. Faire un scan des ports d'une machine\n");
    printf("    4. Quitter\n");
}

int main() {
    printf("\n\n        Bienvenue dans Tnmap !\n");
    int choix = 0;

    while (choix != 4) {
        afficher_menu();
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                printf("    Entrez l'adresse IP à ping (format : xxx.xxx.xxx.xxx) :\n\n");
                char ip[16];
                scanf("%15s", ip);
                printf("\n   Ping en cours de l'adresse %s ...\n", ip);
               
                int result = ping(ip, 5);
                if (result == -1) {
                    printf("\n  Erreur lors du ping\n");
                } else if (result == 0) {
                    printf("\n  Hôte injoignable\n");
                } else {
                    printf("\n  Hôte joignable\n");
                }
                break;
            }
            case 2: {
                printf("    Entrez l'adresse IP du réseau à scanner ainsi que son masque (format : xxx.xxx.xxx.xxx/n) :\n");
                char reseau[16];
                scanf("%15s", reseau);
                printf("\n");
                scan_reseau(reseau);
                break;
            }
            case 3: {
                printf("    Entrez l'adresse IP de la machine à scanner (format : xxx.xxx.xxx.xxx) :\n");
                char ip[16];
                scanf("%15s", ip);
                int choix2 = 0;
                printf("\n");
                printf("\nVoulez-vous scanner :\n");
                printf("1. Un port\n");
                printf("2. Une plage de ports que vous choisissez\n");
                printf("3. Une plage de ports de 1 à 1024\n");
                scanf("%d", &choix2);

                if (choix2 == 1) {
                    printf("    Entrez le port à scanner :\n");
                    int port;
                    scanf("%d", &port);
                    scan(ip, port);
                } else if (choix2 == 2) {
                    printf("    Entrez le port de début :\n");
                    int debut;
                    scanf("%d", &debut);
                    printf("    Entrez le port de fin :\n");
                    int fin;
                    scanf("%d", &fin);
                    scan_ports(ip, debut, fin);
                } else if (choix2 == 3) {
                    scan_ports(ip, 1, 1024);
                } else {
                    printf("Choix invalide\n");
                }
                break;
            }
            case 4: {
                printf("\n  Fermeture de Tnmap !\n");
                break;
            }
            default: {
                printf("Choix invalide\n");
                break;
            }
        }
    }

    return 0;
}
