#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include "userMSN.h"


void main(int argc, char* argv[])
{
    int sd,EOT=0,result;
    struct sockaddr_in sockname;
    request_msn_t req;
    char* ip_server;
    char buffer[82];
    char secret[256];
    int challenge,i;
    unsigned char out[16];
    if(argc > 2) {
        printf("Numero de argumentos incorrecto:\n\
Se esperaba %s [ip_servidor]",argv[0]);
        exit(1);
    } else if(argc == 1) {
        ip_server = "127.0.0.1";
    } else if(argc == 2) {
        ip_server = argv[1];
    }
    if((sd=socket(AF_INET,SOCK_STREAM,0))==-1)
        perror("Cliente:Socket"),exit(1);
    sockname.sin_family=AF_INET;
    sockname.sin_addr.s_addr=inet_addr(ip_server);
    /*  ^^^^^^^^^^^^   Direccion server  */
    sockname.sin_port=htons(5001);
    /*   ^^^^  Sustituir por el puerto correspondiente */
    if(connect(sd,(struct sockaddr*) &sockname,sizeof(sockname))==-1) {
        perror("Cliente:Connect");
        exit(1);
    }

    do {
        char user_name[256];
        char location[25];
        unsigned char out[16];
        int state;
        puts("Seleccione accion:");
        showMenu();
        int opt;
        scanf("%i",&opt);
        switch(opt) {
        case GET_STATE:
            scanf("%s",user_name);
            sprintf(buffer,"%i %s",GET_STATE,user_name);
            if(send(sd,buffer,80,0)==-1) {
                perror("Cliente:Send");
                exit(1);
            }
            //Esperar respuesta.
            //printf("Espera respuesta\n");
            result = recv(sd,buffer,80,0);
            //printf("%s\n",buffer);
            if(result==-1) {
                perror("Cliente:Recv");
                exit(1);
            }
            split_request(buffer,&req);
            if(req.req_type==STATE)
                printf("%s STATE %s\n",req.req[0],state2str(atoi(req.req[1])));
            else if(req.req_type==ERROR)
                printf("No existe el usuario %s.\n",user_name);
            //
            break;
        case POST_NAME_LOCATION_STATE:
            //printf("Introduce nombre\n")
            scanf("%s",user_name);
            scanf("%s",location);
            scanf("%i",&state);
            scanf("%s",secret);
            //Primero nos autenticamos
            sprintf(buffer,"%i %s",AUTH,user_name);
            if(send(sd,buffer,80,0)==-1) {
                perror("Cliente:Send");
                exit(1);
            }
            result = recv(sd,buffer,80,0);
            if(result==-1) {
                perror("Cliente:Recv");
                exit(1);
            }
            split_request(buffer,&req);
            if(req.req_type==CHALLENGE) {
                printf("CHALL (RETO) Recibido %i\n",atoi(req.req[0]));
                //generar md5 user_name:reto:secreto
                sprintf(buffer,"%s%i%s",user_name,atoi(req.req[0]),secret);
                printf("MD5 %s : ",buffer);
                getMD5(buffer,strlen(buffer),out);
                for (i=0; i<16; i++) {
                    printf("%02x", out[i]);
                }
                printf("\n");
                //Enviamos datos update
                sprintf(buffer,"%i %s %s %i",POST_NAME_LOCATION_STATE,
                        user_name,location,state);

                if(send(sd,buffer,80,0)==-1) {
                    perror("Cliente:Send");
                    exit(1);
                }
                //Enviamos md5
                if(send(sd,out,16,0)==-1) {
                    perror("Cliente:Send");
                    exit(1);
                }
                result = recv(sd,buffer,80,0);
                if(result==-1) {
                    perror("Cliente:Recv");
                    exit(1);
                }
                split_request(buffer,&req);
                if(req.req_type==OK) {
                    printf("Datos de usuario actualizados.\n");
                } else	if(req.req_type==ERROR) {
                    printf("El codigo MD5 no es correcto.\nDatos no actualizados\n");

                }


                //Esperamos respuesta
            } else if(req.req_type==ERROR) {

                printf("El usuario %s no existe.\n",user_name);

            }
            //Hemos sido autenticados
            break;
        case GET_LOCATION:
            scanf("%s",user_name);
            sprintf(buffer,"%i %s",GET_LOCATION,user_name);
            if(send(sd,buffer,80,0)==-1) {
                perror("Cliente:Send");
                exit(1);
            }

            //Esperar respuesta.
            printf("Espera respuesta\n");
            result = recv(sd,buffer,80,0);
            printf("%s\n",buffer);
            if(result==-1) {
                perror("Cliente:Recv");
                exit(1);
            }
            split_request(buffer,&req);
            if(req.req_type==LOCATION)
                printf("%s LOCATION %s\n",req.req[0],req.req[1]);
            else if(req.req_type==ERROR)
                printf("No existe el usuario %s.\n",user_name);
            //
            break;
        case EXIT:
            sprintf(buffer, "%i", EXIT);
            if(send(sd,buffer,80,0)==-1) {
                perror("Cliente:Send");
                exit(1);
            }
            shutdown(sd,1);//Dejar de 1-transmitir
            EOT=1;
            puts("Fin de la transmision.");
        }

    } while(!EOT);
}
