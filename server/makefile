
CFLAGS = -Wall

all: tftp_server clean

tftp_server: tftp_server.o
	@$(CC) -o tftp_server tftp_server.o

tftp_server.o: tftp_server.c global_variables.h
	@$(CC) $(CFLAGS) -c tftp_server.c 

clean:
	@$(RM) *.o 

