all: audioserver audioclient

audioserver: audioserver.o audio.o ../include/structures.h
	gcc -o audioserver audioserver2.o audio.o 

audioserver.o: audioserver2.c
	gcc -Wall -g -I ../include -c audioserver2.c

audioclient: audioclient2.o audio.o ../include/structures.h
	gcc -o audioclient audioclient2.o audio.o 

audioclient2.o: audioclient2.c
	gcc -Wall -g -I ../include -c audioclient2.c

audio.o: 
	gcc -Wall -g -I ../include -c audio.c
