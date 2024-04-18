CC = clang
CFLAGS = -Wall -Wextra -pedantic -O0 -g3 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls
LDFLAGS = -lpthread

.PHONY: all clean clean_exe tnmap_client tnmap_serveur tnmap_all

tnmap_client: TNmap_client

tnmap_serveur: TNmap_serveur

tnmap_all: TNmap_client TNmap_serveur

TNmap_serveur: serveurTCP

TNmap_client: clientTCP

serveurTCP: serveurTCP.o scan_horizontal.o scanport.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clientTCP: clientTCP.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

serveurTCP.o: serveurTCP.c
	$(CC) -c $(CFLAGS) $^

clientTCP.o: clientTCP.c
	$(CC) -c $(CFLAGS) $^

scan_horizontal.o: scan_horizontal.c
	$(CC) -c $(CFLAGS) $^

scanport.o: scanport.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f *.o

clean_exe:
	rm -f serveurTCP clientTCP scanport gestionnaire scan
