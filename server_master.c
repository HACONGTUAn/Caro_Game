#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT 5500   /* Port that will be opened */ 
#define BACKLOG 20   /* Number of allowed connections */
#define MAX_BUFFER_SIZE 1024
#define TIMEOUT 20
#define SECRET_KEY 0
#define MAX_NAME_SIZE 20
#define NO_OF_CLIENTS 10
#include "Linklist.h"
#include "connect_db.h"
#include "strucclient.h"
#include "messega_server.h"
#include "structRoom.h"


static int listen_fd = 0;
static int count = 0;
char thong_diep[20] = {0};
char check_name[20] = {0};
char check_pass[20] = {0};
int ID_ROOM = 100;

struct server_data{
  int so_client;
  struct User list_client[NO_OF_CLIENTS];
};

struct server_data server;
struct Room room[100];

int server_create_socket(int * listen_fd);
int server_build_fdset(int listen, fd_set *readfds,fd_set *writefds, fd_set *exceptfd);
int server_select(int max_fd,int listen_fd, fd_set * readfds,fd_set * writefds);
int server_new_client_handle(int listen_fd,int * new_socket_fd);
void server_deleta_client(int client_socket);
void server_add_client(struct sockaddr_in client_addr,int new_socket_fd,struct node *inf_client);
int server_recv_from_client(int new_socket_fd,char *buffer);
int process_recv(int client_socket,char * buffer);
int find_the_client_index_list(int client_socket);
int server_send_to_client(int client_socket, char * mess);
void cleanup(void);
void splitMessage(char buffer[]);
int check_room_id(char*id_phong);
int check_client_in_room(int client_fd);
int check_file_fd_from_id(char* id);
int main(){  
 OpenFile();
 printList();
 setUpRoom(room);
 int socket = 0;
 fd_set readfds;
 fd_set writefds;
 fd_set exceptfd;
 int max_fd = 0;

 memset(&server,0,sizeof(struct server_data));
  printf("SERVER Started \n");

  if(server_create_socket(&listen_fd) != 0){
    perror("ERROR : create soket false");
    exit(0);
  }

  max_fd = listen_fd;
  while(1){
  max_fd = server_build_fdset(listen_fd,&readfds,&writefds,&exceptfd);
  server_select(max_fd, listen_fd, &readfds,&writefds);
  }
  cleanup();
  printf("bye from server\n");
  return 0;
}

int server_create_socket(int * listen_fd){
  struct sockaddr_in server_add;

  if((*listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
    perror("ERROR : socket created fales");
    return -1;
  }

  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(PORT);
  server_add.sin_addr.s_addr = INADDR_ANY;

  if(0 != bind(*listen_fd,(struct sockaddr*)&server_add,sizeof(struct sockaddr))){
    perror("ERROR : creat bind false");
    return -1;
  }

  
  if(0 != listen(*listen_fd,BACKLOG)){
    perror("ERROR : creat listen false");
    return -1;
  }
  return 0;
}

int server_build_fdset(int listen, fd_set *readfds,fd_set *writefds, fd_set *exceptfd){
  int max_fd = listen;

  FD_ZERO(readfds);
  FD_SET(listen,readfds);
  FD_SET(STDIN_FILENO,readfds);
  fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);
  
  for(int i = 0 ; i < server.so_client ; i++){
    FD_SET(server.list_client[i].file_fd,readfds);
    max_fd++;
  }
  return max_fd;
}

