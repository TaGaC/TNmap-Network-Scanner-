#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define SERV_PORT 8080
#define MAX 1024

struct sockaddr_in serv_addr;

// Fonction pour gérer la réception des réponses du serveur
void *receive_response(void *sock) {
    int clientSocket = *((int *)sock);
    char buffer[MAX];

    while (1) {
        ssize_t n = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (n < 0) {
            perror("Erreur lors de la réception de la réponse du serveur");
            exit(1);
        } else if (n == 0) {
            // Connexion fermée par le serveur
            printf("Déconnexion du serveur\n");
            break;
        }
        printf("%s\n", buffer);
        // Vide le buffer 
        memset(buffer, 0, sizeof(buffer));
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP du serveur>\n", argv[0]);
        exit(1);
    }

    char buffer[MAX] = {0};

    // Création du socket
    int clientSocket;
    if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    // Lier l'adresse du serveur et le port au socket
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // Utilise l'adresse IP passée en argument

    // Connexion au serveur
    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        exit(1);
    }

    // Recevoir le message de bienvenue du serveur
    char server_message[MAX];
    ssize_t n = recv(clientSocket, server_message, sizeof(server_message), 0);
    if (n < 0) {
        perror("Erreur lors de la réception du message du serveur");
        exit(1);
    }
    printf("Message du serveur: %s\n", server_message);

    // Créer un thread pour recevoir les réponses du serveur
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_response, (void *)&clientSocket) != 0) {
        perror("Erreur lors de la création du thread de réception");
        exit(1);
    }

    // Boucle pour envoyer des commandes au serveur
    while (1) {
        fgets(buffer, MAX, stdin);

        // Supprimer le caractère de nouvelle ligne de la commande
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Envoyer la commande au serveur
        send(clientSocket, buffer, strlen(buffer), 0);

        // Si la commande est "quit", quitter la boucle
        if (strncmp(buffer, "quit", 4) == 0 || strncmp(buffer, "4", 1) == 0) {
            printf("Fermeture de TNmap et déconnexion du serveur\n");
            break;
        }
        // Vide le buffer
        memset(buffer, 0, sizeof(buffer));
    }

    // Fermer la connexion
    close(clientSocket);

    return 0;
}
