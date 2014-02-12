/**
 * Project 1 - Shell 
 * Team: Mostly Harmless - Jack Burns and Joana Vukatana
 */

#include "3600sh.h"

#define USE(x) (x) = (x)
#define MAX_LEN 200

/*
An array list of booleans to describe what mode the process is in:
mode[0] : INVALID_SYNTAX
mode[1] : INVALID_ESCAPE
mode[2] : BACKGROUND
mode[3] : TERMINATE (Found EOF)
mode[4] : STDIN_redirection
mode[5] : STDOUT_redirection
mode[6] : STDERR_redirection
mode[7] : Unsupported_redirection
*/
typedef char bool;
bool mode[7];

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* This function prints the prompts */
void prompt();

/* This function reads and executes the inputs */
void execute();

/* This function parses the input, and it returns an array list of strings */
char** getArgs(char** file);

/* This function handles the ampersands */
void ampersand(char* ch);

/* This function stores an argument(each arg has a space of size 4) to the array, handling file redirection requests */
void storeArgs(char *arg, char ***arguments, int *count, int *space, char **file);


int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 

  // Main loop that reads a command and executes it
  while (1) {
  // Getting the current working directory 
  char *cwd;
  cwd = (char *) getcwd(cwd, 300);
  // Issuing the prompt 
  prompt();
  // Reading and executing the prompt
  execute();
  }
  return 0;
}

void prompt(char *cwd){
  struct passwd *pd;
  pd = getpwuid(getuid());

  char hostname[1024];
  hostname[1023] = '\0';
  // Getting the hostname
  gethostname(hostname, 1023);
  // Printing the prompt with user name, hostname and directory name */ 
  printf("%s@%s:%s> ", pd->pw_name, hostname, cwd);
}

