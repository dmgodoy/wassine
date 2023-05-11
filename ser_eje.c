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

//node_t user_list;
#define MAX_NUM_CLIENTS 4

int main()
{
    node_t *user_list;
    user_t user;
    user_t* user_ptr;
    int i,state,ci,pid;
    user_list = NULL;
    char location[15];
    int challenge;
    int sd,from_len,new_socket;
    int result,EOT=0;
    request_msn_t req;
    struct sockaddr_in from,sockname;
    char buffer[82];
    unsigned char out[16];
    unsigned char md5[16];


    user_t user1 = {1,"asce","123.4.5.6","asce_secret"};
    user_list = addUser(&user1,user_list);
    //user_ptr = getUser("asce",user_list);

    user_t user2 = {1,"pepe","123.7.8.9","pepe_secret"};
    user_list = addUser(&user2,user_list);
    //user_ptr = getUser("pepe",user_list);

    user_t user3 = {1,"juan","123.10.11.12","juan_secret"};
    user_list = addUser(&user3,user_list);
    //user_ptr = getUser("juan",user_list);


    showUsers(user_list);
    //getchar();

    if((sd=socket(AF_INET,SOCK_STREAM,0))==-1) {
        perror("Servidor:Socket");
        exit(1);
    }
    sockname.sin_family=AF_INET;
    sockname.sin_addr.s_addr=INADDR_ANY;
    sockname.sin_port=htons(5001);
    /*     ^^^^   Sustituir por el puerto correspondiente */
    if(bind(sd,(struct sockaddr*) &sockname,sizeof(sockname))==-1) {
        perror("Servidor:Bind");
        exit(1);
    }
    if(listen(sd,1)==-1) {
        perror("Servidor1:Listen");
        exit(1);
    }
    from_len=sizeof(from);

    for(ci = 0; ci < MAX_NUM_CLIENTS; ci++) {


        //Se bloquea esperando peticion de client
        if((new_socket=accept(sd,(struct sockaddr*) &from,&from_len))==-1) {
            perror("Servidor:Accept");
            exit(1);
        }

        if ( (pid = fork()) == 0 ) {
            close(sd); /* child closes listening socket */
            do {
                result = recv(new_socket,buffer,80,0);

                if(result==-1) {
                    perror("Servidor:Recv");
                    exit(1);
                }
                /*	if(strcmp(buffer,"EOT")==0){
                  printf("Se ha recibido EOT\n");
                  EOT=1;
                }else if(strcmp(buffer,"EOT")!=0){
                printf("El mensaje recibido fue:\n%s\n",buffer);*/
                printf("%s\n",buffer);
                split_request(buffer,&req);

                switch(req.req_type) {
                case GET_STATE:
                    printf("GET_STATE %s\n",req.req[0]);
                    //printf("Buscamos usuario...\n");
                    user_ptr = getUser(req.req[0],user_list);
                    if(user_ptr!=NULL) {

                        sprintf(buffer,"%i %s %i",
                                STATE,user_ptr->name,user_ptr->state);
                        //printf("Encontrado\n Respuesta a enviar: %s\n",buffer);
                    } else {
                        printf("El usuario no existe\n");
                        sprintf(buffer,"%i",ERROR);
                    }
                    if(send(new_socket,buffer,80,0)==-1) {
                        printf("Servidor:Send\n");
                        exit(1);
                    }
                    break;
                case POST_NAME_LOCATION_STATE:
                    printf("POST_NAME_LOCATION_STATE %s %s %i\n",
                           req.req[0],req.req[1],atoi(req.req[2]));

                    strcpy(user.name,req.req[0]);
                    strcpy(user.location,req.req[1]);
                    user.state = atoi(req.req[2]);
                    user_list = addUser(&user,user_list);
                    showUsers(user_list);
                    sprintf(buffer,"%i",OK);
                    if(send(new_socket,buffer,80,0)==-1) {
                        printf("Servidor:Send\n");
                        exit(1);
                    }

                    break;
                case AUTH:
                    printf("AUTH %s\n",req.req[0]);
                    //Comprobamos si existe el usuario
                    user_ptr = getUser(req.req[0],user_list);
                    if(user_ptr!=NULL) { //Si Existe mandamos reto
                        challenge = random();
                        sprintf(buffer,"%i %i",CHALLENGE,challenge);
                        printf("Enviado reto: %s\n",buffer);
                        if(send(new_socket,buffer,80,0)==-1) {
                            printf("Servidor:Send\n");
                            exit(1);
                        }
                        result = recv(new_socket,buffer,80,0);

                        if(result==-1) {
                            perror("Servidor:Recv");
                            exit(1);
                        }
                        split_request(buffer,&req);
                        //printf("Recibimos %s\n",buffer);
                        strcpy(location,req.req[1]);
                        state = atoi(req.req[2]);
                        //Esperamos md5 name:reto:secreto
                        result = recv(new_socket,out,16,0);
                        for (i=0; i<16; i++) {
                            printf("%02x", out[i]);
                        }
                        printf("\n");
                        //Comparamos md5 recibido con nuestro md5
                        sprintf(buffer,"%s%i%s",user_ptr->name,challenge,user_ptr->secret);
                        printf("OUR MD5 %s : ",buffer);
                        getMD5(buffer,strlen(buffer),md5);
                        for (i=0; i<16; i++) {
                            printf("%02x", md5[i]);
                        }
                        printf("\n");
                        if(memcmp(md5,out,16)==0) {
                            printf("MATCH MD5\n");
                            strcpy(user_ptr->location,location);
                            user_ptr->state = state;
                            sprintf(buffer,"%i",OK);
                            if(send(new_socket,buffer,80,0)==-1) {
                                printf("Servidor:Send\n");
                                exit(1);
                            }
                        } else {
                            printf("MD5 incorrecto.\n");
                            sprintf(buffer,"%i",ERROR);
                            if(send(new_socket,buffer,80,0)==-1) {
                                printf("Servidor:Send\n");
                                exit(1);
                            }

                        }

                    } else { //Si no existe mandamos error
                        printf("No existe el usuario.\n");
                        sprintf(buffer,"%i",ERROR);
                        if(send(new_socket,buffer,80,0)==-1) {
                            printf("Servidor:Send\n");
                            exit(1);
                        }
                    }


                    break;
                case GET_LOCATION:
                    printf("GET_LOCATION %s\n",req.req[0]);
                    user_ptr = getUser(req.req[0],user_list);
                    if(user_ptr!=NULL) {
                        sprintf(buffer,"%i %s %s",
                                LOCATION,user_ptr->name,user_ptr->location);
                    } else {
                        sprintf(buffer,"%i",ERROR);
                    }
                    if(send(new_socket,buffer,80,0)==-1) {
                        printf("Servidor:Send\n");
                        exit(1);
                    }
                    break;
                case EXIT:
                    printf("EXIT\n");
                    printf("Fin de la conexion\n");
                    EOT=1;
                }
                freeRequest(&req);
                //}
            } while(!EOT);

        }//Fin codigo hijo
    }//Fin Server for
}
