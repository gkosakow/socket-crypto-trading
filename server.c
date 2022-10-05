/*
Simple Program to work with DB's
Greg Kosakowski
CIS 427
9/22/2022
*/

#include <stdio.h>
#include <stdlib.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sqlite3.h>

#define SERVER_PORT  5425     // defining SERVER_PORT as the last 4 of my student ID.
#define MAX_PENDING  5        // defining MAX_PENDING connections as 5.
#define MAX_LINE     256      // defining MAX_LINE for char arrays as 256.

// callback function retrieves the contents database used by SELECT function, as outlined in SQLite tutorial
// count: holds number of results, columns: hold each column returned in array, data: holds each value in array.
static int callback(void *NotUsed, int count, char **data, char **columns) {
   int i;
   for(i = 0; i < count; i++) {
      printf("%s = %s", columns[i], data[i] ? data[i] : "NULL");
   }
   printf("\n");
   return 0;
}

// defining struct Arguments to initialize command identifiable variables (e.g. Buy.crypto_Name, Sell.user_ID)
struct Arguments{
  char crypto_Name[16];
  double crypto_Amount;
  double price_per_crypto;
  double crypto_Price;
  double crypto_Balance;
  int user_ID;
  int crypto_ID;
  double user_Balance;
  char first_Name[16];
  char last_Name[16];
};

