#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <termios.h>
#include <time.h>

#include "scan_horizontal.h"


#define TEST "1234567890"
#define TEST_LEN 11
#define MTU 1500
#define RECV_TIMEOUT 100




void print_progress_bar(int progress, int total) {
    const int width = 17; // largeur totale de la chaîne affichée
    int len = snprintf(NULL, 0, "%d/%d", progress, total); // longueur de la chaîne sans les espaces
    int spaces = width - len; // nombre d'espaces à ajouter

    printf("\r[%d/%d]%*s", progress, total, spaces, ""); // affiche la chaîne avec les espaces
    fflush(stdout);
}


// Structure d'un paquet ICMP
struct icmp_echo {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t ident;
    uint16_t seq;
    double sending_ts; // sending timestamp
    char test[TEST_LEN]; // Vérification de la non altération du paquet
};

// Renvoie le timestamp actuel
double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + ((double)tv.tv_usec) / 1000000;
}

// Permet de calculer le checksum d'un paquet ICMP
uint16_t calculate_checksum(unsigned char* buffer, int bytes)
{
    uint32_t checksum = 0;
    unsigned char* end = buffer + bytes;

    // Permet de traiter les paquets de taille impaire
    if (bytes % 2 == 1) {
        end = buffer + bytes - 1;
        checksum += (*end) << 8;
    }

    // On additionne les valeurs de chaque paire d'octets
    while (buffer < end) {
        checksum += buffer[0] << 8;
        checksum += buffer[1];
        buffer += 2;
    }

    // On ajoute les éventuels bits restants
    uint32_t carray = checksum >> 16;
    while (carray) {
        checksum = (checksum & 0xffff) + carray;
        carray = checksum >> 16;
    }
    // On prend le complément à 1 du résultat
    checksum = ~checksum;
    return checksum & 0xffff; // On ne garde que les 16 bits de poids faible
}

// Envoie un paquet ICMP de type ECHO_REQUEST
int send_echo_request(int sock, struct sockaddr_in* addr, int ident, int seq)
{
    // On crée un paquet ICMP
    struct icmp_echo icmp;
    bzero(&icmp, sizeof(icmp));
    // On remplit les champs du paquet
    icmp.type = 8; // Type de paquet ICMP Echo Request
    icmp.code = 0; // Code 0 pour un paquet ECHO
    icmp.ident = htons(ident); // Htons permet de convertir un entier court
    icmp.seq = htons(seq);
    // On remplit le champ test avec une chaîne de caractères
    strncpy(icmp.test, TEST, TEST_LEN);

    // On remplit le champ sending_ts avec le timestamp actuel
    icmp.sending_ts = get_timestamp();

    // On calcule le checksum du paquet
    icmp.checksum = htons(
        calculate_checksum((unsigned char*)&icmp, sizeof(icmp))
    );

    // On envoie le paquet ICMP
    int bytes = sendto(sock, &icmp, sizeof(icmp), 0,
        (struct sockaddr*)addr, sizeof(*addr));
    if (bytes == -1) {
        return -1;
    }

    return 0;
}

// Réceptionne un paquet ICMP de type ECHO_REPLY
int recv_echo_reply(int sock, int ident)
{
    // On crée un buffer pour stocker le paquet reçu
    char buffer[MTU];
    struct sockaddr_in peer_addr;

    // On reçoit le paquet ICMP
    socklen_t addr_len = sizeof(peer_addr);
    int bytes = recvfrom(sock, buffer, sizeof(buffer), 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    if (bytes == -1) {
        // Si le paquet n'est pas reçu dans les 100ms, on renvoie -1
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        }
        perror("recvfrom");
        return -1;
    }

    // On vérifie que le paquet reçu a la bonne taille
    if (bytes < (int)sizeof(struct icmp_echo)) {
        fprintf(stderr, "Paquet ICMP tronqué\n");
        return -1;
    }

    // On vérifie que le paquet reçu est bien un paquet ICMP de type ECHO_REPLY
    struct icmp_echo* icmp = (struct icmp_echo*)(buffer + 20);
    
    if (ntohs(icmp->ident) != ident) {
        //fprintf(stderr, "Identifiant ICMP invalide\n");
        return -1;
    }
  

    // On calcule le temps de réponse
    double rtt = (get_timestamp() - icmp->sending_ts) * 1000;

    // On print les infos du paquet reçu
    printf("%s en %5.2fms\n", inet_ntoa(peer_addr.sin_addr),rtt);

    return 0;
}