void execute() {
        /* allocating size of file */
        char  **file = calloc(3, sizeof(char*));
  char  **arguments = getArgs(file);  
    if (!file) {
       do_exit();
    }
  for (int i = 0; i < 3; i++) {
      file[i] = calloc(101, sizeof(char));
      if (!file[i]) {
          do_exit();
      }
  }
  if (mode[0] | mode[1]) {
  // Checking if the error is Invalid syntax
    if (mode[0]) {
    printf("Error: Invalid syntax.\n");
    if (mode[3]) // it is checking if it's terminating
      do_exit();      
    return;
  }
  // Checking if the error is Invalid escape
  if (mode[1]) {
   printf("Error: Unrecognized escape sequence.\n");
   if (mode[3])
    do_exit();      
    return;
  }
}
  /* If the user inputs "exit", we exit the shell */
  if (arguments[0]) {
    if (arguments[1] == NULL && strcmp(arguments[0], "exit") == 0) {
    do_exit();
    }
  }
  int openFile = -1;
  /* check if any redirection was requested and set up proper files */
  if (mode[4]) { 
  // Opening a file as read only, without creating a new one
  // since it is just an input source
    openFile = open(file[STDIN],O_RDONLY);
  // this is an unsupported redirection 
    if (openFile == -1) { 
      printf("Error: Unable to open redirection file.\n");
  // if EOF is found, then exit
      if (mode[3]) { 
         do_exit();
      }
  // otherwise keep printing the next prompt
      return;
  }
  close(STDIN); // close STDIN
  } 

  if (mode[5]) { 
    openFile = open(file[STDOUT],O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
  // unsupported redirection 
    if (openFile == -1) { 
      printf("Error: Invalid syntax.\n");
  // if EOF is found, then exit
      if (mode[3]) { 
         do_exit();
      }
      return; 
  } 
  close(STDOUT); // close STDOUT
  }
  if (mode[6]) {
    openFile = open(file[STDERR],O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR); 
    if (openFile == -1) { // unsupported redirection
      printf("Error: Invalid syntax.\n");
      if (mode[3]) {
         do_exit();
      }
      return;
    } 
  close(STDERR); // close STDERR
  }
  
  pid_t parent = fork();  
  if (parent < 0) {
    printf("Error: process creation failed");  
    do_exit();
  }

if (!parent) {
    // if this is not the parent process, execute the following commands
    if (execvp(arguments[0], arguments)) {
            if(arguments[0]) {
              if(errno == 1){
                printf("Error: Permission denied.\n");
              }
              if(errno == 2){
                printf("Error: Command not found.\n");
              }
              if(errno == 13){
                printf("Error: Permission denied.\n");
              }
          }
    }
    exit(0);
  } else {
  // if this is not a background process, the parent is waiting for the child
  if ((mode[2] != 1)) {  
         waitpid(parent, NULL, 0);
  }
      if (mode[3]) {
           do_exit();
    }
  }


  /* free all the memory that has been allocated */
  for (int i = 0; i < 3; i++) {
    free(file[i]);
  }
  free(file);
  free(arguments);
}

char** getArgs(char** file) {
    char **arguments = (char**)calloc(5, sizeof(char*));
  if (!arguments) {
     do_exit();
  }
  int temp = 0;
  int count = 1;
  char ch = getc(stdin); 
  while (ch == ' ' || ch == '\t') {
    /* remove leading spaces */
    ch = getc(stdin);
  }
  char* cmd_1 = (char*)calloc(11, sizeof(char));
  int stringBlock = 1;
  int strLen = 0;

  /* eliminating whitespace */
  while (ch != '\n' && ch != EOF) {
    if (ch == '\t' || ch == ' ') {
      while (ch == '\t' || ch == ' ') {
        ch = getc(stdin);
      }
      cmd_1[strLen] = 0;
      storeArgs(cmd_1, &arguments, &temp, &count, file);
      stringBlock = 1;
      strLen = 0;
      cmd_1 = (char*)calloc(11, sizeof(char));
      if (!cmd_1) {
         do_exit();
      }
      continue;
    }

    /* handling the escape characters */
    if (ch == '\\') {
      ch = getc(stdin);
      switch(ch) {
        case '\\':
        case ' ' :
        case '&':
          break;
        case 't':
          ch = '\t';
          break;
        case 'n':
          ch = '\n';
          continue;
        default:
          mode[1] = 1;
      }
      cmd_1[strLen] = ch;
      strLen += 1;
      ch = getc(stdin);
      continue;
    }
    if (ch == '&' && ~(mode[2])) { 
      if (mode[2])
          mode[0] = 1;
      ampersand(&ch); 
      if (mode[2])
        break;
    }

    /* we reallocate more characters if the string has nomore space */
    if (strLen == 10 * stringBlock) {
      stringBlock += 1;
      cmd_1 = (char*)realloc(cmd_1, 10 * stringBlock * 4 * sizeof(char*));
      if(!cmd_1) {
      do_exit();
      }
    }

    /* we are adding the character to the string, and incrementing the stringLength by 1 */
    cmd_1[strLen] = ch;
    strLen += 1;
    ch = getc(stdin);
  }
  /* if the string is not followed by white space, then we store them */
  if (strLen) {
    cmd_1[strLen] = 0;
    storeArgs(cmd_1, &arguments, &temp, &count, file);
  }
  
  /* if we find EOF, we terminate the process */
  if (ch == EOF) {
       mode[3] = 1;
  }
  return arguments;
}



void storeArgs(char* arg, char*** arguments, int* count, int* space, char** file) {
  /* checking if it is an unsupported redirection */
  if (mode[7]) {
            if (strcmp(arg, "<") == 0 || strcmp(arg, ">") == 0 ||strcmp(arg, "2>") == 0) {
  mode[0] = 1;
  }
  file[-1] = arg;
  mode[7] = 1;
        }
  /* checking for redirection commands */
  if (strcmp(arg, "<") == 0) {
      mode[4] = 1; // we are setting STDIN_redirection to True
      mode[7] = 1;
   }  
  if (strcmp(arg, ">") == 0) {
      mode[5] = 1; // we are setting STDOUT_redirection to True
      mode[7] = 1;
   } 

  if (strcmp(arg, "2>") == 0) {
      mode[6] = 1; // we are setting STDERR_redirection to True
      mode[7] = 1;        
   } 
  else {
    /* checking if we have reached end of file, and if so, we reallocate arguments */
                if (*count == (*space * 4)) { 
                   (*space) += 1;
                    *arguments = (char**)realloc(*arguments, *space * 4 * sizeof(char*));
    if (!arguments) {
    do_exit();
  }
   }
    /* adding a argument to the array list of arguments and incrementing count by 1 */
                (*arguments)[*count] = arg;
                (*count) += 1;
      }
}

void ampersand(char* string) {
/* if it's a whitespace, skip it, otherwise it is a INVALID_SYNTAX error */
   char ch = getc(stdin); 
   while(ch == ' ' || ch == '\t') {
  ch = getc(stdin);
   }
   if (ch != '\n' && ch != EOF) {
  mode[0] = 1;
   }
   else {
  mode[2] = 1;
   *string = ch;
   }

}

void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
