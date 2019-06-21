#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "global_variables.h"


#define PORT 7561


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


void printCommandList()
{
	printf("Sono disponibili i seguenti comandi:\n");
	printf(BLU "help" RESET);
	printf(" --> mostra l'elenco dei comandi disponibili\n");
	printf(BLU "!mode {txt|bin}" RESET);
	printf(" --> imposta il modo di trasferimento dei files (testo o binario)\n");
	printf(BLU "!get filename nome_locale" RESET);
	printf(" --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\n");
	printf(BLU "!quit" RESET);
	printf(" --> termina il client\n");
}

const char* getTransferMode(RequestPacket req)
{
	return strcmp(req.mode, "octet") == 0 ? "wb" : "w";
}

bool startsWith(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int getWordCount(const char * command) 
{
	int count = 0;
	int len = strlen(command);
	char previousChar;
	if(len > 0) {
        previousChar = command[0];
    }
	for(int i = 0; i <= len; ++i) {
		if((command[i] == ' ' || command[i] == '\0') && previousChar != ' '){
			count++;
		}
		previousChar = command[i];
	} 
	return count;
}

void sendFileRequest(RequestPacket request, int sd, sockaddr_in srv_addr) 
{	
	int ret;

	request.opcode = htons(RRW);
	request.zeroByteCode = 0x00;
	request.zeroByteMode = 0x00;
	do {
		ret = sendto(sd, (char*) &request, sizeof(request), 0,
		        (struct sockaddr*)&srv_addr, sizeof(srv_addr));
		if(ret < 0) sleep(5);
	} while (ret < 0);
	printf(GRN "Richiesta del file %s inviata\n" RESET, request.filename);
}

void receiveAndaSaveFile(int sd, struct sockaddr_in* srv_addr, char* localFilename, const char* transferMode)
{
	char buffer[BUF_LEN];
	DataPacket* dataPacket;
	AckPacket ackPacket;
	ErrorPacket* errorPacket;
	FILE* fptr;
	int ret, messageLen;
	socklen_t len;
	fptr = fopen(localFilename, transferMode);

	printf("%s %s\n", localFilename, transferMode);

	while(true) {
		printf("loop while \n");
		do {
			ret = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*) srv_addr, &len);
			if (ret < 0) {
				sleep(5);
			}
		} while (ret < 0);
		dataPacket = (DataPacket*) buffer;
		printf("Qua entrato => %s\n", dataPacket->data);
		if(ntohs(dataPacket->opcode) == ERR){
			errorPacket = (ErrorPacket*) buffer;
			printf(RED "Errore numero %d: %s\n" RESET, ntohs(errorPacket->errorNumber), errorPacket->errorMessage);
			break;
		}

		messageLen = ret - 4;
		printf("messagelen %d\n%s \n", messageLen, dataPacket->data);

		if( strcmp(transferMode, "wb") == 0 ) // modalita' di trasferimento binaria
			fwrite((void*)dataPacket->data, messageLen, 1, fptr);
		else if( strcmp(transferMode, "w") == 0 ){ // modalita' di trasferimentotesto testuale
			for(int j = 0; j < messageLen; ++j) {
				fputc(dataPacket->data[j], fptr); 				
			}	
		}

		/* Invio ACK */

		ackPacket.opcode = htons(ACK);
		ackPacket.blockNumber = dataPacket->blockNumber; 
		printf("Sto per mandare ack\n");
		do {
			ret = sendto(sd, (char*) &ackPacket, sizeof(ackPacket), 0, (struct sockaddr*) srv_addr, sizeof(*srv_addr));
			if(ret < 0) sleep(5);
		} while (ret < 0);
		printf("ACK inviato\n");

		/* Controllo se sono all'ultimo blocco trasferito */
		printf("messLen %d \n", messageLen);
		if(messageLen < DATA_LENGTH){
			printf(GRN "Download file %s completato\n" RESET, localFilename);
 			break;
		}

	}
	fclose(fptr);
	close(sd);
}

int main(int argc, char* argv[])
{
	// Check number of arguments
	if(argc < 3) {
		printf(RED "Errore: il numero di argomenti non e' sufficiente.\n" RESET);
		return -1;
	}

	const char* ip_server = argv[1];
	int porta_server = atoi(argv[2]);
	
    int ret, sd, len;
    struct sockaddr_in srv_addr, my_addr;
	struct sockaddr_in connecting_addr;
	socklen_t addrlen; 


    /* Creazione socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);

	char command[COMMAND_LEN];

	char help_command[COMMAND_LEN];
	strcpy(help_command, "!help\n");

	char quit_command[COMMAND_LEN];
	strcpy(quit_command, "!quit\n");

	char mode_command[COMMAND_LEN];
	strcpy(mode_command, "!mode");

	char get_command[COMMAND_LEN];
	strcpy(get_command, "!get");

	// Initialise request
	RequestPacket request;
	strcpy(request.mode, "octet");

	printCommandList();

	while(true) {
		/* Creazione indirizzo del server */
		memset(&srv_addr, 0, sizeof(srv_addr)); // Pulizia 
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_port = htons(porta_server);
		inet_pton(AF_INET, ip_server, &srv_addr.sin_addr);

		fgets(command, COMMAND_LEN, stdin);

		// !help 
		if(strcmp(command, help_command) == 0) {
			printCommandList();
			continue;
		}

		// !mode 
		if(startsWith(command, mode_command)){
			char* command_words[2];
			int index = 0;
			int wordsCount = getWordCount(command);
			if(wordsCount < 2){
				printf(RED "Errore: non ci sono abbastanza argomenti\n" RESET);
				continue;
			}
			char* words = strtok(command, " ");
			while(words != NULL){
				command_words[index] = words;
				index++;
				words = strtok(NULL, " ");
			}
			if(strcmp(command_words[1], "txt\n") == 0){
				strcpy(request.mode, "netascii");
				printf(GRN "Modo di trasferimento testuale configurato\n" RESET);
				continue;

			} else if (strcmp(command_words[1], "bin\n") == 0){
				strcpy(request.mode, "octet");
				printf(GRN "Modo di trasferimento binario configurato\n" RESET);
				continue;
			} else {
				printf(RED "Comando non valido\n" RESET);
			}
			
			continue;
		}

		// !get 
		if(startsWith(command, get_command)){
			char* command_words[3];
			char filename[100];
			int index = 0;
			int wordsCount = getWordCount(command);
			if(wordsCount < 3){
				printf(RED "Errore: non ci sono abbastanza argomenti\n" RESET);
				continue;
			}
			char* words = strtok(command, " ");
			while(words != NULL){
				command_words[index] = words;
				index++;
				words = strtok(NULL, " ");
			}
			strcpy(request.filename, command_words[1]);
			sendFileRequest(request, sd, srv_addr);
			strcpy(filename, command_words[2]);
			filename[strlen(filename)-1] = '\0';
			receiveAndaSaveFile(sd, &srv_addr, filename, getTransferMode(request));	
			/* Reinitialise request for future requests */
			memset(&request, 0, sizeof(request)); 
			strcpy(request.mode, "octet");
		}

		// !quit 
		if(strcmp(command, quit_command) == 0){
			close(sd);
			break;
		}

	}
	return 0;
}