int main(int argc, char* argv[]){
  const char* dbFile = "cis427_crypto.sqlite";      // defining dbFile to be opened by openDB.
  sqlite3* DB;                                      // delcaring sqlite3 struct as DB.
  struct sqlite3_stmt *selectstmt;                  // declaring sqlite3)stmt struct selectstatement.

  int rc;                           // return code from sqlite3 commands, will return 0 if command issued successfully.
  char *sql;                        // a pointer sql for sql commands to be stored and used in sqlite3_exec.
  char *ErrMsg;                     // a pointer ErrMsg for sqlite3_exec commands to use. 
  char userInput[100];              // the string where the user's commands will be stored temporarily.
  const char separator[4] = " ";    // and initializing a separator value that will help separate user commands and arugments from their input.
  char *token;                      // pointer token to help store each command and argument into the commands[] array.
  char *commands[5];                // pointer array for 5 commands/arguments.
  int quit_Flag = 0;
  int shutdown_Flag = 0;            // and initializing shutdown_Flag to allow user to shut down the server via SHUTDOWN command.
  char *returnMessage;              // a return message string to send the received command back to the client.
  int k;                            // error handling variable for socket commands.

  // opens SQLite3 database file and if the return code returns anything but 0 (SQLITE_OK), results as an error and outputs "Cannot open db".
  rc = sqlite3_open(dbFile, &DB);
  if(rc){
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(DB));
    return(0);    
  }

  struct sockaddr_in client;   // new struct sockaddr_in as client.
  char buf[MAX_LINE];       // declaring buffer for user input as buf[256].
  int addr_len;             // addr length for socket commands.
  int serverSocket, clientSocket;             // declaring socket ints serverSocket and clientSocket.

  // prepares the sockaddr_in structure
  bzero((char *)&client, sizeof(client));
  client.sin_family = AF_INET;             // defines family AF_INET
  client.sin_addr.s_addr = INADDR_ANY;     // tells socket to listen on network interfaces
  client.sin_port = htons(SERVER_PORT);    // port 5425

  // setting up a passive open and creates new socket serverSocket.
  if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Could not create socket.");
    exit(1); 
  }

  // binds newly created socket to the specified address
  if ((bind(serverSocket, (struct sockaddr *)&client, sizeof(client))) < 0) {
    perror("Could not bind socket.");
    exit(1);
  }
  
  if (listen(serverSocket, MAX_PENDING) < 0){
    perror("Listening error.");
    exit(1);
  }

  // counts rows from Users table, if less than 1, INSERT into Users a single user with $100 usd.
  rc = sqlite3_prepare_v2(DB, "SELECT COUNT(*) FROM Users;", -1, &selectstmt, NULL);
  if(rc == SQLITE_OK) {
    if(sqlite3_step(selectstmt) == SQLITE_ROW) {
      int row_Count = sqlite3_column_double (selectstmt, 0);
      if(row_Count < 1){
        sql = "INSERT INTO Users (email, first_name, last_name, user_name, password, usd_balance) VALUES('gkosakow@umich.edu', 'Greg', 'Kosakowski', 'gkosakow', 'password123', 100);";
        sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
      }
    }
  }           

  // waits for connection, then receive and print text
  while(shutdown_Flag == 0) {
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&client, &addr_len)) < 0) {
      printf("Could not accept connection.");
    }
    // main while loop to allow the user to input multiple SQL commands from client connection
    while (quit_Flag ==  0 && shutdown_Flag == 0){

      // receives messages from client and checks if there is a recv error.
      if((k = recv(clientSocket, buf, 100, 0)) < 0){
        printf("Error receiving message from client.\n");
        exit(1);
      }

      // copies the received string from buffer and loads into userInput.
      strcpy(userInput, buf);
      printf("Received from client: %s", userInput);

      // using strtok to separate string into tokens, in which this gets the first token out of the user input string.
      token = strtok(userInput, separator);

      // stores each individual command/argument as a token and pushes it to the commands[] array.
      int i = 0;
      while (token != 0) {
          commands[i] = token;
          token = strtok(0, separator);
          i++;
          }
      
      // initializing the userCommand pointer as the first token from user input string (e.g. "BUY" from BUY WLVRN 3.4 1.35 1).
      char *userCommand = commands[0];

      // BUY COMMAND BLOCK
      // executes when there are 5 commands from userInput and when userCommand(commands[0]==BUY)
      if(strcmp(userCommand,"BUY") == 0 && i == 5){
        struct Arguments Buy;     // Declaring new Arguments struct for BUY function.

        strcpy(Buy.crypto_Name, commands[1]);                         // Copies second token in command (e.g. "DOGECOIN") to Buy.crypto_Name.
        Buy.crypto_Amount = atof(commands[2]);                        // Copies the third token of the crypto amount to Buy.crypto_Amount.
        Buy.price_per_crypto = atof(commands[3]);                     // Copies fourth token of ppc to Buy.price_per_crypto.
        Buy.user_ID = atoi(commands[4]);                              // Copies fifth token to Buy.user_ID.
        Buy.crypto_Price = Buy.crypto_Amount * Buy.price_per_crypto;  // Calculating the price of the crypto (amount*ppc) to Buy.crypto_Price.

        // Break out of BUY command when user inputs a negative value for crypto_Amount or price_per_crypto
        if(Buy.crypto_Amount <= 0 || Buy.price_per_crypto <= 0){
          returnMessage = "403 message format error: Cannot have negatives or zeroes crypto amounts or prices.\n";
          send(clientSocket,returnMessage,100,0);
          continue;
        }

        // SELECT usd_balance FROM Users WHERE ID = 1 and set Buy.user_Balance to the usd_balance value data.
        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Buy.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Buy.user_Balance = sqlite3_column_double (selectstmt, 0); // Initializing Buy.user_Balance with the result from matching usd_balance from user_ID.
            
            // IF-ELSE block to ensure that the user cannot buy more crypto than they have money for.
            if(Buy.user_Balance - Buy.crypto_Price >= 0){
              asprintf(&sql, "UPDATE Users SET usd_balance = usd_balance - '%f' WHERE ID = '%d';", Buy.crypto_Price, Buy.user_ID);
              sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
            } else {
                asprintf(&returnMessage, "403 message format error: Not enough USD in account for user %d.\n", Buy.user_ID);     // Notifies user when they do not have enough money for transaction.
                send(clientSocket,returnMessage,100,0);
                continue;
            }
            
          } else {
              asprintf(&returnMessage, "403 message format error: User %d does not exist.\n", Buy.user_ID);      // Notifies user when there is no existing user with ID input into the system.
              send(clientSocket,returnMessage,100,0);
              continue;
          }
        }
        
        // This code block INSERTS the new crypto listing in the Cryptos table, if SQL throws error code for existing crypto_Name, UPDATE the listing instead.
        asprintf(&sql, "INSERT INTO Cryptos (crypto_name, crypto_balance, user_id) VALUES('%s', '%f', '%i');", Buy.crypto_Name, Buy.crypto_Amount, Buy.user_ID);
        rc = sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
        if( rc != SQLITE_OK ) {
          asprintf(&sql, "UPDATE Cryptos SET crypto_Balance = crypto_Balance + '%f' WHERE crypto_name = '%s';", Buy.crypto_Amount, Buy.crypto_Name);
          sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
        }

        // This code block SELECTS the crypto_balance matched to the input crypto_name to print the new balance and crypto name for client to read.
        asprintf(&sql, "SELECT crypto_balance FROM Cryptos WHERE crypto_name = '%s';", Buy.crypto_Name);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Buy.crypto_Balance = sqlite3_column_double (selectstmt, 0);
          }
        }

        // Final print function to display the new information after the user buys crypto.
        asprintf(&returnMessage, "200 OK\nBOUGHT: New balance: %.1f %s USD balance $%.2f\n", Buy.crypto_Balance, Buy.crypto_Name, Buy.user_Balance - Buy.crypto_Price);
        send(clientSocket,returnMessage,100,0);
        continue;
      }

      // "SELL" COMMAND BLOCK

      else if(strcmp(userCommand,"SELL") == 0 && i == 5){
        struct Arguments Sell;    // Declaring new Arguments struct for Sell function

        strcpy(Sell.crypto_Name, commands[1]);                            // Copies second token in command (e.g. "DOGECOIN") to Sell.crypto_Name.
        Sell.crypto_Amount = atof(commands[2]);                           // Copies the third token of the crypto amount to Sell.crypto_Amount.
        Sell.price_per_crypto = atof(commands[3]);                        // Copies fourth token of ppc to Sell.price_per_crypto.
        Sell.user_ID = atoi(commands[4]);                                 // Copies fifth token to Sell.user_ID.
        Sell.crypto_Price = Sell.crypto_Amount * Sell.price_per_crypto;   // Calculating the price of the crypto (amount*ppc) to Sell.crypto_Price.

        // Breaks out of SELL command when user inputs a negative value for crypto_Amount or price_per_crypto
        if(Sell.crypto_Amount <= 0 || Sell.price_per_crypto <= 0){
          returnMessage = "403 message format error: Cannot have negatives or zeroes crypto amounts or prices.\n";
          send(clientSocket,returnMessage,100,0);
          continue;
        }

        // SELECT crypto_balance FROM Cryptos WHERE crypto_name = "name of crypto" and set Sell.crypto_Balance to the crypto_balance value data.
        asprintf(&sql, "SELECT crypto_balance FROM Cryptos WHERE crypto_name = '%s';", Sell.crypto_Name);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Sell.crypto_Balance = sqlite3_column_double (selectstmt, 0); // 

            // Checks that the crypto_amount transaction won't fall below 0.
            if(Sell.crypto_Balance >= Sell.crypto_Amount){
              asprintf(&sql, "UPDATE Users SET usd_balance = usd_balance + '%f' WHERE ID = '%d';", Sell.crypto_Price, Sell.user_ID);
              rc = sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
            } else {
                asprintf(&returnMessage, "403 message format error: Not enough %s crypto balance.\n", Sell.crypto_Name);     // Notifies the user when there aren't enough funds in usd_balance for user_ID to buy new crypto.
                send(clientSocket, returnMessage, 100, 0);
                continue;
            }
          } else {
              asprintf(&returnMessage, "403 message format error: Crypto record %s does not exist in Cryptos table.\n", Sell.crypto_Name);      // Notifies the user that the record does not exist when the balance can't be found for crypto_Name.
              send(clientSocket, returnMessage, 100, 0);
              continue;
          }
        }
        
        // Removes crypto_Amount from crypto_balance SELL command from the db Cryptos table.
        asprintf(&sql, "UPDATE Cryptos SET crypto_balance = crypto_balance - '%f' WHERE crypto_name = '%s';", Sell.crypto_Amount, Sell.crypto_Name);
        sqlite3_exec(DB, sql, callback, 0, &ErrMsg);

        // Setting Sell.user_Balance to the usd_balance found for user_ID so that it can be displayed later.
        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Sell.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Sell.user_Balance = sqlite3_column_double (selectstmt, 0);
          }
        }
        
        // Prints the results of the new crypto_Balance after selling and the new user_ID usd_Balance to client.
        asprintf(&returnMessage, "200 OK\nSOLD: New balance: %.1f %s USD balance $%.2f\n", Sell.crypto_Balance - Sell.crypto_Amount, Sell.crypto_Name, Sell.user_Balance);
        send(clientSocket, returnMessage, 100, 0);
        continue; //SELL(dbFile, crypto_Name, crypto_Amount, price_per_crypto, user_ID);
      }

      // "LIST" COMMAND BLOCK

      else if(strcmp(userCommand,"LIST\n") == 0){
        struct Arguments List;      // Declaring new Arguments struct for LIST command.

        // Prints "list of record in Cryptos db" to client
        char cryptoTableList[1024] = "The list of records in the Crypto database for user 1:\n";

        // Function to SELECT and initialize db data into variables so I can display them to the user.
        asprintf(&sql, "SELECT * FROM Cryptos WHERE user_id = 1;");
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          while (sqlite3_step(selectstmt) == SQLITE_ROW) {
            List.crypto_ID = sqlite3_column_int (selectstmt, 0);                      // Initializes List.crypto_ID with the ID found in Cryptos table.
            strcpy(List.crypto_Name, (char*)sqlite3_column_text (selectstmt, 1));     // Initializes List.crypto_Name with crypto_name found in Cryptos table.
            List.crypto_Amount = sqlite3_column_double (selectstmt, 2);               // Initializes List.crypto_Amount from crypto_amount found in Cryptos table.
            List.user_ID = sqlite3_column_int (selectstmt, 3);                        // Initializes List.user_ID with user_id found in Cryptos table.

            // Prints rows from Cryptos table for user_ID to client.
            asprintf(&returnMessage, "%d %s %.1f %d\n", List.crypto_ID, List.crypto_Name, List.crypto_Amount, List.user_ID);
            strcat(cryptoTableList, returnMessage);
          }
          send(clientSocket,cryptoTableList,100,0);
        }
        continue;
      }

      // "BALANCE" COMMAND BLOCK

      else if(strcmp(userCommand,"BALANCE\n") == 0){
        struct Arguments Bal;     // Delcaring new Arguments struct for BALANCE function.
        Bal.user_ID = 1;          // Initalizing user_ID as 1  

        // Initializes Bal user data from Users table with first_name, last_name, and usd_balance.
        asprintf(&sql, "SELECT * FROM Users WHERE ID = '%d';", Bal.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            strcpy(Bal.first_Name, (char*)sqlite3_column_text (selectstmt, 2));
            strcpy(Bal.last_Name, (char*)sqlite3_column_text (selectstmt, 3));
            Bal.user_Balance = sqlite3_column_double (selectstmt, 6);
          }
        }

        // Notifies the client of the user's account balance after SELL.
        asprintf(&returnMessage, "200 OK\nBalance for user %s %s: $%.2f\n", Bal.first_Name, Bal.last_Name, Bal.user_Balance);
        send(clientSocket,returnMessage,100,0);
        continue;
      }

      // "SHUTDOWN" COMMAND BLOCK

      else if(strcmp(commands[0],"SHUTDOWN\n") == 0){
        
        // Notifies the client that the server has shut down.
        returnMessage = "200 OK\nShutting down server...\n";
        send(clientSocket, returnMessage, 100, 0);
        shutdown_Flag = 1;    // When shutdown is called, it sets the shutdown_Flag to one, which ends the do while loop for commands into system.
      }

      // "QUIT" COMMAND BLOCK

      else if(strcmp(commands[0],"QUIT\n") == 0){
        // Notifies the client that the server has shut down.
        returnMessage = "200 OK\nQuitting client...\n";
        send(clientSocket,returnMessage,100,0);
        quit_Flag = 1;
      }
      
      // Invalid Command input by the user. Tells the user to issue another command.
      else {
        // Notifies the client to issue another command.
        returnMessage = "400 invalid command\n";
        send(clientSocket,returnMessage,100,0);
      }
    }
    quit_Flag = 0;
    send(clientSocket,"Socket closed",100,0);     // notifies user that client's socket has been closed.
    close(clientSocket);                          // closes the client socket.
  }
  printf("Shutting down server...\n");
  close(serverSocket);      // closes server socket
  sqlite3_close(DB);        // closes DB file.
  return(0);                // returns 0 for the main function. 
}