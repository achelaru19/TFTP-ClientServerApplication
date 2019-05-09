#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define DATA_LENGTH 512
#define BUF_LEN 516 // Data length + 4 bytes 


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"

int main(int argc, char* argv[])
{

	int ret, sd, len;
	socklen_t addrlen; 
    pid_t pid;
	char buffer[BUF_LEN];
    
    struct sockaddr_in my_addr, connecting_addr;

    
	if(argc < 3) {
		printf(RED "Errore: argomenti non sufficienti\n" RESET);
		return -1;
	}

    int port = atoi(argv[1]);
	const char* directory = argv[2];
    /* Creazione socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);
	printf(BLU "Creazione socket di ascolto\n" RESET);
    
    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); // Pulizia 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    
    if( ret < 0 ){
        perror(RED "Bind non riuscita\n" RESET);
        exit(1);
    }

	while(1) {
		memset(&buffer, 0, sizeof(buffer));
		
		ret = recvfrom(sd, buffer, BUF_LEN, 0, (struct sockaddr*)&connecting_addr, &addrlen);

		if(ret < 0) {
			perror(RED "Errore nella ricezione del messaggio\n" RESET);
			continue;
		} else if (ret == 0) {
			perror(RED "Il socket remoto si e' chiuso\n'"RESET);
			continue;
		}
		
		pid = fork();
		if( pid == 0 ){
            // Sono nel processo figlio
			printf(BLU "Sono nel processo del figlio\n" RESET);
			printf("%s figlio\n", buffer);

		} 
		else if (pid > 0) {
			// Sono nel processo del padre
			printf(BLU "Sono nel processo del padre\n" RESET);
			printf("%s padre\n", buffer);
	
		} 
		else {
			perror(RED "Errore nella creazione di un nuovo processo\n" RESET);
		}

	}
    

	char buff[DATA_LENGTH];
	memset(&buff, 0, DATA_LENGTH);
	int index = 0;

	FILE *fptr;

	fptr = fopen("./files/test.txt", "r");

	if(fptr == NULL) {
		printf("Errore: file non esiste\n");
		exit(1);
	}

	char charRead;
	while ((charRead = fgetc(fptr)) != EOF) {
		buff[index] = charRead;
		index++;
		if(index == DATA_LENGTH - 1){
			printf("Messaggio: %s\n", buff);
			memset(&buff, 0, DATA_LENGTH);	
			index = 0;
		}
	}
	printf("Messaggio: %s\n", buff);
	fclose(fptr);



	return 0;
}
