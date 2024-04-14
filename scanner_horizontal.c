#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <sys/select.h>
#include <time.h>

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char sendline[1024];
    char recvline[1024];
    int n;
    socklen_t len;
    unsigned int addr;

    // On récupère l'adresse IP en argument
    if (argc != 2) {
        printf("Usage: %s <IP address>\n", argv[0]);
        exit(1);
    }

    // On crée un socket de type RAW permettant d'envoyer et de recevoir des paquets ICMP
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        exit(1);
    }
    printf("En attente de connexion...\n");

    // On prépare les adresses IP du serveur et du client
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr); // Convertit l'adresse IP en binaire
    servaddr.sin_addr.s_addr = addr;

    // On crée un paquet ICMP de type ECHO (utilisé dans la commande ping)
    struct icmp *icmp_packet = malloc(sizeof(struct icmp));
    struct icmphdr *icmp_header = &icmp_packet->icmp_hun;
    icmp_header->type = 8; // Type de paquet ICMP Echo Request
    icmp_header->code = 0; // Code 0 pour un paquet ECHO
    icmp_header->checksum = 0; // Le checksum sert à vérifier l'intégrité du paquet
    icmp_header->un.echo.id = getpid(); // On utilise l'ID du processus pour l'ID du paquet, permet de distinguer les paquets
    icmp_header->un.echo.sequence = 0; // On initialise la séquence à 0, qui sera incrémentée à chaque paquet

    // Calcul du checksum
    icmp_header->checksum = csum((unsigned short *) icmp_packet, sizeof(struct icmp) / sizeof(unsigned short));

    // On envoie le paquet ICMP au serveur
    len = sizeof(cliaddr);
    n = sendto(sockfd, icmp_packet, sizeof(struct icmp), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (n < 0) {
        perror("sendto");
        exit(1);
    }
    printf("Paquet ICMP envoyé\n");

    // On attend la réponse du serveur, on stocke l'addresse du client dans cliaddr et la taille de l'adresse dans len
    // Définir un timeout de 5 secondes
    struct timeval tv;
    tv.tv_sec = 6;
    tv.tv_usec = 0;

    // Créer un ensemble de descripteurs de fichiers vides
    fd_set readfds;
    FD_ZERO(&readfds);

    // Ajouter le descripteur de fichier du socket à l'ensemble de descripteurs de fichiers à surveiller
    FD_SET(sockfd, &readfds);

    // Attendre pendant 5 secondes pour voir s'il y a des données à lire sur le socket
    int result = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (result == -1) {
        perror("select");
        exit(1);
    } else if (result == 0) {
        printf("Temps d'attente    "); 
        } 
    
    else {
        // Il y a des données à lire sur le socket
        n = recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom");
            exit(1);
        }
        printf("Réponse reçue\n");

        // Traitement de la réponse
        // On récupère le header ICMP de la réponse
        icmp_header = (struct icmphdr *)recvline;
        if (icmp_header->type == ICMP_ECHOREPLY) { // Si le type du paquet est ECHOREPLY, le serveur a répondu à la requête ICMP ECHO
            printf("Host %s is up\n", argv[1]);
        } else {
            printf("Host %s is down\n", argv[1]);
        }
    } 

    close(sockfd);
    free(icmp_packet);
    return 0;
}

