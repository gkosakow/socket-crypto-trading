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

//Callback function retrieves the contents database used by selectData function, as outlined in SQLite tutorial
//argc: holds number of results, azColName: hold each column returned in array, argv: holds each value in array.
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

static int updateData(const char *s){
    sqlite3* DB;
    char *zErrMsg;

    //Formatting of sqlite3 INSERT command for adding to Cryptos/Users table.
    char *sql = "SELECT * from Users";
    
    int rc = sqlite3_open(s, &DB);
    rc = sqlite3_exec(DB, sql, NULL, 0, &zErrMsg);
    
    //Block that checks the return code, if rc returns anything but 0, throw err or.
    if(rc != SQLITE_OK) {
      fprintf(stderr, "Error in updateData function (SQL error: %s)\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Records updated successfully\n");
    }

    sqlite3_close(DB);
    return(0);
}

struct Arguments{
  char crypto_Name[16];
  float crypto_Amount;
  float price_per_crypto;
  float crypto_Price;
  int user_ID;
  double crypto_Balance;
  double user_Balance;
};

// static int selectData(const char *s, char table, struct Arguments Transaction){
//     sqlite3* DB;
//     char *zErrMsg;
//     char *sql;

//     //Formatting of sqlite3 INSERT command for adding to Cryptos/Users table.
//     sprintf(sql, "SELECT %", );
    
//     int rc = sqlite3_open(s, &DB);
//     rc = sqlite3_exec(DB, sql, NULL, 0, &zErrMsg);
    
//     //Block that checks the return code, if rc returns anything but 0, throw err or.
//     if(rc != SQLITE_OK) {
//       fprintf(stderr, "SQL error: %s\n", zErrMsg);
//       sqlite3_free(zErrMsg);
//     } else {
//       fprintf(stdout, "Records selected successfully\n");
//     }

//     sqlite3_close(DB);
//     return(0);
// }

// static int insertData(const char *s, struct Arguments Transaction){
//     sqlite3* DB;
//     char *zErrMsg;
//     char *sql[1024];

//     Transaction.crypto_Price = (Transaction.crypto_Amount * Transaction.price_per_crypto);

//     sprintf(sql, 1024, "INSERT INTO Cryptos(crypto_name, crypto_balance, user_id) VALUES('%c', '%d', '%i');", Transaction.crypto_Name, Transaction.crypto_Balance, Transaction.user_ID);
    
//     int rc = sqlite3_open(s, &DB);
//     rc = sqlite3_exec(DB, sql, NULL, 0, &zErrMsg);
    
//     //Block that checks the return code, if rc returns anything but 0, throw err or.
//     if(rc != SQLITE_OK) {
//       fprintf(stderr, "Error in BUY function (SQL error: %s)\n", zErrMsg);
//       sqlite3_free(zErrMsg);
//     } else {
//       fprintf(stdout, "Records inserted successfully\n");
//     }

//     sqlite3_close(DB);
//     return(0);
// }

