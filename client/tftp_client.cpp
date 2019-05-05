#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 512
#define COMMAND_LEN 512

#define TXTMODE 1
#define BINMODE 2

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"

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

struct client_request {
	char * filename;
	char * localname;
	uint8_t mode;
};

int main(int argc, char* argv[])
{

	// Check number of arguments
	if(argc < 3) {
		printf(RED "Errore: il numero di argomenti non e' sufficiente.\n" RESET);
		return -1;
	}

	char* ip_server = argv[1];
	char* porta_server = argv[2];

	char command[COMMAND_LEN];

	char help_command[COMMAND_LEN];
	strcpy(help_command, "!help\n");

	char quit_command[COMMAND_LEN];
	strcpy(quit_command, "!quit\n");

	char mode_command[COMMAND_LEN];
	strcpy(mode_command, "!mode");

	char get_command[COMMAND_LEN];
	strcpy(get_command, "!get");

	// Initialise client struct
	client_request client;
	client.mode = 0;


	printCommandList();


	while(true) {

		fgets(command, COMMAND_LEN, stdin);

		// !help COMMAND
		if(strcmp(command, help_command) == 0) {
			printCommandList();
			continue;
		}

		// !mode COMMAND
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
				client.mode = TXTMODE;
				printf(GRN "Modo di trasferimento testuale configurato\n" RESET);
				continue;

			} else if (strcmp(command_words[1], "bin\n") == 0){
				client.mode = BINMODE;
				printf(GRN "Modo di trasferimento binario configurato\n" RESET);
				continue;
			} else {
				printf(RED "Invalid command\n" RESET);
			}
			
			continue;
		}

		// !get COMMAND
		if(startsWith(command, get_command)){
			char* command_words[3];
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
			client.filename = command_words[1];
			client.localname = command_words[2];
			if(client.mode == 0) {
				printf(YEL "Attenzione: il metodo di trasferimento non e' stato ancora configurato\n" RESET);
				continue;
			}
			
		}

		// !quit COMMAND
		if(strcmp(command, quit_command) == 0){
			break;
		}


	}
	return 0;
}











