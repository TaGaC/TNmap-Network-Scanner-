#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#define TEST "1234567890"
#define TEST_LEN 11
#define MTU 1500
#define RECV_TIMEOUT 1000

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
    int addr_len = sizeof(peer_addr);
    int bytes = recvfrom(sock, buffer, sizeof(buffer), 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    if (bytes == -1) {
        // Si le paquet n'est pas reçu dans les 100ms, on renvoie 0
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return -1;
    }

    // On verifie que le paquet reçu est bien un paquet ICMP
    struct icmp_echo* icmp = (struct icmp_echo*)(buffer + 20);
    if (icmp->type != 0 || icmp->code != 0) {
        return 0;
    }
    if (ntohs(icmp->ident) != ident) {
        return 0;
    }

    // On print les infos du paquet reçu
    printf("%s seq=%d %5.2fms\n",
        inet_ntoa(peer_addr.sin_addr),
        ntohs(icmp->seq),
        (get_timestamp() - icmp->sending_ts) * 1000
    );

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
    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret == -1) {
        perror("Set des options du socket échoué\n");
        return -1;
    }

    int requests_sent = 0;
    double next_ts = get_timestamp();
    int ident = getpid();
    int seq = 1;
    int succes = 0;

    while (requests_sent < max_requests) {
        // On envoie un paquet ICMP toutes les secondes
        if (get_timestamp() >= next_ts) {
            // On envoie un paquet ICMP
            ret = send_echo_request(sock, &addr, ident, seq);
            if (ret == -1) {
                perror("L'envoi a échoué\n");
            }

            // On met à jour le timestamp et la séquence
            next_ts += 1;
            seq += 1;
            requests_sent++;
        }

        // Utilisation de select pour attendre soit une réponse, soit un timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int retval = select(sock + 1, &readfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
            return -1;
        } else if (retval) {
            // Si le socket est prêt à lire, il y a une réponse
            ret = recv_echo_reply(sock, ident);
            if (ret == -1) {
                perror("La réception a échoué\n");
            }
            else {
                succes = 1;
            }
        }
     
        // Réinitialiser la temporisation avant la prochaine itération
        tv.tv_sec = 1;
        tv.tv_usec = 0;
    }
    
    return succes;
}

int main(int argc, const char* argv[])
{
    // Vérification du nombre d'arguments
    if (argc != 3) {
        printf("Usage: %s <IP address> <max_requests>\n", argv[0]);
        return -1;
    }

    // Conversion de la chaîne de caractères en entier
    int max_requests = atoi(argv[2]);
    printf("max_requests = %d\n", max_requests);

    // Appel de la fonction ping avec l'adresse IP et le nombre maximal de requêtes
    int up = ping(argv[1], max_requests);
    if (up == 0) {
        printf("Hôte injoignable\n");
    }
    else if (up == -1) {
        printf("Erreur lors du ping\n");
    }
    else {
        printf("Hôte joignable\n");
    }
    
}