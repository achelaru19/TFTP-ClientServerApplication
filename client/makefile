
CFLAGS = -Wall

all: tftp_client clean

tftp_client: tftp_client.o
	@$(CC) -o tftp_client tftp_client.o

tftp_client.o: tftp_client.c
	@$(CC) $(CFLAGS) -c tftp_client.c

clean:
	@$(RM) *.o

