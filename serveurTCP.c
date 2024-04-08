//Auteur: Thomas JEANJACQUOT
//Date: 05/03/2025

// Création d'un serveur TCP permettant de communiquer avec un client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>

#define SERV_PORT 8080
#define MAX 1024


struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

// Fonction pour envoyer un message, on crée un thread pour cette fonction afin qu'elle puisse s'exécuter en parallèle de l'écoute du client
void *send_message(void *dialogSocket) {
    char buffer[MAX];
    int *new_sock = (int *)dialogSocket;

    while (1) {
        fgets(buffer, MAX, stdin);
        send(*new_sock, buffer, strlen(buffer), 0);
    }

    return NULL;
}

int main() {

    char buffer[MAX] = {0};

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


    // Création du socket de dialogue
    int dialogSocket;
    int clilen;
    clilen = sizeof(cli_addr);

    // Boucle pour accepter plusieurs connexions de clients
    while (1) {
        dialogSocket = accept(serverSocket,(struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (dialogSocket < 0) {
            perror("servecho : erreur accep\n");
            exit (1);
        }
        // Afficher l'adresse IP du client
        printf("Connexion acceptée depuis localhost\n");

        // Envoyer le message de bienvenue
        char *hello = "Bonjour depuis le serveur\n";
        ssize_t n = send(dialogSocket, hello, strlen(hello), 0);
        if (n < 0) {
            perror("servecho: erreur send\n");
            exit(1);
        }

        pthread_t send_thread;
        if (pthread_create(&send_thread, NULL, send_message, (void *)&dialogSocket) < 0) {
            perror("servecho: erreur création thread\n");
            exit(1);
        }

        // Boucle pour lire les messages du client
        char buffer[256];
        while ((n = recv(dialogSocket, buffer, 255, 0)) > 0) { // Par défaut le serveur attend un message de 255 caractères maximum et si on ferme la connexion, la fonction recv renvoie 0
            buffer[n] = '\0';  // Ajouter un caractère de fin de chaîne pour pouvoir imprimer le message
            printf("Message reçu: %s", buffer);
            
            // Si le client envoie "quit", fermer la connexion et arrêter le serveur
            if (strncmp(buffer, "quit", 4) == 0) {
                printf("Arrêt du serveur...\n");
                pthread_cancel(send_thread);
                close(dialogSocket);
                close(serverSocket);
                exit(0);
            } 
        }
        close(dialogSocket);

        
    }

    // Fermer le socket du serveur lorsque le serveur se termine
    close(serverSocket);
    
}