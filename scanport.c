#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


// scan d'un port 
void scan (const char *ip, int port) {

    // On crée une structure sockaddr_in pour stocker l'adresse IP
    struct sockaddr_in addr;

    // On remplit la structure avec l'adresse IP passée en argument
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // On crée une socket de type TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // Si la socket n'est pas créée, on affiche un message d'erreur
    if (sock == -1) {
        perror("socket");
        return;
    }

    // On tente de se connecter au port
    int connection = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    // Si la connexion est établie, on affiche un message
    if (connection == 0) {
        printf("Port %d ouvert\n", port);
    }

    // On ferme la socket
    close(sock);
}

// Fonction qui scan plusieurs ports 
void scan_ports(const char *ip, int start, int end) {
    // On vérifie que le port de début est inférieur au port de fin
    if (start > end) {
        printf("Port de début supérieur au port de fin\n");
        return;
    }

    // On scanne les ports de start à end
    for (int port = start; port <= end; port++) {
        scan(ip, port);
    }
    printf("Scan terminé\n");
}
