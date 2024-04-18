CC = clang
CFLAGS = -Wall -Wextra -pedantic -O0 -g3 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls
LDFLAGS = -lpthread

.PHONY: all clean clean_exe tnmap_client tnmap_serveur tnmap_all

tnmap_client: TNmap_client

tnmap_serveur: TNmap_serveur

tnmap_all: TNmap_client TNmap_serveur

TNmap_serveur: TNmap_serveur.o scan_horizontal.o scanport.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

TNmap_client: TNmap_client.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

TNmap_serveur.o: TNmap_serveur.c
	$(CC) -c $(CFLAGS) $^

TNmap_client.o: TNmap_client.c
	$(CC) -c $(CFLAGS) $^

scan_horizontal.o: scan_horizontal.c
	$(CC) -c $(CFLAGS) $^

scanport.o: scanport.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f *.o

clean_exe:
	rm -f TNmap_serveur TNmap_client scanport gestionnaire scan
