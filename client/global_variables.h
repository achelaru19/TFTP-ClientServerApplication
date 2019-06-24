#define DATA_LENGTH 512
#define BUF_LEN 516 // Data length + 4 byte di intestazione
#define FILENAME_SIZE 100
#define MODE_SIZE 10
#define COMMAND_LEN 100

#define RRW 1
#define WRQ 2
#define DTA 3
#define ACK 4
#define ERR 5

#define HELP_COMMAND 0
#define MODE_COMMAND 1
#define GET_COMMAND 2
#define QUIT_COMMAND 3

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"

#define false 0
#define true 1
typedef int bool;
