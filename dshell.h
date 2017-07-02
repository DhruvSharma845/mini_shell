#ifndef _DSHELL_H_
#define _DSHELL_H_

#include<windows.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

#define TOTAL_PATH_LEN 2048
#define MAX_PATHS 128
#define DIR_DELIMITER ";"

#define MAX_PATH_LEN 128

#define MAX_CWD_LEN 64

#define MAX_COMMAND_LEN 256

#define MAX_COMMAND_PARAMS 16
#define PARAM_LEN 16
#define COMMAND_DELIMITER " "

#define MAXFULLPATHNAME 256
#define BUILT_IN_COMMANDS 5

#define MAX_PIPED_CMDS 2

#define OP_NONE 1
#define OP_PIPE 2
struct simple_command_t
{
    char* argv[MAX_COMMAND_PARAMS];
    int argc;
    char* in;
    char* out;
};

struct command_t
{
    int op;
    int numOfCmds;
    struct simple_command_t* head[MAX_PIPED_CMDS];
};

struct builtInCommands
{
    char* command_name;
    int command_num;
};

//Stroes the strings in PATH variable
extern int numOfDirs;
char* dirs[MAX_PATHS];
//Stores the current working directory string
char current_directory[MAX_CWD_LEN];
//Stores the whole command line string
char commandLine[MAX_COMMAND_LEN];


int parsePath(char* dirs[]);
int printPrompt();
int readCommand(char* commandLine);
int parseCommand(char* commandLine,struct command_t* command);
void runCommand(struct command_t*);

int lookupPath(struct simple_command_t* command,char**);
HANDLE executeCommand(struct simple_command_t*,HANDLE,HANDLE);

int checkInBuiltCommand(struct simple_command_t*);
void changeDirectory(char*);
BOOL setVariable(char*);
void getVariable(char*);
void displayFilesInDirectory();

void allocate_new_memory_simple_command_t(struct simple_command_t*);
//void processFileOutCommand(struct command_t*,int);

void run_simple_command(struct simple_command_t*,int);
HANDLE run_simple_command_nowait(struct simple_command_t*,HANDLE,HANDLE);

int recursive_pipe(struct command_t*,HANDLE,HANDLE);
void recursive_wait(struct command_t*,int);

void RedirectHandle(STARTUPINFO*,HANDLE,HANDLE);

struct command_t* initialise_command();
struct simple_command_t* initialise_simple_command();

HANDLE aux1,aux2;
#endif // _DSHELL_H_