int main(int argc, char* argv[]){
    //Defining dbFile to be opened by openDB.
    sqlite3* DB;
    const char* dbFile = "cis427_crypto.db";

    int rc;
    rc = sqlite3_open(dbFile, &DB);
    //If SQLite code returns anything but 0 (SQLITE_OK), results as an error.
    if(rc){
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(DB));
        return(0);    
    }else{
        fprintf(stderr, "Opened database successfully.\n");
    }

    char userInput[100];              // Declares the string where the user's commands will be stored temporarily.
    const char separator[4] = " ";    // Declaring and initializing a separator value that will help separate user commands and arugments from their input.
    char *token;                      // Declaring pointer token to help store each command and argument into the commands[] array.
    char *commands[5];                // Declaring pointer array for 5 commands/arguments.
    int shutdown_Flag = 0;            // Delcaring and initializing shutdown_Flag to allow user to shut down the server via SHUTDOWN command.

    // Main do/while loop to allow user to input multiple SQL commands.
    do{
      // Getting user input string from stdin
      fgets(userInput, 100, stdin);
      
      // Using strtok to separate string into tokens, in which this gets the first token out of the user input string.
      token = strtok(userInput, separator);

      // Stores each individual command/argument as a token and pushes it to the commands[] array.
      int i = 0;
      while (token != 0) {
          commands[i] = token;
          printf("Command %d: %s\n", i, commands[i]); //DEBUGGER
          token = strtok(0, separator);
          i++;
          }
      
        
      char *userCommand = commands[0];

      // BUY COMMAND BLOCK

      if(strcmp(userCommand,"BUY") == 0 && i == 5){
        char *sql;
        struct Arguments Buy;

        strcpy(Buy.crypto_Name, commands[1]);
        Buy.crypto_Amount = atof(commands[2]);
        Buy.price_per_crypto = atof(commands[3]);
        Buy.user_ID = atoi(commands[4]);


        Buy.crypto_Price = (Buy.crypto_Amount * Buy.price_per_crypto);
        if(Buy.crypto_Price < 0){
          printf("Cannot have negative crypto amounts or prices!!\n");
          continue;
        }

        printf("%f",Buy.crypto_Price);

        //SELECT usd_balance FROM Users WHERE ID = 1;
        asprintf(&sql, "SELECT usd_balance FROM Users WHERE ID = '%d';", Buy.user_ID);

        int rc;
        char *zErrMsg;
        rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Operation done successfully\n");
        }

        printf("\n");
        printf("%s\n",sql);
        //
        asprintf(&sql, "INSERT INTO Cryptos(crypto_name, crypto_balance, user_id) VALUES('%s', '%f', '%i');", Buy.crypto_Name, Buy.crypto_Balance, Buy.user_ID);
        rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Operation done successfully\n");
        }
        printf("Received: BUY command\n");
        printf("crypto_Name: %s, crypto_Amount %f, price_per_crypto: %f, user_ID: %d\n", Buy.crypto_Name, Buy.crypto_Amount, Buy.price_per_crypto, Buy.user_ID); //DEBUGGER
        //BUY(dbFile, crypto_Name, crypto_Amount, price_per_crypto, user_ID);
        continue;
      }

      // "SELL" COMMAND BLOCK

      else if(strcmp(userCommand,"SELL") == 0 && i == 5){
        struct Arguments Sell;

        strcpy(Sell.crypto_Name, commands[1]);
        Sell.crypto_Amount = atof(commands[2]);
        Sell.price_per_crypto = atof(commands[3]);
        Sell.user_ID = atoi(commands[4]);

        printf("crypto_Name: %s, crypto_Amount %f, price_per_crypto: %f, user_ID: %d\n", Sell.crypto_Name, Sell.crypto_Amount, Sell.price_per_crypto, Sell.user_ID); //DEBUGGER
        printf("Received: SELL function\n");
        //SELL(dbFile, crypto_Name, crypto_Amount, price_per_crypto, user_ID);
        continue;;
      }

      // "LIST" COMMAND BLOCK

      else if(strcmp(userCommand,"LIST\n") == 0){
        printf("Received: LIST function\n");
        //LIST();
        continue;
      }

      // "BALANCE" COMMAND BLOCK

      else if(strcmp(userCommand,"BALANCE\n") == 0){
        printf("Received: BALANCE function\n");
        //BALANCE();
        continue;
      }

      // "SHUTDOWN" COMMAND BLOCK

      else if(strcmp(commands[0],"SHUTDOWN\n") == 0){
        printf("Received: SHUTDOWN function\n");
        shutdown_Flag = 1;
        continue;
      }

      // IF COMMAND IS NOT VALID

      else {
        printf("Please issue a valid command. Try again.\n");
      }
    }while(shutdown_Flag != 1);

    printf("SERVER CLOSED.\n");
    sqlite3_close(DB);
    return(0);
}