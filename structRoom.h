struct Room{
  char Room_id[20];
  int client1;
  int client2;
  char Pass[50];
  int max;//so nguoi toi da trong 1 phong 
};
void setUpRoom(struct Room room[]){
  int i;
  for( i = 0; i < 100 ; ++i){
    strcpy(room[i].Room_id,"0");
  }
}
