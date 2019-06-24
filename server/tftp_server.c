/**
	Progetto Reti Informatiche 2019
	Server TFTP
	Angel Chelaru
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "global_variables.h"

// Strutture dati per i pacchetti
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
	struct ErrorPacket errorPacket;
	int ret;
	printf("Si e' verificato il seguente errore: %s \n", errMessage);
	errorPacket.opcode = htons(ERR);
	errorPacket.errorNumber = htons(errNumber);
	strcpy(errorPacket.errorMessage, errMessage);
	errorPacket.zeroByte = ZERO_BYTE;
	ret = sendto(sd, (char*) &errorPacket, sizeof(errorPacket), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
	if (ret < 0) 
		printf(RED "Errore nell'invio del massaggio di errore\n" RESET);
	else
		printf(YEL "Inviato messaggio di errore: %s\n" RESET, errMessage);

}

void sendFile(int sd, struct RequestPacket* request, struct sockaddr_in* client_addr, char* directory) 
{
	FILE *fptr;
	struct DataPacket dataPacket;
	char path[strlen(directory) + FILENAME_SIZE];
	char buffer[BUF_LEN];
	int fileSize, numberOfPackets, remainingBytes, ret, i;
	socklen_t len = sizeof(*client_addr);
	const char* typeOfFile = strcmp(request->mode, "netascii") == 0 ? "r" : "rb";

	strcpy(path, directory);
	strcat(path, request->filename);


	fptr = fopen(path, typeOfFile);

	if (fptr == NULL) {
		sendError(sd, "File non esistente", ERR_FILE_NOT_FOUND, client_addr);
		fclose(fptr);
		exit(1);
	}

	fseek(fptr,0,SEEK_END);
	fileSize = ftell(fptr);
	remainingBytes = fileSize;
	fseek(fptr, 0, SEEK_SET);

	numberOfPackets = fileSize/DATA_LENGTH + 1;

	printf(BLU "Inizio invio del file\n" RESET);

	for (i = 0; i < numberOfPackets; ++i) {

		int dataLength = MIN(DATA_LENGTH, remainingBytes);
	
		dataPacket.opcode = htons(DTA);
		dataPacket.blockNumber = htons(i);
		memset(&dataPacket.data, 0, DATA_LENGTH);
		
		if (dataLength != 0) {
			if ( strcmp(typeOfFile, "rb") == 0 ) //modo binario
				fread((void*)dataPacket.data, dataLength, 1, fptr);	
			else if (strcmp(typeOfFile, "r") == 0 ) { //modo testuale
				int j;
				for (j = 0; j < dataLength; ++j) {
					dataPacket.data[j] = fgetc(fptr); 
				}
			}
		} else {
			strcpy(dataPacket.data,"");		
		}

		// Invio pacchetto
		ret = sendto(sd, (const char*)&dataPacket, dataLength + 4, 0 ,(const struct sockaddr*)client_addr, sizeof(*client_addr));
		if (ret < 0) 
			printf(RED "Errore nell'invio del pacchetto numero %d\n" RESET, i);
		
		len = sizeof(*client_addr);

		// Attendo ACK 
		ret = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*)client_addr, &len);
		if (ret < 0) 
			printf(RED "Errore nella ricezione dell'ACK\n" RESET);

		// Eventuali controlli sull'ACK
		struct AckPacket* ackPacket = (struct AckPacket*) &buffer;
		if (ntohs(ackPacket->blockNumber) != i) {
			printf(YEL "ACK ricevuto non e' quello previsto: %d\n" RESET, i);
		}
			
		remainingBytes -= dataLength;

		if ((i+1) % 10 == 0) {
			printf("Inviati %d/%d pacchetti \n", i+1, numberOfPackets);
		}
	}
	fclose(fptr);
	close(sd);
	printf(GRN "Inviati %d/%d pacchetti\n" RESET, numberOfPackets, numberOfPackets);
}

void handleRequest(struct RequestPacket* request, struct sockaddr_in* client_addr, int sock, char* directory)
{
	printf(BLU "Richiesta del file %s ricevuta\n" RESET, request->filename);
	request->opcode = ntohs(request->opcode);
	switch (request->opcode) {
		case RRQ: 
				sendFile(sock, request, client_addr, directory);
				break;
		case WRQ: 
				sendError(sock, "Opcode non valido", ERR_OPCODE_NOT_VALID, client_addr);
				break;
		default: 
				sendError(sock, "Opcode non valido", ERR_OPCODE_NOT_VALID, client_addr);
 				break;
	}
}

int main(int argc, char* argv[])
{
	int ret, sd, port, childSock;
	char* directory;
    struct sockaddr_in my_addr, connecting_addr;
	char buffer[BUF_LEN];
	pid_t pid;
	socklen_t len;
    
	if (argc < 3) {
		printf(RED "Errore: argomenti non sufficienti\n" RESET);
		exit(2);
	}

    port = atoi(argv[1]);
	directory = argv[2];

	// Pulizia 
    memset(&my_addr, 0, sizeof(my_addr)); 

    // Creazione socket 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
	printf(BLU "Creato socket di ascolto %d\n" RESET, sd);
    
    // Creazione indirizzo di bind 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    
    if ( ret < 0 ) {
        perror(RED "Bind non riuscita\n" RESET);
        exit(1);
    }

	printf(GRN "Il server e' pronto per ricevere richieste\n" RESET);

	while (true) {
		// Pulizia 
    	memset(&connecting_addr, 0, sizeof(connecting_addr));
		memset(buffer, 0, BUF_LEN);
	
		len = sizeof(connecting_addr);
		ret = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*)&connecting_addr, &len);
		if (ret < 0) {
			printf(RED "Errore nella ricezione della richiesta del file da scaricare\n" RESET);
		}
		
		childSock = socket(AF_INET, SOCK_DGRAM, 0);

		pid = fork();
		if (pid < 0) {
			perror(RED "Errore nella creazione di un nuovo processo\n" RESET);
		}
		else if (pid == 0) {
			// Processo figlio
			close(sd);
			handleRequest((struct RequestPacket*)&buffer, &connecting_addr, childSock, directory);
			exit(0);
		} 
		else {
			// Processo padre
			close(childSock);
		}
	}
    close(sd);
	return 0;
}
