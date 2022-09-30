# Builds the client and server programs

all: client server

clean:
	rm -f client server
	
# builds client prog
client: client.c
	gcc -o client client.c

# builds server prog
server: server.c
	gcc -o server server.c
