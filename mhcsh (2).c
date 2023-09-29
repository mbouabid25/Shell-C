#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h> //PATH_MAX
#include <string.h>
#include <stdbool.h> //for booleans
#include <ctype.h> //isspace
#include <sys/wait.h> //waitpid()
#include <signal.h> //pid_t

//Each string in this array is one word entered on the command line
char * myArr [255];
//String entered on the command line
char command [2047];
//Number of words entered in the command line
int numCommands;
//Boolean that indicates if a program should run in the background
bool isBackground = false;

/*
 * This function prints our command line prompt with approproate spacing.
 * This prompt contains the name of our shell (mhcsh)
 * followed by a colomn and the name of the working directory
 * and ends with an "arrow"
 */
void printPrompt()
{
  // Pathname of the current working directory
  char cwd[1024];
  // Array to store each item of the pathname sepearately
  char myPath[100][100];
  // Counter to keep track of our position in the pathname
  int counter = 0;

  // Get current working directoy pathname
  getcwd(cwd, sizeof(cwd));
  // Start tokenizing the pathname using a slash as the seperator
  char * token = strtok(cwd, "/");

  // Tokenize the rest of the pathname
  while(token != NULL ) {
      // Add each component of the pathname to an array
      strcpy(myPath[counter], token);
      token = strtok(NULL, "/");
      // Increment counter every time new item is added to array
      counter++;
   }

  // Set the PWD variable to the current working directoy
  setenv("PWD", getcwd(cwd, sizeof(cwd)), 1);
  // Print desired prompt (I added the work directory because I liked it better!)
  printf("mhcsh> ");
}

/* This function checks for a new line character and gets rid of it if there is
 * one. We use this function to make our string comparaisons easier, because fgets
 * often gets user input with a new line character at the end.
 * The function takes a pointer to a char array as a parameter and returns the
 * same pointer
 */
char * deleteNewLine(char * arr) {
  // Find the length of the string
  int ln = strlen (arr);
  // If the string has at least one character and ends with a new line char
  if ((ln > 0) && (arr[ln-1] == '\n')) {
      // Get rid of the new line char
      arr[ln-1] = '\0';
    }
    // Return the desired string
    return arr;
}

/* This function tokenizes our commands into seperate words and adds them individually
 * to an array of strings. This makes the process of handling commands with multiple
 * arguments easier
 */
int tokenize(char * toTokenize) {
  // Allocate space for the tokens
  //myArr = malloc(sizeof(toTokenize) * 50);
  // Counter to keep track of the position in the array
  int counter = 0;
  // Tokenize the first element
  char * token = strtok(toTokenize, " ");

  // Tokenize the rest of the elements with this loop
  while(token != NULL ) {
    if((strcmp(token, " ") !=0)) {
      myArr[counter] = token;
      // Make sure to add the tokens to the array without new line char
      deleteNewLine(myArr[counter]);
      token = strtok(NULL, " ");
      //Go to next element
      counter++;
    }
    else{
      token = strtok(NULL, " ");
      counter++;
    }
   }

   myArr[counter] = NULL;

   //If the string ends with an ampersand
   if(strcmp(myArr[counter-1], "&") == 0) {
     //Get rid of the ampersand
     myArr[counter-1] = NULL;
     //Set boolean for running in the background to true
     isBackground = true;
   }

  //Return position in the array
  return counter;
}

/* This function takes user input and handles it appropriately
 */
int takeUserInput() {
  int i = 2;
  if(fgets(command, sizeof(command), stdin) == NULL) {
    i = 1;
    exit(0);
  }
  else{
    i = 0;
  }
  numCommands = tokenize(command);
  return i;
}

/* This boolean indicates if the command passed is built in. It returns true if it is
 */
bool isBuiltIn() {
  if (strcmp(command, "exit") == 0 || strcmp(command, "set") == 0 || strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0) {
    return true;
  }
  else {
  return false;
  }
}

/* This boolean indicates if the command passed is blank (one or multiple whitespaces)
 */
bool isBlank(char c[]) {
  //Iterates through the characters of the command
  for (int i = 0; i < strlen(c); i++) {
    //If one of the characters is not a whitespace return false
    if (isspace(c[i]) == 0) {
      return false;
    }
    }
    //If all characters are whitespaces return true
    return true;
  }

/* This function checks if the command passed is built is valid. It returns true if it is
 */
bool isValid() {
  //If the command is an empty string return false
  if(strcmp(command, "") == 0) {
    return false;
  }
  //If the command is whitespaces return false
  else if (isBlank(deleteNewLine(command)) == true) {
    return false;
  }
    return true;
  }

