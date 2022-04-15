#include <stdio.h>
#include <stdlib.h>

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


//Retourne une structure contenant l'adresse issue de la resolution du nom de domaine passé en paramètre 
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

    char *server_host_name = argv[1]; //adresse IP ou nom de domaine du serveur
    char *file_name = argv[2]; //nom du fichier
    char *filter_param = argv[3]; //filtre appliqué (facultatif)
    char *filter_param_number = argv[4]; //constante à appliquer (facultative, et si vide, mise à 2 pour jouer deux fois plus vite)

    int volumeFilter=0;

    if (file_name == NULL || server_host_name==NULL) { //si il n'y a pas assez d'arguments on ne lance pas le client
        perror("Fatal error ! Not Enough Arguments !");
        return -1;
    }

    int socketClient = socket (AF_INET, SOCK_DGRAM, 0); //création du socket qui servira de client 
    if (socketClient < 0) {
        perror("Fatal error ! Bad Creation");
    }
    
    struct in_addr addr;
    get_adress(server_host_name, &addr); //résolution du nom de domaine passé en paramètre du client 

    struct sockaddr_in dest; //structure contenant les informations permettant de joindre le serveur (port défini sur 1234 ici)
    dest.sin_family = AF_INET;
    dest.sin_port = htons(1234);
    dest.sin_addr.s_addr = addr.s_addr; 

    int send = sendto(socketClient, file_name, strlen(file_name)+1, 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in)); //envoi du nom du fichier a lire 
    if(send < 0){
        perror("Fatal Error ! Sending");
    }

    socklen_t receiveHeader, fromlen;
    fromlen = sizeof(struct sockaddr_in);
    struct sockaddr_in from;
    struct bufferHeader buf; //structure permettant de récupérer le contenu de l'en-tête du fichier wav à lire 
    receiveHeader = recvfrom(socketClient, &buf, sizeof(buf), 0, (struct sockaddr*) &from, &fromlen); //reception des caractéristiques du header 
    if(receiveHeader < 0 ){
       perror("Fatal error ! Receiving");
    }
    if(buf.channels < 1 || buf.channels > 2 ){ //si le serveur renvoi un paquet à un client qui contient des valeurs différentes de 1 et 2 pour le champ "channels" alors on sait que le serveur est en cours de transmission
        perror("Fatal Error ! Server already in use !");
        return -1;
    }
    printf("Receiveed %d bytes from host %s port %d: sample_rate : %d\n sample_size : %d\n channels : %d\n", receiveHeader,
                inet_ntoa(from.sin_addr), ntohs(from.sin_port), buf.sp_rate, buf.sp_size, buf.channels);

    if(filter_param != NULL){
        if(strcmp(filter_param, "speed")==0) { //on applique le filtre "speed" 
            if(filter_param_number==NULL){
                buf.sp_rate = buf.sp_rate * 2; //si l'utilisateur n'a pas mis de constante on mulitplie la fréquence d'échantillonage par 2 par défaut
            }
            else{
                buf.sp_rate = buf.sp_rate * atoi(filter_param_number); //on multiplie la fréquence d'échantillonage par la constante indiquée par l'utilisateur
            }
        }
        else if(strcmp(filter_param, "mono")==0){ //on applique le filtre "mono" qui force l'écriture des echantillons en mono 
            printf("File forced to mono\n");
            buf.channels = 1;
        }

        else if(strcmp(filter_param, "volume")==0){ //on applique le filtre "volume" qui augmente le volume du fichier demandé 
            volumeFilter = 1;
        }

        else { //le filtre est inconnu, on termine la communication avec le serveur
            perror("Fatal error ! Filtre inconnu !");
            return -1;
        }
    }


    int statut_write_init = aud_writeinit(buf.sp_rate, buf.sp_size, buf.channels); //declaration des paramètres du fichier demandé au serveur 
    if(statut_write_init < 0){
        perror("Fatal error ! Write");
    }

    char buffer[buf.sp_size]; //buffer de lecture et d'écriture des échantillons de la taille des échantillons du fichier 
    const char *ack = "Ack";
    socklen_t receiveData;
    int sendAck = sendto(socketClient, &ack, sizeof(ack), 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in)); //envoi du premier acquittement servant à informer le serveur qu'il peut commencer à transmettre les échantillons du fichier audio
    if(sendAck < 0){
        perror("Fatal Error ! Sending");
    }
    do { //protocole de transmission des échantillons côté client 
        receiveData = recvfrom(socketClient, &buffer, sizeof(buffer), 0, (struct sockaddr*) &from, &fromlen); //réception d'un échantillon

        if(volumeFilter == 1) { //l'utilisateur a demandé l'utilisation du filtre "volume" 
            for(int i=0; i<buf.sp_size; i++){
                if(buf.sp_size == 8) {  //le fichier est en mono 
                    int8_t tmp = buffer[i]*4; //on multiplie les échantillions par une constante (ici 4)
                    buffer[i] = tmp;
                }
                else if (buf.sp_size == 16){ //le fichier est en stéréo
                    int16_t tmp = buffer[i]*4;
                    buffer[i] = tmp;
                }
            }
        }
        
        if(receiveData < 0 ){
            perror("Fatal error ! Receiving");
        }

        int statut_write = write(statut_write_init, buffer, buf.sp_size); //écriture de l'échantillon pour être joué sur le haut-parleur 
        if(statut_write<0){
            perror("Fatal error ! Write");
        }

        sendAck = sendto(socketClient, &ack, sizeof(ack), 0, 
                           (struct sockaddr*) &dest, sizeof(struct sockaddr_in)); //envoi de l'acquittement informant le serveur qu'il peut transmettre le prochain échantillon
        if(sendAck < 0){
            perror("Fatal Error ! Sending");
        }

        if(strstr( buffer,"EOF" )!=NULL){ //arrêt de la transmssion si le serveur a transmis "EOF"
            printf("\n FIN DE TRANSMISSION \n");
            break;
        }
        
    }
    while(receiveData == buf.sp_size);
    


    close(socketClient);
    return 0;
    





}