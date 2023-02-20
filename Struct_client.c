#define MAX 100

int ArrListClient[MAX];

struct User{
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
}
