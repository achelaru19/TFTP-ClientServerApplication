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



bool startsWith(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
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
		printf("%s\n", command);
		if(strcmp(command, help_command) == 0) {
			printf("%s", MESSAGGIO_HELP);
			continue;
		}
		if(startsWith(command, mode_command)){
			printf("mode command");		
			continue;
		}
		if(startsWith(command, get_command)){
			printf("get command");
		}

		if(strcmp(command, quit_command) == 0){
			break;
		}


	}
	
	return 0;
}











