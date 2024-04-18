#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>


#include "scan_horizontal.h"
#include "scanport.h"

#define SERV_PORT 8080
#define MAX 1024

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

// Fonction pour gérer la connexion avec un client
void *handle_client(void *dialogSocket) {
    char buffer[MAX];
    int *new_sock = (int *)dialogSocket;

    // Envoyer le message de bienvenue
    char *welcome_message = "\n\n        Bienvenue dans Tnmap !\n";
    send(*new_sock, welcome_message, strlen(welcome_message), 0);
    memset(buffer, 0, sizeof(buffer));

    // Boucle pour gérer les interactions avec le client
    while (1) {
        // Envoyer le menu au client
        char *menu_message = "\nQue voulez-vous faire ?\n    1. Ping une adresse IP\n    2. Scanner toutes les adresses IP d'un réseau\n    3. Faire un scan des ports d'une machine\n    4. Quitter\n";
        send(*new_sock, menu_message, strlen(menu_message), 0);
        memset(buffer, 0, sizeof(buffer));

        // Recevoir la réponse du client
        ssize_t n = recv(*new_sock, buffer, MAX, 0);
        if (n <= 0) {
            // Si la connexion est fermée ou s'il y a une erreur, quitter la boucle
            break;
        }
        // Ajouter un caractère de fin de chaîne pour pouvoir traiter le message
        buffer[n] = '\0';

        // Traiter la réponse du client
        int choix = atoi(buffer);
        switch (choix) {
            case 1: {
                // Demander et recevoir l'adresse IP à ping
                char *ping_message = "Entrez l'adresse IP à ping (format : xxx.xxx.xxx.xxx) :\n";
                send(*new_sock, ping_message, strlen(ping_message), 0);
                memset(buffer, 0, sizeof(buffer));
                n = recv(*new_sock, buffer, MAX, 0);
                if (n <= 0) {
                    break;
                }
                buffer[n] = '\0';
                printf("Ping en cours de l'adresse %s ...\n", buffer);
                // Traiter le ping
                int result = ping(buffer, 5);
                if (result == -1) {
                    ping_message = "Erreur lors du ping\n";
                    send(*new_sock, ping_message, strlen(ping_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                } else if (result == 0) {
                    ping_message = "    Hôte injoignable\n";
                    send(*new_sock, ping_message, strlen(ping_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                } else {
                    ping_message = "    Hôte joignable\n";
                    // Envoie du résultat au client...
                    send(*new_sock, ping_message, strlen(ping_message), 0); 
                    memset(buffer, 0, sizeof(buffer)); 
                }
                

                break;
            }

            case 2: {
                // Demander et recevoir l'adresse IP du réseau à scanner
                char *scan_message = "Entrez l'adresse IP du réseau à scanner ainsi que son masque (format : xxx.xxx.xxx.xxx/n) :\n";
                send(*new_sock, scan_message, strlen(scan_message), 0);
                memset(buffer, 0, sizeof(buffer));// Vider le buffer après chaque envoi de message

                n = recv(*new_sock, buffer, MAX, 0);
                if (n <= 0) {
                    break;
                }
                buffer[n] = '\0';

                printf("Scan du réseau %s en cours...\n", buffer);
                //Envoyer au client le print précédent
                char start_message[MAX];

                // Formattez le message avec l'adresse IP et stockez-le dans scan_message
                sprintf(start_message, "Scan en cours sur l'adresse : %s\n", buffer);
                // Envoyez le message formaté au client
                send(*new_sock, start_message, strlen(start_message), 0);


                // Traiter le scan du réseau
                int nb_hotes = 0;
                IPAddress* ip_list = scan_reseau(buffer, &nb_hotes);
                if (ip_list != NULL) {
                    char result_message[MAX];
                    sprintf(result_message, "Résultat du scan du réseau :\n  Nombre d'hôtes joignables : %i\n", nb_hotes);
                    for (int i = 0; i < nb_hotes; i++) {
                        // Concaténer chaque adresse IP dans la chaîne de résultat
                        strcat(result_message, "  Liste des adresses IP joignables :\n  ");
                        strcat(result_message, ip_list[i].ip);
                        strcat(result_message, "\n  ");
                        // Ajoutez ici la gestion des erreurs si nécessaire
                    }
                    // Envoyer la chaîne de résultat au client
                    send(*new_sock, result_message, strlen(result_message), 0);

                    // Libérez la mémoire allouée pour le tableau d'adresses IP
                    free(ip_list);
                }
                break;
            }

            case 3: {
                // Scanner les ports d'une machine
                // Demander et recevoir l'adresse IP de la machine à scanner
                char *scan_ports_message = "Entrez l'adresse IP de la machine à scanner (format : xxx.xxx.xxx.xxx) :\n";
                send(*new_sock, scan_ports_message, strlen(scan_ports_message), 0);
                memset(buffer, 0, sizeof(buffer));
                n = recv(*new_sock, buffer, MAX, 0);
                if (n <= 0) {
                    break;
                }
                buffer[n] = '\0';

                // Déclarez un tableau de caractères pour stocker l'adresse IP
                char ip_dest[256];

                // Copiez l'adresse IP dans le tableau ip_dest
                strcpy(ip_dest, buffer);


                printf("Scan de la machine %s en cours...\n", buffer);
                // On vérifie en premier si l'ip est joignable
                int result = ping(ip_dest, 5);
                if (result == -1) {
                    scan_ports_message = "Erreur lors du ping\n";
                    send(*new_sock, scan_ports_message, strlen(scan_ports_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                    break;
                } else if (result == 0) {
                    scan_ports_message = "    Hôte injoignable\n";
                    send(*new_sock, scan_ports_message, strlen(scan_ports_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                    break;
                } else {
                    scan_ports_message = "    Hôte joignable\n";
                    // Envoie du résultat au client...
                    send(*new_sock, scan_ports_message, strlen(scan_ports_message), 0);  
                    memset(buffer, 0, sizeof(buffer));
                }

                // Demander et recevoir le choix de scan
                char *scan_choice_message = "Voulez-vous scanner :\n    1. Un port\n    2. Une plage de ports que vous choisissez\n    3. Une plage de ports de 1 à 1024\n";
                send(*new_sock, scan_choice_message, strlen(scan_choice_message), 0);
                memset(buffer, 0, sizeof(buffer));
                n = recv(*new_sock, buffer, MAX, 0);
                if (n <= 0) {
                    break;
                }
                buffer[n] = '\0';

                // Traiter le choix de scan
                int choix2 = atoi(buffer);
                if (choix2 == 1) {
                    // Demander et recevoir le port à scanner
                    char *port_message = "Entrez le port à scanner :\n";
                    send(*new_sock, port_message, strlen(port_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                    n = recv(*new_sock, buffer, MAX, 0);
                    if (n <= 0) {
                        break;
                    }
                    buffer[n] = '\0';

                    // Traiter le scan du port
                    int port = atoi(buffer);
                    printf("    Scan du port %i en cours...\n", port);
                    int scan_result = scan(ip_dest, port);
                    if (scan_result == 0) {
                        port_message = "    Port fermé\n";
                        send(*new_sock, port_message, strlen(port_message), 0);
                        memset(buffer, 0, sizeof(buffer));
                    } else {
                        port_message = "    Port ouvert\n";
                        send(*new_sock, port_message, strlen(port_message), 0);
                        memset(buffer, 0, sizeof(buffer));
                    }
                } else if (choix2 == 2) {
                    // Demander et recevoir la plage de ports à scanner
                    char *port_range_message = "Entrez le port de début :\n";
                    send(*new_sock, port_range_message, strlen(port_range_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                    n = recv(*new_sock, buffer, MAX, 0);
                    if (n <= 0) {
                        break;
                    }
                    buffer[n] = '\0';
                    int debut = atoi(buffer);

                    port_range_message = "Entrez le port de fin :\n";
                    send(*new_sock, port_range_message, strlen(port_range_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                    n = recv(*new_sock, buffer, MAX, 0);
                    if (n <= 0) {
                        break;
                    }
                    buffer[n] = '\0';
                    int fin = atoi(buffer);

                    // Traiter le scan de la plage de ports
                    char result_message[MAX];
                    sprintf(result_message, "Scan de la plage de ports de %d à %d :\n", debut, fin);
                    send(*new_sock, result_message, strlen(result_message), 0);
                    memset(buffer, 0, sizeof(buffer));

                    // Appeler scan_ports pour récupérer les ports ouverts
                    int nb_ports_ouverts = 0;
                    PortInfo* ports_ouverts = scan_ports(ip_dest, debut, fin, &nb_ports_ouverts);
                    if (ports_ouverts != NULL) {
                        // Envoyer les ports ouverts au client
                        for (int i = 0; i < nb_ports_ouverts; i++) {
                            sprintf(result_message, "Port ouvert : %d\n", ports_ouverts[i].port);
                            send(*new_sock, result_message, strlen(result_message), 0);
                        }
                        // Libérer la mémoire allouée pour les ports ouverts
                        free(ports_ouverts);
                    } else {
                        // En cas d'erreur lors de l'appel à scan_ports
                        char *error_message = "Erreur lors du scan des ports\n";
                        send(*new_sock, error_message, strlen(error_message), 0);
                    }
                } else if (choix2 == 3) {
                    // Traiter le scan de la plage de ports de 1 à 1024
                    char result_message[] = "Scan de la plage de ports de 1 à 1024 :\n";
                    send(*new_sock, result_message, strlen(result_message), 0);
                    memset(buffer, 0, sizeof(buffer));

                    // Appeler scan_ports pour récupérer les ports ouverts
                    int nb_ports_ouverts = 0;
                    PortInfo* ports_ouverts = scan_ports(ip_dest, 1, 1024, &nb_ports_ouverts);
                    if (ports_ouverts != NULL) {
                        // Envoyer les ports ouverts au client
                        for (int i = 0; i < nb_ports_ouverts; i++) {
                            sprintf(result_message, "Port ouvert : %d\n", ports_ouverts[i].port);
                            send(*new_sock, result_message, strlen(result_message), 0);
                        }
                        // Libérer la mémoire allouée pour les ports ouverts
                        free(ports_ouverts);
                    } else {
                        // En cas d'erreur lors de l'appel à scan_ports
                        char *error_message = "Erreur lors du scan des ports\n";
                        send(*new_sock, error_message, strlen(error_message), 0);
                    }
                } else {
                    // Message d'erreur pour une entrée invalide
                    char *error_message = "Choix invalide\n";
                    send(*new_sock, error_message, strlen(error_message), 0);
                    memset(buffer, 0, sizeof(buffer));
                }
                break;
             
            }

            case 4: {
                // Message de fermeture et quitter la boucle
                // Déconnexion du client
                printf("Déconnexion demandée par le client\nVous pouvez fermer le serveur avec Ctrl+C ou attendre une nouvelle connexion");
                close(*new_sock);
                free(new_sock);
                pthread_exit(NULL);
                break;
            }
            // Ajoutez des cas pour les autres options du menu
            // Assurez-vous d'envoyer les messages appropriés au client
            default: {
                // Message d'erreur pour une entrée invalide
                char *error_message = "Choix invalide\n";
                send(*new_sock, error_message, strlen(error_message), 0);
                memset(buffer, 0, sizeof(buffer));
                break;
            }
        }
    }

    // Fermer la connexion lorsque le client se déconnecte
    printf("Déconnexion du client\n");
    close(*new_sock);
    free(new_sock);
    pthread_exit(NULL);
}

int main() {
    // Création du socket
    int serverSocket;
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("erreur socket");
        exit (1);
    }

    // Lier l'adresse local et le port au socket
    memset (&serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(serverSocket,(struct sockaddr *)&serv_addr, sizeof(serv_addr) ) <0) {
        perror ("servecho: erreur bind\n");
        exit (1);
    }

    // Ecoute les connexions entrantes et paramètre le nombre de connexions en attente
    if (listen(serverSocket,SOMAXCONN) <0) {
        perror ("servecho: erreur listen\n");
        exit (1);
    }

    // Le serveur attend une connexion
    printf("Le serveur écoute à présent sur le port 8080... \n En attente d'une connexion\n");

    // Accepter les connexions des clients
    int dialogSocket;
    socklen_t clilen;
    clilen = sizeof(cli_addr);

    while (1) {
        dialogSocket = accept(serverSocket,(struct sockaddr *)&cli_addr, &clilen);
        if (dialogSocket < 0) {
            perror("servecho : erreur accept\n");
            exit (1);
        }

        // Afficher un message pour prévenir qu'un client s'est connecté
        printf("Client connecté. En attente de commandes pour TNmap de la part du client...\n");

        // Créer un thread pour gérer la connexion avec le client
        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = dialogSocket;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) < 0) {
            perror("servecho: erreur création thread\n");
            exit(1);
        }
    }

    // Fermer le socket du serveur lorsque le serveur se termine (cela ne sera pas atteint dans cette implémentation)
    close(serverSocket); 
    return 0;
}
