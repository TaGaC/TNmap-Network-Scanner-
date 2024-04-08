//Auteur: Thomas JEANJACQUOT
//Date: 05/03/2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#define SERV_PORT 8080
#define SERV_IP "127.0.0.1"
#define MAX 1024

// Pour compiler ce programme, il faut ajouter l'option -lpthread

struct sockaddr_in serv_addr;

void *receive_message(void *clientSocket) {
    char buffer[MAX];
    int *new_sock = (int *)clientSocket;

    while (1) {
        memset(buffer, 0, sizeof(buffer));  // Vider le buffer
        recv(*new_sock, buffer, MAX, 0);
        printf("Message reçu: %s\n", buffer);
    }

    return NULL;
}

int main() {
    char buffer[MAX] = {0}; 

    // Création du socket
    int clientSocket;
    if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("erreur socket");
        exit (1);
    }

    // Lier l'adresse local et le port au socket
    memset (&serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    if (connect (clientSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0){
        perror ("cliecho : erreur connect");
        exit (1);
    }

    printf("Connecté au serveur\n");

    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_message, (void *)&clientSocket) < 0) {
        perror("erreur création thread\n");
        exit(1);
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));  // Vider le buffer
        // Envoyer un message au serveur
        fgets(buffer, MAX, stdin);
        send(clientSocket, buffer, strlen(buffer), 0);

        // Si le message est "quit", on se décconecte du serveur
        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Déconnexion du serveur\n");
            printf("Fermeture du client\n");
            break;
        }
    }

    pthread_cancel(receive_thread);
    close(clientSocket);

    return 0;
}