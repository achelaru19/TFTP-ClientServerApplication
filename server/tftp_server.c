#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "global_variables.h"


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

void sendError(int sd, const char* errMessage, int errNumber, struct sockaddr_in* client_addr)
{
	ErrorPacket errorPacket;
	errorPacket.opcode = htons(ERR);
	errorPacket.errorNumber = htons(errNumber);
	strcpy(errorPacket.errorMessage, errMessage);
	errorPacket.zeroByte = ZERO_BYTE;
	sendto(sd, (char*) &errorPacket, sizeof(errorPacket), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
	printf(YEL "Inviato messaggio di errore\n" RESET);

}

void sendFile(int sd, RequestPacket* request, struct sockaddr_in* client_addr) 
{
	FILE *fptr;
	DataPacket dataPacket;
	char path[1000];
	char buffer[DATA_LENGTH];
	char file[FILENAME_SIZE];
	int fileSize, numberOfPackets, remainingBytes;
	socklen_t len;
	const char* typeOfFile = strcmp(request->mode, "netascii") == 0 ? "r" : "rb";

	strcpy(file, request->filename);


	strcpy(path, "./files/");

	strcat(path, file);
	fptr = fopen(path, typeOfFile);

	if (fptr == NULL) {
		sendError(sd, "File non esistente", ERR_FILE_NOT_FOUND, client_addr);
		fclose(fptr);
		exit(1);
	}

	fseek(fptr,0,SEEK_END);
	fileSize=ftell(fptr);
	remainingBytes = fileSize;
	fseek(fptr,0,SEEK_SET);
	numberOfPackets = fileSize/DATA_LENGTH + 1;

	for (int i = 0; i < numberOfPackets; ++i) {

		int dataLength = MIN(DATA_LENGTH, remainingBytes);
	
		dataPacket.opcode = htons(DTA);
		dataPacket.blockNumber = htons(i);
		
		if( dataLength != 0 ){
			if( strcmp(typeOfFile, "rb") == 0 ) //modo binario
				fread((void*)dataPacket.data, dataLength, 1, fptr);	
			else if( strcmp(typeOfFile, "r") == 0 ){ //modo testuale
				for (int j = 0; j < dataLength; ++j) {
					dataPacket.data[j] = fgetc(fptr); 
				}
			}
		} else{
			strcpy(dataPacket.data,"");		
		}

		// Invio pacchetto
		sendto(sd, (char*)&dataPacket, sizeof(dataPacket), 0 ,(struct sockaddr*)client_addr, sizeof(*client_addr));

		// Attendo ACK
		recvfrom(sd, buffer, DATA_LENGTH, 0, (struct sockaddr*)client_addr, &len);

		/* Controllo che l'ACK sia quello corretto */
		AckPacket* ackPacket = (AckPacket*) &buffer;
		ackPacket->blockNumber = ntohs(ackPacket->blockNumber);
		if(ackPacket->blockNumber != i) {
			printf(RED "E' stato perso il pacchetto %d\n" RESET, i);
		}
				
		remainingBytes -= dataLength;	
	}
	fclose(fptr);
	close(sd);
	printf(GRN "Inviati %d/%d pacchetti\n" RESET, numberOfPackets, numberOfPackets);
}

void handleRequest(RequestPacket* request, struct sockaddr_in* client_addr)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);

	request->opcode = ntohs(request->opcode);
	switch (request->opcode) {
		case RRQ: sendFile(sock, request, client_addr);
				   break;
		case WRQ: sendError(sock, "Opcode non valido", ERR_OPCODE_NOT_VALID, client_addr);
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

    port = atoi(argv[1]);
	directory = argv[2];

	/* Pulizia */
    memset(&my_addr, 0, sizeof(my_addr)); 

    /* Creazione socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);
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
		printf(BLU "Richiesta ricevuta\n" RESET);

		pid = fork();

		if (pid == 0) {
			handleRequest((RequestPacket*)&buffer, &connecting_addr);
		} 
		else if (pid < 0) {
			perror(RED "Errore nella creazione di un nuovo processo\n" RESET);
		}
	}

    close(sd);
	return 0;
}
