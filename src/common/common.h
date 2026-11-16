
#define MAX_USERNAME_LEN 64
#define MAX_MESSAGE_LEN 256

typedef struct Message {
  char user[MAX_USERNAME_LEN];
  char message[MAX_MESSAGE_LEN];
} msg_t;
