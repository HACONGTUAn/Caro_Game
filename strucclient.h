#define MAX 100

int ALL_CLIENT[MAX];

struct User{
  char id[20];
  char username[20];
  char password[20];
  char nickname[20];
  int file_fd;
  int chat_fd;
  int numberOfGame;
  int numberOfDraw;
  int numberOfWin;
  int numberOfLoss;
  int rank;
  int port;
  char ip[INET_ADDRSTRLEN];
  int onl;
  char flag[20];
};
