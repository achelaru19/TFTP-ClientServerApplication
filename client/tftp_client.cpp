#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 512
#define MESSAGGIO_HELP "Sono disponibili i seguenti comandi:\r\n!help --> mostra l'elenco dei comandi disponibili\r\n!mode {txt|bin} --> imposta il modo di trasferimento dei files (testo o binario)\r\n!get filename nome_locale --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\r\n!quit --> termina il client\r\n"
#define COMMAND_LEN 512


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


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

int main(int argc, char* argv[]){

	char command[COMMAND_LEN];

	char help_command[COMMAND_LEN];
	strcpy(help_command, "!help\n");

	char quit_command[COMMAND_LEN];
	strcpy(quit_command, "!quit\n");

	char mode_command[COMMAND_LEN];
	strcpy(mode_command, "!mode");

	char get_command[COMMAND_LEN];
	strcpy(get_command, "!get");



	printf("%s", MESSAGGIO_HELP);


	while(true) {

		fgets(command, COMMAND_LEN, stdin);

		if(strcmp(command, help_command) == 0) {
			printf("%s", MESSAGGIO_HELP);
			continue;
		}
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
				printf(BLU "Set mode to txt\n" RESET);
				continue;

			} else if (strcmp(command_words[1], "bin\n") == 0){
				printf(BLU "Set mode to bin\n" RESET);
				continue;
			} else {
				printf(RED "Invalid command\n" RESET);
			}
			
			continue;
		}





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
			printf("%s\n", command_words[2]);
			
		}

		if(strcmp(command, quit_command) == 0){
			break;
		}


	}
	
	return 0;
}