int server_select(int max_fd,int listen_fd, fd_set * readfds,fd_set * writefds){
  int new_socket_fd = 0;
  char recv_mess[MAX_BUFFER_SIZE];
  char send_buff[MAX_BUFFER_SIZE];
  int key;
  int i;
  struct timeval tv;
  
  memset(recv_mess,0,sizeof(recv_mess));
  memset(send_buff,0,sizeof(send_buff));

  int acction = select(max_fd+1,readfds,writefds,NULL,NULL);

  if(acction == -1 && acction == 0){
    perror("ERROR : SELECT");
    exit(0);
  }
  // lang nghe tu o cam 
  if(FD_ISSET(listen_fd,readfds)){
    server_new_client_handle(listen_fd,&new_socket_fd);
    printf("new socket cread = %d \n",new_socket_fd);
  }
  // lang nghe tu cac client
  for(int i = 0 ; i < server.so_client ; i++){
    if(FD_ISSET(server.list_client[i].file_fd,readfds)){
      printf("***************************************\n");
      printf("%d\n",server.list_client[i].file_fd);
      server_recv_from_client(server.list_client[i].file_fd,recv_mess);
    }
  }
  return 0;
}
int server_new_client_handle(int listen_fd,int * new_socket_fd){
  struct sockaddr_in client_addr;
  int len = sizeof(struct sockaddr);
  char buffer_check[MAX_BUFFER_SIZE];
  char send_check[2048];
  int write_byte;
  int len_check;
  int size_check_mess;
  struct node* check;
  bzero(&client_addr,sizeof(client_addr));
  if((*new_socket_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&len)) <0){
    perror("ERROR : accept faile");
    exit(1);
  }
  ALL_CLIENT[count] = *new_socket_fd;
  // nhan name va pass tu phia client
  if(size_check_mess = recv(*new_socket_fd,buffer_check,MAX_BUFFER_SIZE,0) > 0){
    splitMessage(buffer_check);
    if(strcmp(thong_diep,"client-verify") == 0){
      printf("client gui thong diep muon dang nhap\n");
      //check name xem co trong linklist khong
      check = find(check_name);
      if(check != NULL){
	//check den mat khau
	printf("tai khoang ton tai\n");
	if(strcmp(check -> password ,check_pass) == 0){
	  printf("mat khau dung\n");
	  //send cho client thong diep dang nhap thanh cong

	  sprintf(send_check,"%s,%s,%s,%s,%s,%d,%d,%d,%d,%d\n",LOGIN,check->id,check->username,check->password,check->nickname,check->numberOfGame,check->numberOfDraw,check->numberOfWin,check->numberOfLoss,check->rank);
	  len_check = strlen(send_check);
	  if(write_byte = send(*new_socket_fd,send_check,len_check,0) < 0){
    perror("ERROR : send failed");
    return -1;    
	  }
	  server_add_client(client_addr,*new_socket_fd,check);
	}else{
	  //send cho client loi
	  strcpy(send_check,LOGIN_FALSE);
	  len_check = strlen(send_check);
	  write_byte = send(*new_socket_fd,send_check,len_check,0);
	}
      }
      else{
	// send cho client loi
	 strcpy(send_check,LOGIN_FALSE);
	 len_check = strlen(send_check);
	 write_byte = send(*new_socket_fd,send_check,len_check,0);
      }
    }
    }else{
    printf("loi nhan thong diep tu client\n");
  }
  
// server_add_client(client_addr,*new_socket_fd);
  count++;
}

void server_add_client(struct sockaddr_in client_addr,int new_socket_fd,struct node* inf_client){
  char ip[INET_ADDRSTRLEN] = {0};
  char buffer[MAX_BUFFER_SIZE] = {0};
 
  int port = ntohs(client_addr.sin_port);
  inet_ntop(AF_INET,&(client_addr.sin_addr),ip,INET_ADDRSTRLEN);
  printf("client info : port : %d | ip : %s \n",port,ip);
 
  // them client vao struct
  strcpy(server.list_client[server.so_client].id,inf_client->id);
  strcpy(server.list_client[server.so_client].username,inf_client->username);
  printf("name client : %s \n ",server.list_client[server.so_client].username);
  server.list_client[server.so_client].port = port;
  strcpy(server.list_client[server.so_client].ip,ip);
  server.list_client[server.so_client].file_fd = new_socket_fd;
  server.list_client[server.so_client].onl = 1;
  server.so_client++;
}

