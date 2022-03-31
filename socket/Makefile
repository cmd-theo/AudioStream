all: audioserver audioclient

audioserver: audioserver.o audio.o ../include/structures.h
	gcc -o audioserver audioserver.o audio.o 

audioserver.o: audioserver.c
	gcc -Wall -g -I ../include -c audioserver.c

audioclient: audioclient.o audio.o ../include/structures.h
	gcc -o audioclient audioclient.o audio.o 

audioclient.o: audioclient.c
	gcc -Wall -g -I ../include -c audioclient.c

audio.o: 
	gcc -Wall -g -I ../include -c audio.c
