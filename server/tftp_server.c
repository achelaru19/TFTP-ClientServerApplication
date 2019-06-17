#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DATA_LENGTH 512
#define BUF_LEN 516 // Data length + 4 bytes 
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

int globalSD;


struct DataPacket {
	uint16_t opcode;
	uint16_t blockNumber;
	char data[DATA_LENGTH];
};

struct AckPacket {
	uint16_t opcode;
	uint16_t blockNumber;
};

struct ErrorPacket {
	uint16_t opcode;
	uint16_t errorNumber;
	char errorMessage[DATA_LENGTH];
	uint8_t zeroByte;
};

struct RequestPacket{
	uint16_t opcode;
	char filename[FILENAME_SIZE];
	uint8_t zeroByteCode;
	char mode[MODE_SIZE];
	uint8_t zeroByteMode;
};

void handler(int signal) 
{
	close(globalSD);
	exit(0);
}

void sendError(int sd, const char* errMessage, int errNumber, struct sockaddr_in* client_addr)
{
	ErrorPacket* errorPacket;
	errorPacket->opcode = htons(ERR);
	errorPacket->errorNumber = htons(errNumber);
	strcpy(errorPacket->errorMessage, errMessage);
	errorPacket->zeroByte = ZERO_BYTE;

	sendto(sd, (char*) &errorPacket, sizeof(errorPacket), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
	printf(YEL "Inviato messaggio di errore\n" RESET);

}

void sendFile() 
{
/*
	FILE *fptr;
	
	const char* typeOfFile = strcmp(mode, "netascii") == 0 ? "r" : "rb";

	fptr = fopen(filename, typeOfFile);

	if(fptr == NULL) {
		char buffer[512];
		uint16_t opcode = htons(ERR);
		uint16_t errorNumber = htons(ERR_FILE_NOT_FOUND);
		const char* errorMessage = "File non trovato\0";
	
		memcpy(buffer+pos, &opcode, sizeof(opcode));
		pos += sizeof(opcode);

		memcpy(buffer+pos, &errorNumber, sizeof(errorNumber));
		pos += sizeof(errorNumber);

		strcpy(buffer+pos, errorMessage);
		pos += strlen(errorMessage) + 1;
		printf("%d\n", sizeof(buffer));
		printf("%d\n", sizeof(connecting_addr));
		ret = sendto(childSocket, buffer, pos, 0, (struct sockaddr *)&connecting_addr, sizeof(connecting_addr));
		printf("Inviato messaggio di errore. ret vale %d\n", ret);
		fclose(fptr);
		exit(1);
	}
*/
}

void handleRequest(RequestPacket* request, struct sockaddr_in* client_addr)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);

	switch (request->opcode) {
		case(RRQ): sendFile();
				   break;
		case(WRQ): sendError(sock, "Opcode non valido", ERR_OPCODE_NOT_VALID, client_addr);
				   break;
		default: sendError(sock, "Opcode non valido", ERR_OPCODE_NOT_VALID, client_addr);
 				 break;
	}
}

int main(int argc, char* argv[])
{
	int ret, sd, port;
	char* directory;
    struct sockaddr_in my_addr;
    
	if(argc < 3) {
		printf(RED "Errore: argomenti non sufficienti\n" RESET);
		exit(2);
	}

	/* Gestisci segnale di interruzione per chiudere il socket 
	   in caso di terminazione forzata del server */
	signal(SIGKILL, handler);

    port = atoi(argv[1]);
	directory = argv[2];

	/* Pulizia */
    memset(&my_addr, 0, sizeof(my_addr)); 

    /* Creazione socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);
	globalSD = sd;
	printf(BLU "Creato socket di ascolto\n" RESET);
    
    /* Creazione indirizzo di bind */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    
    if( ret < 0 ){
        perror(RED "Bind non riuscita\n" RESET);
        exit(1);
    }

	while(true) {
		struct sockaddr_in connecting_addr;
		char buffer[BUF_LEN];
    	pid_t pid;
		socklen_t len = sizeof(connecting_addr);

		/* Pulizia */
    	memset(&connecting_addr, 0, len);
		memset(&buffer, 0, sizeof(buffer));
		
		ret = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*)&connecting_addr, &len);

		if (ret < 0) {
			perror(RED "Errore nella ricezione del messaggio\n" RESET);
			continue;
		} else if (ret == 0) {
			perror(RED "Il socket remoto si e' chiuso\n'" RESET);
			continue;
		}
		
		pid = fork();

		if (pid == 0) {
			handleRequest((RequestPacket*)&buffer, &connecting_addr);
		} 
		else if (pid < 0){
			perror(RED "Errore nella creazione di un nuovo processo\n" RESET);
		}
	}

    close(sd);
	return 0;
}
