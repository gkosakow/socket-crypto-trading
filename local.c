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

struct Arguments{
  char crypto_Name[16];
  float crypto_Amount;
  float price_per_crypto;
  float crypto_Price;
  int user_ID;
  double crypto_Balance;
  double user_Balance;
};

int main(int argc, char* argv[]){
    //Defining dbFile to be opened by openDB.
    sqlite3* DB;
    const char* dbFile = "cis427_crypto.sqlite";

    // Opens SQLite3 database file and if the return code returns anything but 0 (SQLITE_OK), results as an error and outputs "Cannot open db"
    int rc;

    // TEST IF WORKS
    char *sql;
    char *zErrMsg;

    rc = sqlite3_open(dbFile, &DB);
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

    // Main do/while loop to allow the user to input multiple SQL commands.
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
        struct Arguments Buy;

        strcpy(Buy.crypto_Name, commands[1]);
        Buy.crypto_Amount = atof(commands[2]);
        Buy.price_per_crypto = atof(commands[3]);
        Buy.user_ID = atoi(commands[4]);


        Buy.crypto_Price = Buy.crypto_Amount * Buy.price_per_crypto;
        if(Buy.crypto_Amount < 0 || Buy.price_per_crypto < 0){
          printf("Cannot have negative crypto amounts or prices!!\n");
          continue;
        }
        printf("Buy.crypto_Price: %f\n",Buy.crypto_Price); //DEBUGGER


        //SELECT usd_balance FROM Users WHERE ID = 1;
        asprintf(&sql, "UPDATE SET user_Balance = user_Balance ", Buy.user_ID);
        
        rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            printf("User does not exist...\n");
            sqlite3_free(zErrMsg);
            continue;
        } else {
            fprintf(stdout, "Operation done successfully\n");
        }
        

        printf("\n");
        printf("%s\n",sql);
        
        //only execute if user id exists and when they have enough usd_balance
        asprintf(&sql, "INSERT INTO Cryptos(crypto_name, crypto_balance, user_id) VALUES('%s', '%f', '%i') ON CONFLICT(crypto_Name) DO UPDATE SET crypto_Balance = crypto_Balance + %f;", Buy.crypto_Name, Buy.crypto_Price, Buy.user_ID, Buy.crypto_Price);
        

        rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Insertion done successfully\n");
        }

        // asprintf(&sql, "UPDATE Cryptos SET crypto_Balance = crypto_Balance + %f WHERE crypto_Name = '%s';", Buy.crypto_Price, Buy.crypto_Name);
        // rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        // if( rc != SQLITE_OK ) {
        //     fprintf(stderr, "SQL error: %s\n", zErrMsg);
        //     sqlite3_free(zErrMsg);
        // } else {
        //     fprintf(stdout, "Update done successfully\n");
        // }

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
        sql = "SELECT * FROM Cryptos;";
        sqlite3_exec(DB, sql, callback, 0, &zErrMsg);

        //LIST();
        continue;
      }

      // "BALANCE" COMMAND BLOCK

      else if(strcmp(userCommand,"BALANCE\n") == 0){
        sql = "SELECT usd_balance FROM Users WHERE ID = '1';\n";
        printf("%s",sql);
        
        rc = sqlite3_exec(DB, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            printf("User does not exist...\n");
            sqlite3_free(zErrMsg);
            continue;
        } else {
            fprintf(stdout, "Operation done successfully\n");
        }

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