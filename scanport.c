#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void print_progress_bar_port(int progress, int total) {
    const int width = 17; // largeur totale de la chaîne affichée
    int len = snprintf(NULL, 0, "%d/%d", progress, total); // longueur de la chaîne sans les espaces
    int spaces = width - len; // nombre d'espaces à ajouter

    printf("\r[%d/%d]%*s", progress, total, spaces, ""); // affiche la chaîne avec les espaces
    fflush(stdout);
}

// Balayage d'un port avec la méthode TCP connect 
// scan d'un port 
int scan (const char *ip, int port) {
    // On crée le retour, 1 si le port est ouvert, 0 sinon
    int retour = 0;
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
        return 0;
    }

    // On tente de se connecter au port
    int connection = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    // Si la connexion est établie, on affiche un message
    if (connection == 0) {
        printf("    Port %d ouvert\n", port);
        retour = 1;
    }
    else
    {
        retour = 0;   
    }

    // On ferme la socket
    close(sock);
    return retour;
}

// Fonction qui scan plusieurs ports 
void scan_ports(const char *ip, int start, int end) {
    // On vérifie que le port de début est inférieur au port de fin
    if (start > end) {
        printf("    Port de début supérieur au port de fin\n");
        return;
    }

    // On scanne les ports de start à end
    for (int port = start; port <= end; port++) {
        print_progress_bar_port(port, end);
        scan(ip, port);
    }
    printf("Scan terminé\n");
}
