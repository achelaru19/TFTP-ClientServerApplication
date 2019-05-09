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



struct client_request {
	char * filename;
	char * localname;
	uint8_t mode;
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

void receiveFile(client_request cl, int sd, sockaddr_in srv_addr) 
{
	int ret;
	const char* fileToReceive = cl.filename;
	ret = sendto(sd, fileToReceive, sizeof(fileToReceive), 0,
            (struct sockaddr*)&srv_addr, sizeof(srv_addr));
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
    
    /* Creazione socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    
    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); // Pulizia 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4243);
    my_addr.sin_addr.s_addr = INADDR_ANY;


    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    
    if( ret < 0 ){
        perror(RED "Bind non riuscita\n" RESET);
        exit(0);
    }
    
    
    /* Creazione indirizzo del server */
    memset(&srv_addr, 0, sizeof(srv_addr)); // Pulizia 
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(porta_server);
    inet_pton(AF_INET, ip_server, &srv_addr.sin_addr);



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
	client.mode = BINMODE;


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
			receiveFile(client, sd, srv_addr);
				
		}

		// !quit COMMAND
		if(strcmp(command, quit_command) == 0){
			break;
		}


	}
	return 0;
}