/* This function exits the shell
 */
void exitShell() {
  //If no arguments are passed in
  if(numCommands == 1) {
    //Exit with value 0
    exit(0);
    }
    //check for when the user wants to exit the shell with an argument
  else if(numCommands == 2) {
    exit(atoi(myArr[1]));
    }
}

/* This function prints the current working directory pathname
 */
void pwd() {
  //Throw error if there is an incorrect number of arguments
  if (numCommands != 1) {
    fprintf(stderr,"pwd : too many arguments! \n");
    //Continue execution
    printPrompt();
  }
  else {
  //Print current working drectory pathname
  char cwd[PATH_MAX];
  printf("%s \n", getcwd(cwd, sizeof(cwd)));
  //Continue execution
  printPrompt();
  }
}

/* This function sets environemnent variables
 */
void set() {
  //Throw error if there is an incorrect number of arguments
  if (numCommands != 3) {
    fprintf(stderr,"set : command should take two arguments! \n");
    //Continue execution
    printPrompt();
  }
  else {
  //Set enviornment variable with arguments passed in
  int setenvSucc = setenv(myArr[1],myArr[2],1);
  //Throw error if set fails
  if (setenvSucc != 0) {
    fprintf(stderr,"Error : Unable to set environemnent variable! \n");
  }
  //Continue execution
  printPrompt();
  }
}

/* This function enables to user to change the current working directory
 */
void cd() {
  //If appropriate number of arguments are passed in execute
  if (numCommands == 1 || numCommands == 2) {
    //If no arguments are passed in
    if (numCommands == 1) {
      //Go to home directory
      chdir(getenv("HOME"));
      printPrompt();
    }
    else {
      //If one argument is passed in go to appropriate directory
      int chdirSucc = chdir(myArr[1]);
      //Throw error if no directory can be found
      if (chdirSucc != 0) {
        fprintf(stderr,"Error : Unable to find directoy! \n");
      }
      printPrompt();
    }
  }
  //Throw error if too many arguments are passed in
  else {
    fprintf(stderr,"cd : Too many arguments! \n");
    printPrompt();
  }
}

/* This function executes the appropriate built-in command
 */
void executeBuiltIn() {
  //If the command is exit
  if(strcmp(myArr[0], "exit") == 0) {
    //Call exitShell
    exitShell();
}
  //If the command is set
  if (strcmp(myArr[0], "set") == 0) {
    //Call set
    set();
  }
  //If the command is cd
  if (strcmp(myArr[0], "cd") == 0) {
    //Call cd
    cd();
  }
  //If the command is pwd
  if (strcmp(myArr[0], "pwd") == 0) {
    //Call pwd
    pwd();
  }
}

/* This function executes the appropriate external programs
 */
void executeExternal() {
  pid_t pid, wpid;

  //Fork
  pid = fork();
  //If fork is successful, replace chilf with the desired program
  if (pid == 0) {
    if (execvp(myArr[0], myArr) == -1) {
      //Throw error if exec fails or no file is found on path
      fprintf(stderr,"External program couldn't be executed/no such file on path\n");
      exit(0);
      printPrompt();
    }
    //Throw error if fork fails
  } else if (pid < 0) {
    fprintf(stderr,"Error forking \n");
    printPrompt();
  }
    else {
    //Return shell if command runs in background
    if (isBackground == true) {
        printPrompt();
    }
    //Return shell after child is done (wait) if not in the background
    else if (isBackground == false){
      printPrompt();
      wpid = waitpid(pid, NULL, 0);
    }
  }
}

/* Main function
 */
int main (int argc, char * argv[]) {
  //If too many arguments are passed, throw error message and exit shell
  if (argc != 1) {
    fprintf(stderr,"/mhcsh : too many arguments)! \n");
    exit(1);
  }

  else {
  //Print prompt
  printPrompt();

      //Infinite loop
      while(1) {
        //If stdinn ends
        if(takeUserInput() == 1) {
          //Exit the shell
          exitShell();
        }
        //If the command is valid and built in execute one of the built in commands
        if (isValid() && isBuiltIn()) {
          executeBuiltIn();
        }
        //If the command is valid and not built in execute external programs
        else if (isValid() && !isBuiltIn()) {
          executeExternal();
        }
        //If the command is invalid don't do anything and continue
        else{
          printPrompt();
        }
      }
    }

//Return
return 1;

}
