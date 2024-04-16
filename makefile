CLN = clang -Wall -Wextra -pedantic -O0 -g3 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls
CLNO = clang -c -Wall -Wextra -pedantic -O0 -g3 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls

serveurTCP : serveurTCP.o
	@${CLN} serveurTCP.o -o serveurTCP -lpthread

serveurTCP.o : serveurTCP.c
	@${CLNO} serveurTCP.c

clientTCP : clientTCP.o
	@${CLN} clientTCP.o -o clientTCP -lpthread

clientTCP.o : clientTCP.c
	@${CLNO} clientTCP.c

gestionnaire : gestionnaire.o scan_horizontal.o
	@${CLN} gestionnaire.o scan_horizontal.o -o gestionnaire -lpthread

gestionnaire.o : gestionnaire.c
	@${CLNO} gestionnaire.c

scan_horizontal.o : scan_horizontal.c
	@${CLNO} scan_horizontal.c

scanport : scanport.o
	@${CLN} scanport.o -o scanport

scanport.o : scanport.c
	@${CLNO} scanport.c

all : serveurTCP clientTCP gestionnaire

TCP : serveurTCP clientTCP

clean :
	@rm -f *.o

clean_exe :
	@rm -f serveurTCP clientTCP gestionnaire scanport scan