// Fonction principale qui imite la commande ping, prend une adresse IP et un nombre maximal de requêtes
int ping(const char *ip, int max_requests)
{
    // Vérification du nombre maximal de requêtes
    if (max_requests <= 0) {
        printf("Nombre maximal de requêtes invalide\n");
        return -1;
    }

    // On crée une structure sockaddr_in pour stocker l'adresse IP
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    // On remplit la structure avec l'adresse IP passée en argument
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    if (inet_aton(ip, (struct in_addr*)&addr.sin_addr.s_addr) == 0) {
        printf("Adresse Invalide\n");
        return -1;
    };

    // On crée un socket de type RAW permettant d'envoyer et de recevoir des paquets ICMP
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        perror("Création du socket échouée\n");
        return -1;
    }

    // On paramètre le timeout de réception à 100ms
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = RECV_TIMEOUT;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        perror("Set des options du socket échoué\n");
        close(sock); // Fermeture du socket en cas d'erreur
        return -1;
    }

    
    int requests_sent = 0;
    double next_ts = get_timestamp();
    u_int16_t ident = rand() & 0xFFFF; // Identifiant aléatoire
    int seq = 1;
    int succes = 0;

    while (requests_sent < max_requests) {
        // On envoie un paquet ICMP toutes les secondes
        if (get_timestamp() >= next_ts) {
            // On envoie un paquet ICMP
            if (send_echo_request(sock, &addr, ident, seq) == -1) {
                perror("L'envoi a échoué\n");
            }

            // On met à jour la séquence
            seq += 1;
            requests_sent++;
        }

        // Utilisation de select pour attendre soit une réponse, soit un timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        int retval = select(sock + 1, &readfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
            close(sock); // Fermeture du socket en cas d'erreur
            return -1;
        } else if (retval) {
            // Si le socket est prêt à lire, il y a une réponse
            if (recv_echo_reply(sock, ident) == -1) {
                //perror("La réception a échoué\n");
            }
            else {
                succes = 1;
                // On met à jour le timestamp pour la prochaine requête
                next_ts = get_timestamp() + 1;
            }
        }
    }
    
    // Si une erreur s'est produite pendant l'envoi ou la réception des requêtes
    if (succes == -1) {
        close(sock); // Fermeture du socket
        return -1; // Erreur
    }

    // Si aucune réponse n'a été reçue après avoir envoyé le nombre maximum de requêtes
    else if (!succes) {
        close(sock); // Fermeture du socket
        return 0; // Hôte injoignable
    }
    else {
        close(sock); // Fermeture du socket après la boucle
        return succes; // Hôte joignable
    }

    
}


// Fonction qui scanne un réseau en utilisant la commande ping
IPAddress* scan_reseau(const char *ip_reseau, int *nb_hotes) {
    char *ip_copie = strdup(ip_reseau);
    char *ip_str = strtok(ip_copie, "/");
    char *slash = strtok(NULL, "/");

    if (!ip_str || !slash) {
        printf("Format d'IP incorrect\n");
        return NULL;
    }

    int masque = atoi(slash);

    if (masque < 1 || masque > 32) {
        printf("Masque de sous-réseau invalide\n");
        return NULL;
    }

    struct sockaddr_in addr;
    if (inet_aton(ip_str, &(addr.sin_addr)) == 0) {
        printf("Adresse Invalide\n");
        return NULL;
    }

   
    // Calcul de l'adresse de réseau et de l'adresse de diffusion
    uint32_t ip_net = ntohl(addr.sin_addr.s_addr) & (~((1 << (32 - masque)) - 1));
    uint32_t broadcast = ip_net | ((1 << (32 - masque)) - 1);

    int total = broadcast - ip_net - 1;
    int progress = 0;

    //Print du réseau et du masque
    printf("\nLancement du scan sur le réseau : %s de  masque : %d\n\n[1/%i]             ", ip_str, masque, total);
    *nb_hotes = 0;

    // Allouer de l'espace pour le tableau dynamique d'adresses IP
    IPAddress* ip_list = malloc(total * sizeof(IPAddress));
    if (ip_list == NULL) {
        printf("Erreur lors de l'allocation de la mémoire\n");
        return NULL;
    }

    int index = 0;

    // Boucle sur toutes les adresses IP du réseau
    for (uint32_t ip = ip_net + 1; ip < broadcast; ip++) {
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(ip);
        char *ip_str = inet_ntoa(ip_addr);
        int succes = ping(ip_str, 1);
        if (succes >= 1) {
            // Ajouter l'adresse IP au tableau
            strcpy(ip_list[index].ip, ip_str);
            index++;
        }

        progress++;
        print_progress_bar(progress, total);
    }

    *nb_hotes = index; // Mettre à jour le nombre total d'hôtes connectés

    printf("\n\n    Scan terminé, %i hôtes connectés sur le réseau %s\n", *nb_hotes, ip_reseau);

    // Retourner le tableau d'adresses IP
    return ip_list;
}
