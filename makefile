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

gestionnaire : gestionnaire.o
	@${CLN} gestionnaire.o -o gestionnaire -lpthread

gestionnaire.o : gestionnaire.c
	@${CLNO} gestionnaire.c

scanner_horizontal : scanner_horizontal.o
	@${CLN} scanner_horizontal.o -o scanner_horizontal

scanner_horizontal.o : scanner_horizontal.c
	@${CLNO} scanner_horizontal.c

all : serveurTCP clientTCP gestionnaire scanner_horizontal

TCP : serveurTCP clientTCP

clean :
	@rm -f *.o

clean_exe :
	@rm -f serveurTCP clientTCP gestionnaire scanner_horizontal