void server_delete_client(int socket_fd_del){
  int i = 0;
  int index = 0;
  for(i = 0 ; i < NO_OF_CLIENTS ; i++){
    if(server.list_client[i].file_fd == socket_fd_del){
      for(index = i ; index < NO_OF_CLIENTS; index ++){
	server.list_client[index] =  server.list_client[index+1];
      }
    }
  }
  server.so_client --;
  printf("socket delete = [%d]\n",socket_fd_del);
  close(socket_fd_del);
}

int server_recv_from_client(int new_socket_fd,char *buffer){
  int read_byte = 0;

  memset(buffer,0,strlen(buffer));

  if((read_byte = recv(new_socket_fd,buffer,MAX_BUFFER_SIZE,0)) > 0){
    process_recv(new_socket_fd,buffer);
  }
  else if(read_byte == 0){
    printf("client disconnect\n");
    server_delete_client(new_socket_fd);
  }
  else{
    printf("ERROR:recv failse\n");
  }
  return 0;
}

int process_recv(int client_socket,char * buffer){
  char chat_c[MAX_BUFFER_SIZE];
  char buffer_send[MAX_BUFFER_SIZE] = {0};
  int index_sender = 0;
  int index_recv = 0;
  int index_sender2;
  int len = 0;
  int i;
  int count1 = 1;
  int check_room = 0;
  int check_fd;
  char id_room[50];
  int win_of_lose;
  struct node* check;
  index_sender = find_the_client_index_list(client_socket);
  
  // gui chat cho toan bo client
  if(strncmp(buffer,"chat-server",11) == 0){
     splitMessage(buffer);
      sprintf(buffer_send,"chat-server,%s>>%s\n",server.list_client[index_sender].username,check_name);
    for( i = 0; i < server.so_client ; i++){
      if(server.list_client[index_sender].file_fd != ALL_CLIENT[i])
	server_send_to_client(ALL_CLIENT[i],buffer_send);
    }
    goto out;
  }
  // tao phong cho client 
  if(strncmp(buffer,"create-room",11) == 0){
    splitMessage(buffer);
    sprintf(id_room,"%d",ID_ROOM);
    // ghi du lieu cho Room co id
    if(strcmp(check_name,"0") != 0){
      
    strcpy(room[count1].Room_id,id_room);
    room[count1].client1 = server.list_client[index_sender].file_fd;
    strcpy(room[count1].Pass,check_pass);
    
    sprintf(buffer_send,"%s,%s,%s\n",CREATE_ROM,room[count1].Room_id,room[count1].Pass);
    server_send_to_client(server.list_client[index_sender].file_fd,buffer_send);
    printf("%s",buffer_send);
    ID_ROOM++;
    count1++;
    }
    // room khong mat khau
    if(strcmp(check_name,"0") == 0){
       strcpy(room[count1].Room_id,id_room);
       strcpy(room[count1].Pass,"NO_PASS");
       room[count1].client1 = server.list_client[index_sender].file_fd;
        
    sprintf(buffer_send,"%s,%s,%s\n",CREATE_ROM,room[count1].Room_id,room[count1].Pass);
    server_send_to_client(server.list_client[index_sender].file_fd,buffer_send);
       ID_ROOM++;
       count1++;
    }
    goto out;
  }
  // vao phong
  if(strncmp(buffer,"go-to-room",10)==0){
    
    splitMessage(buffer);
    //tim kiem id phong

  check_room = check_room_id(check_name);
  
    if(check_room == 0){
      printf("phong khong ton tai \n");
    }else{
      if(strcmp(room[check_room].Pass,check_pass) != 0){
	printf("sai mat khau phong \n");
      }else{
	room[check_room].client2 = server.list_client[index_sender].file_fd;
	// gui thong diep cho 2 client trong phong
	if(room[check_room].client1 != 0){
	  
   index_sender = find_the_client_index_list(room[check_room].client2);
   check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
   
	  // gui cho client 1 ve thong tin cua client 2
  sprintf(buffer_send,"go-to-room,%s,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
	  room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	  server.list_client[index_sender].username,
	  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
	  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
	  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
	  server.list_client[index_sender].rank);
   
   server_send_to_client(room[check_room].client1,buffer_send);
   room[check_room].max++;
	}
	
	if(room[check_room].client2 != 0){
	  // gui cho clien 2 ve thong tin cua clien 1
	  
   index_sender = find_the_client_index_list(room[check_room].client1);
    check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
sprintf(buffer_send,"go-to-room,%s,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
	room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	server.list_client[index_sender].username,server.list_client[index_sender].password,
	server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
	server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
	server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);

  server_send_to_client(room[check_room].client2,buffer_send);
  room[check_room].max++;
      }
      }
    }
  
    goto out;
    
  }

  if(strncmp(buffer,"caro",4) == 0){
    splitMessage(buffer);
    if(check_room == 0 ){
      for(i = 1 ; i < 100 ; ++i){
	if(strcmp(room[i].Pass,"NO_PASS_S") == 0){
	  check_room = i;
	}
      }
    }
    // check nguoi gui la ai va gui nguoc lai nguoi con lai
    sprintf(buffer_send,"caro,%s,%s\n",check_name,check_pass);
    if(server.list_client[index_sender].file_fd == room[check_room].client1){
      server_send_to_client(room[check_room].client2,buffer_send);
    }
     if(server.list_client[index_sender].file_fd == room[check_room].client2){
      server_send_to_client(room[check_room].client1,buffer_send);
    }
    goto out;
  }
  
  if(strncmp(buffer,"win",3) == 0){
    sprintf(buffer_send,"new-game\n");
    server_send_to_client(room[check_room].client1,buffer_send);
    server_send_to_client(room[check_room].client2,buffer_send);
    goto out;
  }
  if(strncmp(buffer,"draw-request",12) == 0){
  sprintf(buffer_send,"draw-request\n");
    if(server.list_client[index_sender].file_fd == room[check_room].client1){
      server_send_to_client(room[check_room].client2,buffer_send);
    }
     if(server.list_client[index_sender].file_fd == room[check_room].client2){
      server_send_to_client(room[check_room].client1,buffer_send);
    }
     goto out;
  }

  if(strncmp(buffer,"draw-confirm",12)==0){
    sprintf(buffer_send,"draw-game\n");
     server_send_to_client(room[check_room].client1,buffer_send);
    server_send_to_client(room[check_room].client2,buffer_send);
    goto out;
  }
  if(strncmp(buffer,"draw-refuse",11)==0){
    sprintf(buffer_send,"draw-refuse\n");
     if(server.list_client[index_sender].file_fd == room[check_room].client1){
      server_send_to_client(room[check_room].client2,buffer_send);
    }
     if(server.list_client[index_sender].file_fd == room[check_room].client2){
      server_send_to_client(room[check_room].client1,buffer_send);
    }
    goto out;
  }

  if(strncmp(buffer,"view-room-list",14) == 0){
    for(i = 1 ; i < 100 ; ++i){
      if(strcmp(room[i].Room_id,"0") != 0){
      sprintf(buffer_send,"room-list,%s,%s\n",room[i].Room_id,room[i].Pass);
      server_send_to_client(server.list_client[index_sender].file_fd,buffer_send);
      }
    }
    goto out;
  }
  // xu ly vao phong khong dat mat khau trong list room
  if(strncmp(buffer,"join-room",9) == 0){
     splitMessage(buffer);
    check_room = check_room_id(check_name);

    if(check_room == 0){
      printf("phong khong ton tai \n");
    }else{
      if(strcmp(check_pass,"NO_PASS") != 0){
	printf("sai mat khau phong trong listRoom \n");
      }else{
	room[check_room].client2 = server.list_client[index_sender].file_fd;
	// gui thong diep cho 2 client trong phong
	if(room[check_room].client1 != 0){
	  
   index_sender = find_the_client_index_list(room[check_room].client2);
   check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
	  // gui cho client 1 ve thong tin cua client 2
  sprintf(buffer_send,"go-to-room,%s,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
	  room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	  server.list_client[index_sender].username,
	  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
	  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
	  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
	  server.list_client[index_sender].rank);
   
   server_send_to_client(room[check_room].client1,buffer_send);
   room[check_room].max++;
	}
	
	if(room[check_room].client2 != 0){
	  // gui cho clien 2 ve thong tin cua clien 1
	  
   index_sender = find_the_client_index_list(room[check_room].client1);
    check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
sprintf(buffer_send,"go-to-room,%s,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
	room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	server.list_client[index_sender].username,server.list_client[index_sender].password,
	server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
	server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
	server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);

  server_send_to_client(room[check_room].client2,buffer_send);
  room[check_room].max++;
      }
      }
      if( strcmp(room[check_room].Pass,check_pass) != 0){
	printf("sai mat khau phong trong listRoom \n");
      }else{
	room[check_room].client2 = server.list_client[index_sender].file_fd;
	// gui thong diep cho 2 client trong phong
	if(room[check_room].client1 != 0){
	  
   index_sender = find_the_client_index_list(room[check_room].client2);
   check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
	  // gui cho client 1 ve thong tin cua client 2
  sprintf(buffer_send,"go-to-room,%s,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
	  room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	  server.list_client[index_sender].username,
	  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
	  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
	  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
	  server.list_client[index_sender].rank);
   
   server_send_to_client(room[check_room].client1,buffer_send);
   room[check_room].max++;
	}
	
	if(room[check_room].client2 != 0){
	  // gui cho clien 2 ve thong tin cua clien 1
	  
   index_sender = find_the_client_index_list(room[check_room].client1);
    check = find(server.list_client[index_sender].username);
   strcpy(server.list_client[index_sender].nickname,check->nickname);
sprintf(buffer_send,"go-to-room,%s,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
	room[check_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
	server.list_client[index_sender].username,server.list_client[index_sender].password,
	server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
	server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
	server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);

  server_send_to_client(room[check_room].client2,buffer_send);
  room[check_room].max++;
      }
      }
    }
    goto out;
  }

  if(strncmp(buffer,"chat",4)==0){
      splitMessage(buffer);
    // check nguoi gui la ai va gui nguoc lai nguoi con lai
    sprintf(buffer_send,"chat,%s\n",check_name);
    if(server.list_client[index_sender].file_fd == room[check_room].client1){
      server_send_to_client(room[check_room].client2,buffer_send);}
    if(server.list_client[index_sender].file_fd == room[check_room].client2){
      server_send_to_client(room[check_room].client1,buffer_send);
    }
      goto out;
    }
  // chuc nang tim nhanh
  if(strncmp(buffer,"quick-room",10) == 0){
    int flast_room;
      splitMessage(buffer);
      strcpy(server.list_client[index_sender].flag,check_pass);
      printf("tim phong nhanh\n");
	   // kiem tra xem co client nao tim nhanh khong thi mix voi nhau
      /*   
       for( i = 0; i < server.so_client ; i++){
	 if(server.list_client[index_sender].file_fd != ALL_CLIENT[i]){
	   
	   flast_room =  check_client_in_room(ALL_CLIENT[i]);
	   
	   if( strcmp(server.list_client[index_sender].flag,room[flast_room].Pass ) == 0){
	     printf("tim thay doi tuong\n");
	    
	     room[flast_room].client2 = server.list_client[index_sender].file_fd;

	   	if(room[flast_room].client1 != 0){
	  
		  index_sender = find_the_client_index_list(room[flast_room].client2);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
   
		  sprintf(buffer_send,"go-to-room,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[flast_room].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].username,
			  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
			  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
			  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
			  server.list_client[index_sender].rank);
		  
		  server_send_to_client(room[flast_room].client1,buffer_send);
		  room[flast_room].max++;
		}
		
		if(room[flast_room].client2 != 0){
		  
		  index_sender = find_the_client_index_list(room[flast_room].client1);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
		  sprintf(buffer_send,"go-to-room,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[flast_room].Room_id,server.list_client[index_sender].ip,
			  server.list_client[index_sender].username,server.list_client[index_sender].password,
			  server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
			  server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
			  server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);

		  server_send_to_client(room[flast_room].client2,buffer_send);
		  room[flast_room].max++;
		}
		goto out;	
	   }
	 } 
       }
      */
        // kiem tra xem co phong nao NO_PASS con slot khong
       for(i = 0 ; i < 100;++i){
	 if(room[i].max < 2 && strcmp(room[i].Pass,"NO_PASS") == 0){
	   room[i].client2 = server.list_client[index_sender].file_fd;
	   printf("vao phong khong mat khau\n");
	   	if(room[i].client1 != 0){
	  
		  index_sender = find_the_client_index_list(room[i].client2);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
   
		  sprintf(buffer_send,"go-to-room,%s,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[i].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
			  server.list_client[index_sender].username,
			  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
			  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
			  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
			  server.list_client[index_sender].rank);
		  
		  server_send_to_client(room[i].client1,buffer_send);
		  room[i].max++;
		}
		
		if(room[i].client2 != 0){
		  // gui cho clien 2 ve thong tin cua clien 1
		  
		  index_sender = find_the_client_index_list(room[i].client1);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
		  sprintf(buffer_send,"go-to-room,%s,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[i].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
			  server.list_client[index_sender].username,server.list_client[index_sender].password,
			  server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
			  server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
			  server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);

		  server_send_to_client(room[i].client2,buffer_send);
		  room[i].max++;
		}
		goto out;
	 }
       }
       // tao 1 phong cho neu k co clien nao tim nhanh
       /*
       sprintf(id_room,"%d",ID_ROOM);
       strcpy(room[count1].Room_id,id_room);
       room[count1].client1 = server.list_client[index_sender].file_fd;
       strcpy(room[count1].Pass,check_pass);
       sprintf(buffer_send,"%s,%s,%s\n",CREATE_ROM,room[count1].Room_id,room[count1].Pass);
       printf("tao phong nhanh\n");
    goto out;
       */
  }
  if(strncmp(buffer,"view-challe-list",16) == 0){
    for(i = 0 ; i < server.so_client ; i++){
      if(server.list_client[i].file_fd != server.list_client[index_sender].file_fd ){
      sprintf(buffer_send,"return-challe-list,%s,%s,1,1\n",server.list_client[i].id,server.list_client[i].username);
      server_send_to_client(server.list_client[index_sender].file_fd,buffer_send);
      }
    }
    goto out;
  }

  if(strncmp(buffer,"check-friend",12) == 0){
    splitMessage(buffer);
    
  }

  if(strncmp(buffer,"duel-request",12) == 0){
     splitMessage(buffer);
     
    check_fd = check_file_fd_from_id(check_name);
    
    sprintf(buffer_send,"duel-notice,%s,%s\n",server.list_client[index_sender].id,server.list_client[index_sender].username);
    
    server_send_to_client(check_fd,buffer_send);
    goto out;
  }

  if(strncmp(buffer,"agree-duel",10) == 0){
     splitMessage(buffer);
     check_fd = check_file_fd_from_id(check_name);
     
     sprintf(id_room,"%d",ID_ROOM);
     
     strcpy(room[count1].Room_id,id_room);
     strcpy(room[count1].Pass,"NO_PASS_S");
     room[count1].client1 = server.list_client[index_sender].file_fd;
     room[count1].client2 = check_fd;
     
     // sprintf(buffer_send,"%s,%s,%s\n",CREATE_ROM,room[count1].Room_id,room[count1].Pass);
     
     // server_send_to_client(server.list_client[index_sender].file_fd,buffer_send);
     
     	if(room[count1].client1 != 0){
	  
		  index_sender = find_the_client_index_list(room[count1].client2);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
		  
		  sprintf(buffer_send,"go-to-room,%s,%s,%s,0,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[i].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
			  server.list_client[index_sender].username,
			  server.list_client[index_sender].password,server.list_client[index_sender].nickname,
			  server.list_client[index_sender].numberOfGame,server.list_client[index_sender].numberOfDraw,
			  server.list_client[index_sender].numberOfWin,server.list_client[index_sender].numberOfLoss,
			  server.list_client[index_sender].rank);
		  
		  server_send_to_client(room[count1].client1,buffer_send);
		  room[count1].max++;
	}
	
	if(room[count1].client2 != 0){
		  // gui cho clien 2 ve thong tin cua clien 1
		  
		  index_sender = find_the_client_index_list(room[count1].client1);
		  check = find(server.list_client[index_sender].username);
		  strcpy(server.list_client[index_sender].nickname,check->nickname);
		  sprintf(buffer_send,"go-to-room,%s,%s,%s,1,%s,%s,%s,%d,%d,%d,%d,%d\n",
			  room[i].Room_id,server.list_client[index_sender].ip,server.list_client[index_sender].id,
			  server.list_client[index_sender].username,server.list_client[index_sender].password,
			  server.list_client[index_sender].nickname,server.list_client[index_sender].numberOfGame,
			  server.list_client[index_sender].numberOfDraw,server.list_client[index_sender].numberOfWin,
			  server.list_client[index_sender].numberOfLoss,server.list_client[index_sender].rank);
		  
		  server_send_to_client(room[count1].client2,buffer_send);
		  room[count1].max++;
		}
     
     ID_ROOM++;
     count1++;
       
    goto out;
  }

  // xu ly win va lose
  if( strncmp(buffer,"win",3) == 0){
    ++server.list_client[index_sender].numberOfGame;
    ++server.list_client[index_sender].numberOfWin;
    server.list_client[index_sender].numberOfDraw = server.list_client[index_sender].numberOfGame - server.list_client[index_sender].numberOfWin - server.list_client[index_sender].numberOfLoss;
    win_of_lose = check_client_in_room(server.list_client[index_sender].file_fd);
    index_sender2 = find_the_client_index_list(room[win_of_lose].client2);
    ++server.list_client[index_sender2].numberOfGame;
    ++server.list_client[index_sender2].numberOfLoss;
    server.list_client[index_sender2].numberOfDraw = server.list_client[index_sender2].numberOfGame - server.list_client[index_sender2].numberOfWin - server.list_client[index_sender2].numberOfLoss;
    WritetoFile();
    goto out; 
  }
  
 out:
  return 0;
}

int find_the_client_index_list(int client_socket){
  int index = 0;
  for(int i = 0 ; i < count ; i++){
    if(ALL_CLIENT[i] == client_socket){
      index = i;
    }
  }
  return index;
}

int server_send_to_client(int client_socket, char * mess){
  int write_byte = 0;
  int len = strlen(mess);
  if((write_byte = send(client_socket,mess,len,0)) > 0 ){
    printf("toan bo client nhan duoc tin tu %s \n",mess);
  }else{
    perror("ERROR : send failed");
    return -1;
  }
  return write_byte;
}

void cleanup(){
  close(listen_fd);
  for(int i = 0 ; i < server.so_client; i++){
    close(server.list_client[i].file_fd);
  }
}


void splitMessage(char buffer[]){
  int len = strlen(buffer);
  char *p;
  char *h;
  int i,j = 0,k;
  p = strtok(buffer,",");
  strcpy(thong_diep,p);
  while(p != NULL){
    p =strtok(NULL,",");
    if(p!=NULL){
      strcpy(check_name,p);
      
      if(p!= NULL){
	p = strtok(NULL,"\n");
	strcpy(check_pass,p);
      }  
    }
    }
}
int check_room_id(char * id_phong){
  int i;
  int index = 0;
  for(i = 1 ; i < 100 ; ++i){
    if(strcmp(room[i].Room_id,id_phong) == 0){
      index = i;
    }
  }
  return index;
}

int check_client_in_room(int client_fd){
  int i;
  int index = 0;
  for(i ; i < 100 ; i++){
    if(room[i].client1 == client_fd){
      index = i;
    }
    if(room[i].client2 == client_fd){
      index = i;
    }
  }
  return index;
}

int check_file_fd_from_id(char* id){
  int i;
  int index = 0;
  for(i = 0 ; i < server.so_client ; i++){
    if(strcmp(server.list_client[i].id,id) == 0){
      index = server.list_client[i].file_fd;
    }
  }
  return index;
}
