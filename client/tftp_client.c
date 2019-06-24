/**
	Progetto Reti Informatiche 2019
	Client TFTP
	Angel Chelaru
*/

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

const char* getTransferMode(struct RequestPacket req)
{
	return strcmp(req.mode, "octet") == 0 ? "wb" : "w";
}

bool startsWith(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int getWordCount(const char * command) 
{
	int i, count = 0;
	int len = strlen(command);
	char previousChar;
	if(len > 0) {
        previousChar = command[0];
    }
	for(i = 0; i <= len; ++i) {
		if((command[i] == ' ' || command[i] == '\0') && previousChar != ' '){
			count++;
		}
		previousChar = command[i];
	} 
	return count;
}

void printInvalidCommand()
{
	printf(YEL "Comando non valido. Usare il comando !help per ottenere la lista dei comandi validi\n" RESET);
}

int getCommandCode(char* command)
{
	if(strcmp(command, "!help\n") == 0) return HELP_COMMAND;
	if(strcmp(command, "!quit\n") == 0) return QUIT_COMMAND;
	if(startsWith(command, "!get ")) return GET_COMMAND;
	if(startsWith(command, "!mode ")) return MODE_COMMAND;
	return -1; // Commando non valido
}

void setMode(struct RequestPacket* request, char* command)
{
	char* command_words[2];
	int index = 0;
	int wordsCount = getWordCount(command);
	if(wordsCount < 2){
		printInvalidCommand();
		return;
	}
	char* words = strtok(command, " ");
	while(words != NULL){
		command_words[index] = words;
		index++;
		words = strtok(NULL, " ");
	}
	if(strcmp(command_words[1], "txt\n") == 0){
		strcpy(request->mode, "netascii");
		printf(GRN "Modo di trasferimento testuale configurato\n" RESET);
	} else if (strcmp(command_words[1], "bin\n") == 0){
		strcpy(request->mode, "octet");
		printf(GRN "Modo di trasferimento binario configurato\n" RESET);
	} else {
		printInvalidCommand();
	}
}

void sendFileRequest(struct RequestPacket* request, int sd, struct sockaddr_in* srv_addr) 
{	
	int ret;
	request->opcode = htons(RRW);
	request->zeroByteCode = 0x00;
	request->zeroByteMode = 0x00;
	ret = sendto(sd, (char*) request, sizeof(*request), 0,
	        (struct sockaddr*) srv_addr, sizeof(*srv_addr));
	if(ret < 0)  {
		printf(RED "Errore nell'invio della richiesta \n"RESET);
		exit(1);
	}
	printf(BLU "Richiesta del file %s inviata\n" RESET, request->filename);
}

void receiveAndSaveFile(int sd, char* localFilename, const char* transferMode)
{
	struct DataPacket* dataPacket;
	struct ErrorPacket* errorPacket;
	struct AckPacket ackPacket;
	FILE* fptr;
	char buffer[BUF_LEN];
	int packetLength, messageLength, ret;
	struct sockaddr_in srv_addr;
	socklen_t len;

	// Creo file se non esiste, altrimenti lo sovrascrivo
	fptr = fopen(localFilename, transferMode);
	
	// Pulizia
	memset(&srv_addr, 0, sizeof(srv_addr));

	while (1) {
		len = sizeof(srv_addr);
		packetLength = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*) &srv_addr, &len);
		if (packetLength < 0) {
			printf(RED "Errore nella ricezione del pacchetto\n" RESET);
		}

		dataPacket = (struct DataPacket*) buffer;
		if(ntohs(dataPacket->opcode) == ERR){
			errorPacket = (struct ErrorPacket*) buffer;
			printf(RED "Errore con codice %d: %s\n" RESET, ntohs(errorPacket->errorNumber), errorPacket->errorMessage);
			break;
		}

		// Inizializzo messageLength come packetLength meno i byte usati per l'intestazione
		messageLength = packetLength - 4;

		if( strcmp(transferMode, "wb") == 0 ) // modalita' di trasferimento binaria
			fwrite((void*)dataPacket->data, messageLength, 1, fptr);
		else if( strcmp(transferMode, "w") == 0 ){ // modalita' di trasferimentotesto testuale
			int j;
			for (j = 0; j < messageLength; ++j) {
				fputc(dataPacket->data[j], fptr); 				
			}	
		}

		// Invio ACK 
		ackPacket.opcode = htons(ACK);
		ackPacket.blockNumber = dataPacket->blockNumber; 
		ret = sendto(sd, (char*)&ackPacket, sizeof(ackPacket), 0, (struct sockaddr*) &srv_addr, len);
		if (ret < 0) {
			printf(YEL "Errore nell'invio dell'ACK\n" RESET);
		}

		// Controllo se sono all'ultimo blocco trasferito 
		if (messageLength < DATA_LENGTH) {
			printf(GRN "Download file %s completato\n" RESET, localFilename);
			printf(BLU "Il file e' stato salvato\n" RESET);
 			break;
		}
	}
	fclose(fptr);
}

void handleGetRequest(int sd, struct RequestPacket* request, char* command, struct sockaddr_in* srv_addr)
{
	char* command_words[3];
	char filename[FILENAME_SIZE];

	// Estrai filename e local filename dal comando
	int index = 0;
	int wordsCount = getWordCount(command);
	if(wordsCount < 3){
		printInvalidCommand();
		return;
	}
	char* words = strtok(command, " ");
	while(words != NULL){
		command_words[index] = words;
		index++;
		words = strtok(NULL, " ");
	}

	strcpy(request->filename, command_words[1]);

	// Invia richiesta del file
	sendFileRequest(request, sd, srv_addr);

	strcpy(filename, command_words[2]);
	filename[strlen(filename)-1] = '\0';
	// Ricevi il file
	receiveAndSaveFile(sd, filename, getTransferMode(*request));	

	// Reinizializza request per richieste future 
	memset(request, 0, sizeof(*request)); 
	strcpy(request->mode, "octet");
}

int main(int argc, char* argv[])
{
	
    int sd, porta_server, commandCode;
    struct sockaddr_in srv_addr;
	struct RequestPacket request;

	// Controllo il numero di argomenti
	if(argc < 3) {
		printf(RED "Errore: il numero di argomenti non e' sufficiente.\n" RESET);
		exit(-1);
	}

	const char* ip_server = argv[1];
	porta_server = atoi(argv[2]);
	
    // Creazione socket 
    sd = socket(AF_INET,SOCK_DGRAM,0);

	// Creazione indirizzo del server 
	memset(&srv_addr, 0, sizeof(srv_addr)); // Pulizia 
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(porta_server);
	inet_pton(AF_INET, ip_server, &srv_addr.sin_addr);

	char command[COMMAND_LEN];

	// Inizializza request
	strcpy(request.mode, "octet");

	printCommandList();

	while (true) {
		printf("Inserisci il comando\n");
		printf(BLU ">" RESET);
		fgets(command, COMMAND_LEN, stdin);

		commandCode = getCommandCode(command);

		switch (commandCode) {
			case HELP_COMMAND:
				printCommandList();
				break;
			case MODE_COMMAND:
				setMode(&request, command);
				break;
			case GET_COMMAND:
				handleGetRequest(sd, &request, command, &srv_addr);
				break;
			case QUIT_COMMAND:
				printf(YEL "Ciao! Al prossimo download!\n" RESET);
				close(sd);
				return 0;
			default:
				printInvalidCommand();
		}
	}
	return 0;
}

