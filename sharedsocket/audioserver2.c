#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/audio.h"
#include "../include/structures.h"
#include <stdlib.h>

int treat_request(int socketServer, char* filename, struct sockaddr_in from, socklen_t fromlen){
    printf("Transfert du fichier audio : %s", filename);


    int sample_rate;
    int sample_size;
    int channels;
    if(access(filename, R_OK|W_OK)!=0){
                        
        perror("Filename does not exist !");
        return -1;
    }

                    //Récupération du header 
    int statut_readinit = aud_readinit(filename, &sample_rate, &sample_size, &channels); //lecture de l'en-tête du fichier
    if(statut_readinit < 0){
        perror("Fatal error ! ReadInit");
    }


    struct bufferHeader buf;
    buf.sp_rate = sample_rate;
    buf.sp_size = sample_size;
    buf.channels = channels;

    int sendHeader = sendto(socketServer, &buf, sizeof(buf), 0, 
                                        (struct sockaddr*) &from, sizeof(struct sockaddr_in));
    if(sendHeader<0){
        perror("Fatal Error ! Header");
    }

                    //Envoi des paquets du fichier filename vers le client

    char buffer[sample_size]; //buffer de lecture et d'écriture des échantillons de la taille des échantillons du fichier 
    int statut_read = sample_size;

    socklen_t receiveAck;
    while(statut_read == sample_size){
        char ack[5];
        receiveAck = recvfrom(socketServer, &ack, sizeof(ack), 0, (struct sockaddr*) &from, &fromlen);
        if(receiveAck < 0 ){
            perror("Fatal error ! Receiving");
        }
        statut_read = read(statut_readinit, buffer, sample_size); //lecture des échantillons 
        if(statut_read<0){
            perror("Fatal error ! Read");
        }
        int sendData = sendto(socketServer, &buffer, sizeof(buffer), 0, 
                                        (struct sockaddr*) &from, sizeof(struct sockaddr_in));
        if(sendData<0){
            perror("Fatal Error ! Header");
        }
    }
    char bufferEnd[3] = "EOF";
    int sendEnd = sendto(socketServer, &bufferEnd, sizeof(bufferEnd), 0, 
                                        (struct sockaddr*) &from, sizeof(struct sockaddr_in));
                       
}


int main(){

    char filename[50];

    struct sockaddr_in netadress;
    netadress.sin_family = AF_INET;
    netadress.sin_port = htons(1234);
    netadress.sin_addr.s_addr = htonl(INADDR_ANY);

    int socketServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketServer < 0) {
        perror("Fatal error ! Bad Creation");
    }

    //on définit un numéro de port au serveur (ici 1234)
    int bindSocket = bind(socketServer, (struct sockaddr *)&netadress, sizeof(struct sockaddr_in));
    if(bindSocket < 0){
        perror("Fatal error ! Port Declaration");
    }
    
    socklen_t receive , fromlen;
    fromlen = sizeof(struct sockaddr_in);
    struct sockaddr_in from;
    int pid;
    while(1){
                   
                    receive = recvfrom(socketServer, filename, sizeof(filename), 0, (struct sockaddr*) &from, &fromlen);
                    if(receive < 0 ){
                        perror("Fatal error ! Receiving");
                    }
                        printf("Received %d bytes from host %s port %d: %s", receive,
                        inet_ntoa(from.sin_addr), ntohs(from.sin_port), filename);
                    printf(filename +" 1" );
                    pid = fork();
                    printf(filename +" 2" );
                    if(pid==0){
                    printf(filename +" 3" );
                        treat_request(socketServer, filename, from, fromlen);
                        printf(filename +" 4" );
                        exit(0);
                    }
                    
                    //Transfert fichier audio au client 

                    
                    }
                    close(socketServer);
                










                    
    close(socketServer);
    return 0;


}