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
#define COMMAND_LEN 16


int main(int argc, char* argv[]){

	char command[COMMAND_LEN];

	char help_command[COMMAND_LEN];
	strcpy(help_command, "!help\n");

	char quit_command[COMMAND_LEN];
	strcpy(quit_command, "!quit\n");


	while(true) {
		printf("%s", MESSAGGIO_HELP);
		fgets(command, COMMAND_LEN, stdin);
		printf("%s\n", command);
		if(strcmp(command, help_command) == 0) {
			printf("%s", MESSAGGIO_HELP);
			continue;
		}

		if(strcmp(command, quit_command) == 0){
			break;
		}


	}
	
	return 0;
}
