# Builds the client and server programs

all: client server

clean:
	rm -f client server
	
# builds client prog
client: client.c
	gcc client.c -o client

# builds server prog
server: server.c
	gcc -O3 server.c sqlite3.c -o server -lpthread -ldl