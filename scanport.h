#ifndef SCANPORT_H
#define SCANPORT_H

// Structure pour stocker les informations sur les ports ouverts
typedef struct {
    int port;
} PortInfo;


int scan(const char *ip, int port);
PortInfo* scan_ports(const char *ip, int start, int end, int *nb_ports_ouverts);
#endif // SCANPORT_H
