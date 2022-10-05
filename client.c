#include <stdio.h>
#include <stdlib.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5425
#define MAX_LINE 256

int
main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in server;
  char *host;
  char buf[MAX_LINE];
  int s, k;
  int shutdownFlag = 0; 
  int quitFlag = 0;
  int len;

  if (argc==2) {
    host = argv[1];
  }
  else {
    fprintf(stderr, "Missing host address.\n");
    exit(1); }

  /* translate host name into peer’s IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
  exit(1); }

  /* build address data structure */
  bzero((char *)&server, sizeof(server));
  server.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
  server.sin_port = htons(SERVER_PORT);

/* active open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      printf("Could not create socket.");
      exit(1);
      }
      
  if ((k = connect(s, (struct sockaddr *)&server, sizeof(server))) < 0) {
      printf("Could not connect.");
      close(s);
      exit(1);
  }

/* main loop: get and send lines of text */
  while(shutdownFlag == 0 && quitFlag == 0){
    printf("\nc: ");
    fgets(buf,100,stdin);

    if(strncmp(buf,"QUIT",4)==0){
      quitFlag = 1;
    }

    if(strncmp(buf,"SHUTDOWN",8)==0){
      shutdownFlag = 1;
    }
    //Use "QUIT” to end communication with server

    k=send(s,buf,100,0);
    if(k==-1){
      printf("Error in sending");
    }

    k=recv(s,buf,100,0);
    if(k==-1){
      printf("Error in receiving");
    }

    printf("s: %s",buf);
  }
  close(s);
  return(0);
}