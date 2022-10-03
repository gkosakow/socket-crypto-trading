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

// callback function retrieves the contents database used by selectData function, as outlined in SQLite tutorial
// argc: holds number of results, azColName: hold each column returned in array, argv: holds each value in array.
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
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
  int user_ID;
  double crypto_Balance;
  double user_Balance;
  char first_Name[16];
  char last_Name[16];
};

int main(int argc, char* argv[]){
    // Defining dbFile to be opened by openDB.
    const char* dbFile = "cis427_crypto.sqlite";
    sqlite3* DB;
    struct sqlite3_stmt *selectstmt;

    int rc;                           // return code from sqlite3 commands, will return 0 if command issued successfully.
    char *sql;                        // a pointer sql for sql commands to be stored and used in sqlite3_exec.
    char *ErrMsg;                     // a pointer ErrMsg for sqlite3_exec commands to use. 
    char userInput[100];              // the string where the user's commands will be stored temporarily.
    const char separator[4] = " ";    // and initializing a separator value that will help separate user commands and arugments from their input.
    char *token;                      // pointer token to help store each command and argument into the commands[] array.
    char *commands[5];                // pointer array for 5 commands/arguments.
    int shutdown_Flag = 0;            // and initializing shutdown_Flag to allow user to shut down the server via SHUTDOWN command.

    // opens SQLite3 database file and if the return code returns anything but 0 (SQLITE_OK), results as an error and outputs "Cannot open db"
    rc = sqlite3_open(dbFile, &DB);
    if(rc){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(DB));
        return(0);    
    }

    // main do/while loop to allow the user to input multiple SQL commands.
    do{
      // TODO: Currently getting user input string from stdin. Needs to be changed to buffer from client.
      fgets(userInput, 100, stdin);
      printf("Received: %s", userInput);

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
        // declaring new arguments struct for Buy variables.
        struct Arguments Buy;

        strcpy(Buy.crypto_Name, commands[1]);
        Buy.crypto_Amount = atof(commands[2]);
        Buy.price_per_crypto = atof(commands[3]);
        Buy.user_ID = atoi(commands[4]);
        Buy.crypto_Price = Buy.crypto_Amount * Buy.price_per_crypto;

        // Fail BUY command when user inputs a negative value for crypto_Amount or price_per_crypto
        if(Buy.crypto_Amount <= 0 || Buy.price_per_crypto <= 0){
          printf("Cannot have negatives or zeroes crypto amounts or prices!!\n");
          continue;
        }

        // SELECT usd_balance FROM Users WHERE ID = 1 and set Buy.user_Balance to the usd_balance value data.
        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Buy.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Buy.user_Balance = sqlite3_column_double (selectstmt, 0);
            
            // IF-ELSE BLOCK THAT CHECKS IF THE USER HAS ENOUGH FUNDS.
            if(Buy.user_Balance - Buy.crypto_Price >= 0){
              asprintf(&sql, "UPDATE Users SET usd_balance = usd_balance - '%f' WHERE ID = '%d';", Buy.crypto_Price, Buy.user_ID);
              sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
            } else {
                printf("User does not have enough money.\n");
                continue;
            }
          } else {
              printf("User record does not exist.\n");
              continue;
          }
        }
        
        //only execute if user id exists and when they have enough usd_balance
        asprintf(&sql, "INSERT INTO Cryptos (crypto_name, crypto_balance, user_id) VALUES('%s', '%f', '%i');", Buy.crypto_Name, Buy.crypto_Amount, Buy.user_ID);
        rc = sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
        if( rc != SQLITE_OK ) {
          asprintf(&sql, "UPDATE Cryptos SET crypto_Balance = crypto_Balance + '%f' WHERE crypto_name = '%s';", Buy.crypto_Amount, Buy.crypto_Name);
          sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
        }

        asprintf(&sql, "SELECT crypto_balance FROM Cryptos WHERE crypto_name = '%s';", Buy.crypto_Name);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Buy.crypto_Balance = sqlite3_column_double (selectstmt, 0);
          }
        }

        printf("200 OK\nBOUGHT: New balance: %.1f %s USD balance $%.2f\n", Buy.crypto_Balance, Buy.crypto_Name, Buy.user_Balance - Buy.crypto_Price);
        continue;
      }

      // "SELL" COMMAND BLOCK

      else if(strcmp(userCommand,"SELL") == 0 && i == 5){
      // declaring new arguments struct for Buy variables.
        struct Arguments Sell;

        strcpy(Sell.crypto_Name, commands[1]);
        Sell.crypto_Amount = atof(commands[2]);
        Sell.price_per_crypto = atof(commands[3]);
        Sell.user_ID = atoi(commands[4]);
        Sell.crypto_Price = Sell.crypto_Amount * Sell.price_per_crypto;

        // Fail BUY command when user inputs a negative value for crypto_Amount or price_per_crypto
        if(Sell.crypto_Amount <= 0 || Sell.price_per_crypto <= 0){
          printf("Cannot have negatives or zeroes crypto amounts or prices!!\n");
          continue;
        }

        // SELECT crypto_balance FROM Cryptos WHERE crypto_name = "name of crypto" and set Sell.crypto_Balance to the crypto_balance value data.
        asprintf(&sql, "SELECT crypto_balance FROM Cryptos WHERE crypto_name = '%s';", Sell.crypto_Name);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Sell.crypto_Balance = sqlite3_column_double (selectstmt, 0); 

            // TODO: Change this logic to compare crypto amounts from user_id
            if(Sell.crypto_Balance - Sell.crypto_Amount >= 0){
              asprintf(&sql, "UPDATE Users SET usd_balance = usd_balance + '%f' WHERE ID = '%d';", Sell.crypto_Price, Sell.user_ID);
              rc = sqlite3_exec(DB, sql, callback, 0, &ErrMsg);
            } else {
                printf("Crypto funds insufficient.\n");
                continue;
            }
          } else {
              printf("Crypto record does not exist.\n");
              continue;
          }
        }
        
        // Removes crypto_Amount from crypto_balance SELL command from the db Cryptos table.
        asprintf(&sql, "UPDATE Cryptos SET crypto_balance = crypto_balance - '%f' WHERE crypto_name = '%s';", Sell.crypto_Amount, Sell.crypto_Name);
        sqlite3_exec(DB, sql, callback, 0, &ErrMsg);

        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Sell.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Sell.user_Balance = sqlite3_column_double (selectstmt, 0);
          }
        }
        
        printf("200 OK\nSOLD: New balance: %.1f %s USD balance $%.2f\n", Sell.crypto_Balance - Sell.crypto_Amount, Sell.crypto_Name, Sell.user_Balance);
        continue; //SELL(dbFile, crypto_Name, crypto_Amount, price_per_crypto, user_ID);
      }

      // "LIST" COMMAND BLOCK

      else if(strcmp(userCommand,"LIST\n") == 0){
        sql = "SELECT * FROM Cryptos;";
        sqlite3_exec(DB, sql, callback, 0, &ErrMsg);

        //LIST();
        continue;
      }

      // "BALANCE" COMMAND BLOCK

      else if(strcmp(userCommand,"BALANCE\n") == 0){
        struct Arguments Bal;
        Bal.user_ID = 1;

        asprintf(&sql, "SELECT first_name FROM Users WHERE ID = '%d';", Bal.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            strcpy(Bal.first_Name, (char*)sqlite3_column_text (selectstmt, 0));
          }
        }
        asprintf(&sql, "SELECT last_name FROM Users WHERE ID = '%d';", Bal.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            strcpy(Bal.last_Name, (char*)sqlite3_column_text (selectstmt, 0));
          }
        }
        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Bal.user_ID);
        rc = sqlite3_prepare_v2(DB, sql, -1, &selectstmt, NULL);
        if(rc == SQLITE_OK) {
          if (sqlite3_step(selectstmt) == SQLITE_ROW) {
            Bal.user_Balance = sqlite3_column_double (selectstmt, 0);
          }
        }

        printf("200 OK\nBalance for user %s %s: $%.2f\n", Bal.first_Name, Bal.last_Name, Bal.user_Balance);
        continue;
      }

      // "SHUTDOWN" COMMAND BLOCK
      else if(strcmp(commands[0],"SHUTDOWN\n") == 0){
        shutdown_Flag = 1;
        continue;
      }
      
      // Invalid command block
      else {
        printf("Please issue a valid command. Try again.\n");
      }
    }while(shutdown_Flag != 1);

    sqlite3_close(DB);
    printf("SERVER CLOSED.\n");
    return(0);
}