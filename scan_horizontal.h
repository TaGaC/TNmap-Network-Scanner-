#ifndef SCAN_HORIZONTALE_H
#define SCAN_HORIZONTALE_H

typedef struct {
    char ip[16]; // Pour stocker une adresse IP au format xxx.xxx.xxx.xxx\0
} IPAddress;



int ping(const char *ip, int max_requests);
IPAddress* scan_reseau(const char *ip_reseau, int *nb_hotes);

#endif // SCAN_HORIZONTALE_H
