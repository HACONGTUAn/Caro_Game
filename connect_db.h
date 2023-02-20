// mo file danh sach nguoi choi 
#include "queue.h"

void OpenFile(){
  FILE * fptr;
  char id[20];
  char username[20];
  char password[20];
  char nickname[20];
  int numberOfGame;
  int numberOfDraw;
  int numberOfWin;
  int numberOfLoss;
  int rank;
  if((fptr = fopen("data/account.txt","r")) == NULL){
    printf("khong mo duoc file\n");
  }
  while(fscanf(fptr,"%s %s %s %s %d %d %d %d %d",id,username,password,nickname,&numberOfGame,&numberOfDraw,&numberOfWin,&numberOfLoss,&rank) != EOF){
    insertFirst(id,username,password,nickname,numberOfGame,numberOfDraw,numberOfWin,numberOfLoss,rank);
  }
  fclose(fptr);
}
// ghi danh sach nguoi choi moi ra file
void WritetoFile(){
  FILE * fptr;
  if((fptr = fopen("data/account.txt","w")) == NULL){
    printf ("khong ghi du lieu lai dc\n");
  }
  struct node *ptr = head;
  while(ptr != NULL){
    fprintf(fptr,"%s %s %s %s %d %d %d %d %d\n",ptr->id,ptr->username,ptr->password,ptr->nickname,ptr->numberOfGame,ptr->numberOfDraw,ptr->numberOfWin,ptr->numberOfLoss,ptr->rank);
    ptr = ptr->next;
  }
  fclose(fptr);
}

// mo danh sach nguoi ban
void OpenFriend(){
  FILE * ptr;
  if((ptr = fopen("data/listFriend.txt","r")) == NULL){
    printf("khong mo dc listFriend");
  }
}
// ghi vao danh sach listFriend
void tach_chuoi_friend(FILE *p,queue *q){
char ch[128];
 char x[20];
 int pt;
  fgets(ch,128,p);
  // printf("%s\n",ch);
  char * token = strtok(ch," ");
  while(token != NULL){
    // printf("%s-",token);
    // pt = atoi(token);
    enqueue(q,token);
    if(token = strtok(NULL," "));
  }
}
