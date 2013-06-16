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


int main()
{
  node_t *user_list;
  user_t user;
  user_t* user_ptr;
  user_list = NULL;
     int sd,from_len,new_socket;
     int result,EOT=0;
     request_msn_t req;
     struct sockaddr_in from,sockname;
     char buffer[82];

 
     if((sd=socket(AF_INET,SOCK_STREAM,0))==-1){
       perror("Servidor:Socket");
       exit(1);
     }
      sockname.sin_family=AF_INET;
      sockname.sin_addr.s_addr=INADDR_ANY;
      sockname.sin_port=htons(5001);
                       /*     ^^^^   Sustituir por el puerto correspondiente */
      if(bind(sd,(struct sockaddr*) &sockname,sizeof(sockname))==-1){
	perror("Servidor:Bind");
	exit(1);
      }
      if(listen(sd,1)==-1){
	perror("Servidor1:Listen");
	exit(1);
      }
      from_len=sizeof(from);
      if((new_socket=accept(sd,(struct sockaddr*) &from,&from_len))==-1){
        perror("Servidor:Accept");
	exit(1);
      }
      do{
	result = recv(new_socket,buffer,80,0);

	if(result==-1){
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
	  
	  switch(req.req_type){
	  case GET_STATE:
	    printf("GET_STATE %s\n",req.req[0]);
	    user_ptr = getUser(req.req[0],user_list);
	    if(user_ptr!=NULL){
	      sprintf(buffer,"%i %s %i",
		      STATE,user_ptr->name,user_ptr->state);
	    }else{
	      sprintf(buffer,"%i",ERROR);
	    }
	    if(send(new_socket,buffer,80,0)==-1){
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
	    if(send(new_socket,buffer,80,0)==-1){
	      printf("Servidor:Send\n");
	      exit(1);
	    }
	    
	    break;
	  case GET_LOCATION:
	    printf("GET_LOCATION %s\n",req.req[0]);
	    user_ptr = getUser(req.req[0],user_list);
	    if(user_ptr!=NULL){
	      sprintf(buffer,"%i %s %s",
		      LOCATION,user_ptr->name,user_ptr->location);
	    }else{
	      sprintf(buffer,"%i",ERROR);
	    }
	    if(send(new_socket,buffer,80,0)==-1){
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
      }while(!EOT);
}
