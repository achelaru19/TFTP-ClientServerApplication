#define DATA_LENGTH 512
#define BUF_LEN 516
#define FILENAME_SIZE 100
#define MODE_SIZE 10

#define RRQ 1
#define WRQ 2
#define DTA 3
#define ACK 4
#define ERR 5

#define ZERO_BYTE 0x00
#define ERR_FILE_NOT_FOUND 1
#define ERR_OPCODE_NOT_VALID 2

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"

#define MIN(a,b) ((a) < (b) ? a : b)



