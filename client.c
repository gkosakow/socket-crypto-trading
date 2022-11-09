#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5425      // defining SERVER_PORT as the last 4 of my student ID.
#define MAX_LINE 256          // defining MAX_LINE for char arrays as 256.

int
main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in server;
  char *host;
  char buf[MAX_LINE];
  int clientSocket, k;
  int shutdownFlag = 0; 
  int quitFlag = 0;
  int len;

  // setting host to argv[1] (e.g. localhost, 127.0.0.1, etc.)
  if ( argc == 2) {
    host = argv[1];
  } else {
      fprintf(stderr, "Missing host address.\n");     // throws error if address is not not specified
      exit(1); 
    }

  // translate host name into peer’s IP address
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "Unknown host: %s\n", host);
    exit(1); 
  }

  // build address data structure
  bzero((char *)&server, sizeof(server));
  server.sin_family = AF_INET;                                    // defines family AF_INET
  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);      // tells socket to listen on network interfaces
  server.sin_port = htons(SERVER_PORT);                           // port 5425

  // establishes new serverSocket and throws error if serversocket < 0.
  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      printf("Could not create socket.\n");
      exit(1);
      }
      
  // throws error if client cannot connect the socket to the server using the address struct.
  if ((k = connect(clientSocket, (struct sockaddr *)&server, sizeof(server))) < 0) {
      printf("Could not connect.\n");
      close(clientSocket);      // closes the socket and exits program.
      exit(1);
  }

  // main loop: get and send lines of text.
  while(shutdownFlag == 0 && quitFlag == 0){
    printf("\nc: ");
    fgets(buf, MAX_LINE, stdin);

    // quits out of program when "QUIT" is read from buf by setting quitFlag to 1 which ends while loop and sends "QUIT" to server.
    if(strncmp(buf, "QUIT", 4)==0){
      quitFlag = 1;
    }

    // quits out of program when "SHUTDOWN" is read from buf by setting shutdownFlag to 1 which ends while loop and sends "SHUTDOWN" to server.
    if(strncmp(buf, "SHUTDOWN", 8)==0){
      shutdownFlag = 1;
    }

    // notifies client that there was a sending error.
    k=send(clientSocket, buf, 100, 0);
    if(k==-1){
      printf("Error in sending\n");
    }

    // notifies client that there was a receiving error.
    k=recv(clientSocket, buf, 100, 0);
    if(k==-1){
      printf("Error in receiving\n");
    }

    printf("s: %s",buf);      // prints server socket's return message.
  }
  close(clientSocket);      // closes the 
  return(0);
}