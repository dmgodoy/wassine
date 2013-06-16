#include <string.h>
#include <stdlib.h>
#define MAX_NUM_PARAMS_REQUEST 10

typedef enum{
  AUSENTE,
  PRESENTE,
}state_t;

typedef enum{
  GET_STATE,
  POST_NAME_LOCATION_STATE,
  GET_LOCATION,
  EXIT,
}request_type_t;

typedef enum{
  OK,
  ERROR,
  STATE,
  LOCATION,
}response_t;



typedef struct{
  state_t state;
  char name[50];
  char location[15];
}user_t;

struct node_type{
  user_t user;
  struct node_type* next;
  struct node_type* previous;
};// node_t;
typedef struct node_type node_t;

typedef struct{
  char* req[MAX_NUM_PARAMS_REQUEST+1];
  int num_params;
  request_type_t req_type;
}request_msn_t;



int split_request(char* req_buff, request_msn_t* req);
void usercpy(user_t *dest,user_t* src);
void freeRequest(request_msn_t* req);
node_t* addUser(user_t* user,node_t* user_list);//SEGFAULT en usercpy
void showUser(user_t* user);
char* state2str();

int split_request(char* req_buff, request_msn_t* req){
  
  char* str =req_buff;
  char * pch;
  char *saveptr;
  int index = 0;
  req->num_params = 0;
  //printf ("Splitting string \"%s\" into tokens:\n",str);
  pch = strtok_r (str," ,-",&saveptr);
  if(pch == NULL)
    return 0;
  req->req_type = atoi(pch);
  pch = strtok_r (NULL," ,-",&saveptr);

  
  while (pch != NULL)
    {
      req->req[index] = strdup(pch);
      req->num_params++;
      index++;
      //printf ("%s\n",pch);
      pch = strtok_r (NULL, " ,-",&saveptr);
    }

  return 1;

}

void freeRequest(request_msn_t* req){
  int i;
  for(i = 0; i < req->num_params; i++){
    free(req->req[i]);
  }
}

node_t* addUser(user_t* user,node_t* user_list){

  node_t* actual_node = user_list;
  if(actual_node==NULL){
    actual_node = malloc(sizeof(node_t));
    usercpy(&actual_node->user,user);
    actual_node->next=NULL;
    actual_node->previous = NULL;
    return actual_node;

  }
  for(actual_node = user_list; actual_node!= NULL ;
      actual_node = actual_node->next ){
    if(strcmp(actual_node->user.name,user->name)==0){
      usercpy(&actual_node->user,user);
      return user_list;
    }
    if(actual_node->next == NULL){
      actual_node->next = malloc(sizeof(node_t));
      usercpy(&actual_node->next->user,user); 
      actual_node->next->next=NULL;
      actual_node->next->previous = actual_node->next;
      return user_list;
    }
  }
  
}

void usercpy(user_t *dest,user_t* src){

  dest->state = src->state;
  strcpy(dest->name,src->name);
  strcpy(dest->location,src->location);

}

void showUser(user_t* user){
  printf("Name: %s\nLocation: %s\nState: %s\n\n",user->name,
	 user->location,state2str(user->state));
}

char* state2str(state_t state){

  if(state==AUSENTE)
    return "ausente";
  else return "presente";

}

node_t* removeUser(char* user_name,node_t* list){
  node_t* actual = list;
  node_t* tmp ;
  while(actual!=NULL){
    if(strcmp(actual->user.name,user_name)==0){
      actual->previous->next=actual->next;
      tmp = actual->next;
      free(actual);
      actual = tmp;
    }
  }
  return NULL;
}
user_t* getUser(char* user_name,node_t* list){
  node_t* actual = list;

  while(actual!=NULL){
    if(strcmp(actual->user.name,user_name)==0){
      return &actual->user;
    }
  }
  return NULL;
}


void showUsers(node_t* list){
  node_t* actual = list;
  while(actual!=NULL){
    showUser(&actual->user);
    actual=actual->next;

  }

}



void showMenu(){

  printf("%i - Get state from 'user'\n",GET_STATE);
  printf("%i - Post name,location and state\n",POST_NAME_LOCATION_STATE);
  printf("%i - Get location 'user'\n",GET_LOCATION);
  printf("%i - Close conection\n\n",EXIT);

}
