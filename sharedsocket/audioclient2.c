#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include "../include/audio.h"
#include "../include/structures.h"



main(int argc, char **argv)
{
int sockfd,n,i;
char recu[50];
struct sockaddr_in servaddr;
 
if (argc!=3)
        {
        printf("Utilisation : client AdresseIP Port\n");
        exit(-1);
        }
 
bzero(&servaddr,sizeof(servaddr));
servaddr.sin_family=AF_INET;
servaddr.sin_port=htons(atoi(argv[2]));
inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
//transforme l'adresse IP X.X.X.X passÃ©e en parametre en adresse comprehensible
//par le systeme (32 bits)
 
if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
{
        printf("Erreur de socket\n");
        exit(-1);
}
 
if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
{       
        printf("erreur de connexion");
}
// le sleep n 
sleep(5);
while((n=read(sockfd,recu,50))>0)//quand le serveur clot la connexion, n=0
{
                char reponse[30]="\nBonjour serveur\n";
                write(sockfd,reponse,sizeof(reponse));
                recu[n]='\0';
                fputs(recu,stdout);
}
}









/*
int get_adress(const char *name, struct in_addr *addr){
    struct hostent *resolv;

    resolv = gethostbyname(name);
    if(resolv==NULL){
        printf("Adresse non resolue !");
        return -1;
    }
    else {
        addr = (struct in_addr*) resolv->h_addr_list[0];
        printf("L'adresse IP est : %s\n", inet_ntoa(*addr));
        return 0;
    }
}

int main(int args, char** argv){

    char *server_host_name = argv[1];
    char *file_name = argv[2];

    if (file_name == NULL || server_host_name==NULL) {
        perror("Fatal error ! Not Enough Arguments !");
        return -1;
    }

    int socketClient = socket (AF_INET, SOCK_DGRAM, 0);
    if (socketClient < 0) {
        perror("Fatal error ! Bad Creation");
    }
    
    struct in_addr addr;
    get_adress(server_host_name, &addr);

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(1234);
    dest.sin_addr.s_addr = addr.s_addr; 

    int send = sendto(socketClient, file_name, strlen(file_name)+1, 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
    if(send < 0){
        perror("Fatal Error ! Sending");
    }

    //recvfrom() fichier wav en lecture
    socklen_t receiveHeader, fromlen;
    fromlen = sizeof(struct sockaddr_in);
    struct sockaddr_in from;
    struct bufferHeader buf;
    receiveHeader = recvfrom(socketClient, &buf, sizeof(buf), 0, (struct sockaddr*) &from, &fromlen);
    if(receiveHeader < 0 ){
       perror("Fatal error ! Receiving");
    }
   
    printf("Receiveed %d bytes from host %s port %d: sample_rate : %d\n sample_size : %d\n channels : %d\n", receiveHeader,
                inet_ntoa(from.sin_addr), ntohs(from.sin_port), buf.sp_rate, buf.sp_size, buf.channels);

    int statut_write_init = aud_writeinit(buf.sp_rate, buf.sp_size, buf.channels); 
    if(statut_write_init < 0){
        perror("Fatal error ! Write");
    }

    char buffer[buf.sp_size]; //buffer de lecture et d'écriture des échantillons de la taille des échantillons du fichier 
    const char *ack = "Ack";
    socklen_t receiveData;
    int sendAck = sendto(socketClient, &ack, sizeof(ack), 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
    if(sendAck < 0){
        perror("Fatal Error ! Sending");
    }
    do {
        receiveData = recvfrom(socketClient, &buffer, sizeof(buffer), 0, (struct sockaddr*) &from, &fromlen);
        
        if(receiveData < 0 ){
            perror("Fatal error ! Receiving");
        }
        int statut_write = write(statut_write_init, buffer, buf.sp_size); //écriture des échantillons 
        if(statut_write<0){
            perror("Fatal error ! Write");
        }

        sendAck = sendto(socketClient, &ack, sizeof(ack), 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
        if(sendAck < 0){
            perror("Fatal Error ! Sending");
        }

        if( strstr( buffer,"EOF" )!=NULL){
            printf("\n FIN DE TRANSMISSION \n");
            break;
        }
        
    }
    while(receiveData == buf.sp_size);
    


    close(socketClient);
    return 0;
    





}*/