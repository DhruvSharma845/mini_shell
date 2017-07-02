#include "dshell.h"

struct builtInCommands builtComnds[]=
{
    {"exit",0},
    {"cd",1},
    {"dir",2},
    {"set",3},
    {"get",4}
};
int numOfDirs=0;

int main(int argc,char* argv[])
{


    //Extracting all the directories from PATH environment variable
    if(parsePath(dirs)==-1)
    {
        fprintf(stderr,"Error in parsing PATH environment variable.\n");
        exit(0);
    }

    while(1)
    {
        struct command_t *command=initialise_command();

        //print current directory and prompt character
        if(printPrompt()==-1)
            break;

        if((readCommand(commandLine))==-1)
            break;
        if(*commandLine=='\0')
            continue;
        //fprintf(stdout,"%s\n",commandLine);
        if(parseCommand(commandLine,command)==-1)
        {
            fprintf(stdout,"The syntax of command is incorrect.\n");
            continue;
        }
        /*struct simple_command_t* temp=command->head;
        while(temp!=NULL)
        {
            fprintf(stdout,"%d:%s,%s\n",temp->argc,temp->argv[0],temp->in);
            temp=temp->next;
        }*/

        runCommand(command);
        free(command);
    }
    return EXIT_SUCCESS;
}

/**
*  Objective : initialize the command structure
*  @return : pointer to initialized command structure object
*/
struct command_t* initialise_command()
{
        int i;
        struct command_t* newCom=(struct command_t*)malloc(sizeof(struct command_t));
        for( i=0 ; i<MAX_PIPED_CMDS ; i++)
            newCom->head[i]=NULL;
        newCom->op=0;
        newCom->numOfCmds=0;
        return newCom;
}

/**
* Objective: Parses the PATH environment variable
* @return: dirs is array of strings of paths
*         numOfDirs = total number of directories in PATH variable
*/
int parsePath(char* dirs[])
{
    char* pathEnvVar;
    int i;
    for(i=0;i<MAX_PATHS;i++)
        dirs[i]=NULL;

    pathEnvVar=(char*) malloc(TOTAL_PATH_LEN);
    if(GetEnvironmentVariable("Path",pathEnvVar,TOTAL_PATH_LEN)==0)
        return -1;
    /*
    fprintf(stdout,"The PATH environment variable is: %s\n",path_p);
    */
    /* parsing of path_p string */
    dirs[numOfDirs]=strtok(pathEnvVar,DIR_DELIMITER);

    ++numOfDirs;
    while((dirs[numOfDirs]=strtok(NULL,DIR_DELIMITER))!=NULL)
        ++numOfDirs;

    /*fprintf(stdout,"The number of directories are: %d",numOfDirs);
    for(i=0;i<numOfDirs;i++)
        fprintf(stdout,"%d:%s\n",i,dirs[i]);*/

    free(pathEnvVar);
    return 1;
}

/**
* Objective: Displays the prompt
*            Current Directory...$
*/
int printPrompt()
{
    if((GetCurrentDirectory(MAX_CWD_LEN,current_directory))==0)
    {
        fprintf(stdout,"Reading of current working directory failed.\n");
        return -1;
    }
    fprintf(stdout,"%s $",current_directory);
    return 1;

}

/**
* Objective: Takes the command from user
*            waits till the user inputs the command
* Note: type 'exit' or 'quit' to exit
*/
int readCommand(char* commandLine)
{
    if((fgets(commandLine,MAX_COMMAND_LEN,stdin))==NULL)
    {
        fprintf(stdout,"Couldn't take the input.\n");
         return -1;
    }
    // because \n is also included in the string by fgets
    commandLine[strlen(commandLine)-1]='\0';

    if(strcmp(commandLine,"exit")==0 || strcmp(commandLine,"quit")==0)
    {
         fprintf(stdout,"Good-Bye.\n");
         ExitProcess(0);
    }
    //fprintf(stdout,"The command is: %s",commandLine);
    return 1;
}

/**
*  Objective : initialize the simple command structure
*  @return : pointer to initialized simple command structure object
*/
struct simple_command_t* initialise_simple_command()
{
    int i;

    struct simple_command_t* scmd=(struct simple_command_t*) malloc(sizeof(struct simple_command_t));

    scmd->in=INVALID_HANDLE_VALUE;
    scmd->out=INVALID_HANDLE_VALUE;
    for(i=0;i<MAX_COMMAND_PARAMS;i++)
        scmd->argv[i]=NULL;
    scmd->argc=0;
    return scmd;
}
/**
* Objective: parses the command and stores it in command structures
* @return : 'command' points to the head and tail of linked list of structures of simple commands
*/
int parseCommand(char* commandLine,struct command_t* command)
{
    int argc=0;

    //fprintf(stdout,"%s\n",commandLine);
    struct simple_command_t* scmd=initialise_simple_command();

    scmd->argv[argc]=strtok(commandLine,COMMAND_DELIMITER);
    if(scmd->argv[argc]==NULL || strcmp(scmd->argv[argc],"<")==0 || strcmp(scmd->argv[argc],">")==0 || strcmp(scmd->argv[argc],"|")==0 )
       return -1;

    ++argc;

    while((scmd->argv[argc]=strtok(NULL,COMMAND_DELIMITER))!=NULL)
    {
        if(strchr(scmd->argv[argc],'*')!=NULL)
        {
            HANDLE hFind;
            WIN32_FIND_DATA ffd;
            char* szDir=(char*)malloc(sizeof(current_directory)+4);
            memcpy(szDir,current_directory,sizeof(current_directory));
            strcat(szDir,"\\");
            strcat(szDir,scmd->argv[argc]);

            hFind=FindFirstFile(szDir,&ffd);
            if(hFind==INVALID_HANDLE_VALUE)
            {
                fprintf(stdout,"Arguments Invalid.No file found in current directory.\n");
                return -1;
            }
            do
            {
                scmd->argv[argc]=ffd.cFileName;
                fprintf(stdout,"%s",scmd->argv[argc]);
                argc++;
            }while(FindNextFile(hFind,&ffd)!=0);
        }
        else if(strcmp(scmd->argv[argc],">")==0)
        {
            scmd->argv[argc]=NULL;
            char* p=strtok(NULL,COMMAND_DELIMITER);
            if(p==NULL || strcmp(p,"<")==0 || strcmp(p,">")==0 || strcmp(p,"|")==0)
                return -1;
            else
                scmd->out=p;
        }
        else if(strcmp(scmd->argv[argc],"<")==0)
        {
            scmd->argv[argc]=NULL;
            char* p=strtok(NULL,COMMAND_DELIMITER);
            if(p==NULL || strcmp(p,"<")==0 || strcmp(p,">")==0 || strcmp(p,"|")==0)
                return -1;
            else
                scmd->in=p;
        }
        else if(strcmp(scmd->argv[argc],"|")==0)
        {
            scmd->argv[argc]=NULL;
            scmd->argc=argc;
            argc=0;

            //Saving the simple command in array of simple command structures
            command->head[command->numOfCmds]=scmd;
            command->numOfCmds++;
            command->op=OP_PIPE;

            scmd=initialise_simple_command();

            scmd->argv[argc]=strtok(NULL,COMMAND_DELIMITER);
            if(scmd->argv[argc]==NULL || strcmp(scmd->argv[argc],"<")==0 || strcmp(scmd->argv[argc],">")==0 || strcmp(scmd->argv[argc],"|")==0 )
                return -1;
            ++argc;
        }
        else
            ++argc;
    }
    scmd->argc=argc;
    //Saving the simple command in array of simple command structures
    command->head[command->numOfCmds]=scmd;
    if(command->numOfCmds==0)
    {
        command->op=OP_NONE;
    }
    command->numOfCmds++;
    //fprintf(stdout,"%d",command->op);
    return 1;
}
/**
* Objective: Handles all the commands on the basis of opcodes
*            OP_NONE: simple command
*           OP_PIPE: piped commands
*/
void runCommand(struct command_t* command)
{

    switch(command->op)
    {
    case OP_NONE:
        run_simple_command(command->head[0],1);
        break;
    case OP_PIPE:
        if(recursive_pipe(command,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE)==-1)
            return;
        recursive_wait(command,1);
    default:
        fprintf(stdout,"Invalid command operation.\n");
    }
}
/**
* Objective: run simple commands with wait facility
*/
void run_simple_command(struct simple_command_t* cmd,int wait)
{
    HANDLE proc;
    DWORD dwRes;
    HANDLE _in=INVALID_HANDLE_VALUE;
    HANDLE _out=INVALID_HANDLE_VALUE;
    if(checkInBuiltCommand(cmd)==-1)
    {
        if((proc=run_simple_command_nowait(cmd,_in,_out))==INVALID_HANDLE_VALUE)
            return;
        if(wait==1 && (dwRes=WaitForSingleObject(proc,INFINITE))==WAIT_OBJECT_0)
        {
            GetExitCodeProcess(proc,&dwRes);
        }
        CloseHandle(proc);
    }
}
/**
* Objective: Tests for in-built commands
*            cd : change directory
*            dir : display files and directories
*            set : sets the value of environment variable
*            get : gets the value of environment variable
*/
int checkInBuiltCommand(struct simple_command_t* command)
{
    int i=0;
    struct builtInCommands* com=&builtComnds[i];

    while(i<BUILT_IN_COMMANDS && strcmp(com->command_name,command->argv[0])!=0)
        com=&builtComnds[++i];
    if(i==BUILT_IN_COMMANDS)
        return -1;
    switch(com->command_num)
    {
    case 0:
        ExitProcess(0);
        break;
    case 1:
        changeDirectory(command->argv[1]);
        break;
    case 2:
        displayFilesInDirectory();
        break;
    case 3:
        if(setVariable(command->argv[1])==FALSE)
            return -1;
        break;
    case 4:
        getVariable(command->argv[1]);
        break;
    }
    return 1;
}

/**
* Objective: Sets the environment variable
*  @param : Command in the form of 'set name=value' pair
*/
BOOL setVariable(char* comm)
{
    BOOL rc;
    char* nm,*val;
    char* epos=strchr(comm,'=');
    int nameLength=0,valLength=0;
    char* temp=comm;
    while(temp!=epos)
    {
        temp++;
        nameLength++;
    }
    nm=(char*)malloc(nameLength+1);
    memcpy(nm,comm,nameLength);
    nm[nameLength]='\0';

    valLength=strlen(comm)-nameLength-1;
    val=(char*)malloc(valLength+1);
    memcpy(val,comm+nameLength+1,valLength);
    val[valLength]='\0';

    //fprintf(stdout,"%s:%s\n",nm,val);
    rc=SetEnvironmentVariable(nm,val);
    return rc;
}
/**
    Objective: print the desired environment variable
    @param : environment variable name
*/
void getVariable(char* comm)
{
    char env[2048];
    GetEnvironmentVariable(comm,env,2048);
    fprintf(stdout,"%s\n",env);
}
/**
* Objective: change current directory
*          .. : goes to parent directory
*          full path name: goes to the directory specified in full path name
*/
void changeDirectory(char* newDir)
{
    int i;
    int lastIndex;
    if(strcmp(newDir,"..")==0)
    {
        for(i=0;i<strlen(current_directory);i++)
        {
            if(current_directory[i]=='\\')
                lastIndex=i;
        }

        current_directory[lastIndex]='\0';
        SetCurrentDirectory(current_directory);
    }
    else
    {
        if(SetCurrentDirectory(newDir)==FALSE)
            fprintf(stdout,"Not able to set current working directory");
    }
}
/**
* Objective : Display the listing of files in current directory
*/
void displayFilesInDirectory()
{
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    char* szDir=(char*)malloc(sizeof(current_directory)+4);
    memcpy(szDir,current_directory,sizeof(current_directory));
    strcat(szDir,"\\*");

    hFind=FindFirstFile(szDir,&ffd);
    if(hFind==INVALID_HANDLE_VALUE)
    {
        fprintf(stdout,"No file found in current directory.\n");
        return;
    }
    do
    {
        if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            fprintf(stdout,"<DIR>    %s\n",ffd.cFileName);
        else
            fprintf(stdout,"<FILE>   %s\n",ffd.cFileName);
    }while(FindNextFile(hFind,&ffd)!=0);
    fprintf(stdout,"\n");
}
/**
* Objective: Assigns the standard directives for the command
*/
HANDLE run_simple_command_nowait(struct simple_command_t* sc,HANDLE _in,HANDLE _out)
{
    HANDLE proc;
    DWORD open_mode;
    if(sc->in!=INVALID_HANDLE_VALUE)
    {
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa,sizeof(sa));

        // The handle can be inherited by child processes
        sa.bInheritHandle=TRUE;

        if(access(sc->in,F_OK)==-1)
        {
            fprintf(stdout,"File not present.\n");
            return INVALID_HANDLE_VALUE;
        }
        //             Filename      Access       Share mode
        _in=CreateFile( sc->in , GENERIC_READ, FILE_SHARE_READ , &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(_in==INVALID_HANDLE_VALUE)
        {
            fprintf(stdout,"Error in opening file: %s",sc->in);
            return INVALID_HANDLE_VALUE;
        }
    }
    if(sc->out!=INVALID_HANDLE_VALUE)
    {
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa,sizeof(sa));
        sa.bInheritHandle=TRUE;
        open_mode=OPEN_ALWAYS;

        if(access(sc->out,F_OK)==-1)
            open_mode=CREATE_ALWAYS;

        _out=CreateFile(sc->out,GENERIC_WRITE, FILE_SHARE_WRITE, &sa,open_mode, FILE_ATTRIBUTE_NORMAL, NULL);

        if(open_mode==OPEN_ALWAYS)
            SetFilePointer(sc->out,0,NULL,FILE_END);

        if(_out==INVALID_HANDLE_VALUE)
        {
            fprintf(stdout,"Error in opening file: %s",sc->out);
            return INVALID_HANDLE_VALUE;
        }
    }

    proc=executeCommand(sc,_in,_out);

    CloseHandle(_in);
    CloseHandle(_out);

    return proc;
}
/**
* Objective: Searches the command in current directory
*            If not found,then in directories present in PATH environment variable
* Output: returns 1 if file found and changes the argv[0] with full path name
*         otherwise,returns -1.
*/
int lookupPath(struct simple_command_t* command,char** dirs)
{
    int i;
    int hFind;
    char* temp;

    if(*(command->argv[0])=='/')
        return 1;
    if(access(command->argv[0],F_OK)!=-1)
    {
        //fprintf(stdout,"The file is in current directory\n");
        return 1;
    }
    temp=(char*)malloc(MAXFULLPATHNAME);
    for(i=0;i<numOfDirs-1;i++)
    {
        temp[0]='\0';
        if(strcmp(dirs[i],".") && strcmp(dirs[i],".."))
        {
            strcat(temp,dirs[i]);
            strcat(temp,"\\");
            strcat(temp,command->argv[0]);
            temp[strlen(temp)]='\0';
        }
        hFind=access(temp,F_OK);
        if(hFind!=-1)
        {
            command->argv[0]=temp;
            return 1;
        }

    }
    if(i==numOfDirs-1)
    {
        fprintf(stdout,"...command not found in any directory.\n");
        return -1;
    }
    return -1;
}



/**
* Objective: Creates the process specified in simple_command_t structure
* @return : Returns the process handle
*/
HANDLE executeCommand(struct simple_command_t* command,HANDLE in,HANDLE out)
{
    STARTUPINFO startinfo;
    PROCESS_INFORMATION processInfo;
    char* commandline;
    int i=0;
    if(!strstr(command->argv[0],".exe"))
        strcat(command->argv[0],".exe");

    if(lookupPath(command,dirs)==-1)
        return INVALID_HANDLE_VALUE;
    //GetCurrentDirectory(MAX_CWD_LEN,curDir);

    commandline=(char*) malloc(MAX_COMMAND_LEN);
    commandline[0]='\0';
    strcat(commandline,command->argv[0]);
    while(++i!=command->argc)
    {
        strcat(commandline," ");
        strcat(commandline,command->argv[i]);
    }
    commandline[strlen(commandline)]='\0';
    //fprintf(stdout,"The full command is:%s\n",commandline);

    ZeroMemory(&startinfo,sizeof(startinfo));
    startinfo.cb=sizeof(startinfo);
    ZeroMemory(&processInfo,sizeof(processInfo));

    RedirectHandle(&startinfo,in,out);

    if(!CreateProcess(NULL,commandline,NULL,NULL,TRUE,0,NULL,NULL,&startinfo,&processInfo))
    {
        fprintf(stderr, "Execution failed for '%s'\n", command->argv[0]);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(processInfo.hThread);
    return processInfo.hProcess;
}

/**
*  sets the standard directives in STARTUPINFO object
*/
void RedirectHandle(STARTUPINFO *psi, HANDLE in, HANDLE out)
{
 	psi->dwFlags = STARTF_USESTDHANDLES;
 	psi->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
 	psi->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
 	//psi->hStdError = GetStdHandle(STD_ERROR_HANDLE);

 	if (in != INVALID_HANDLE_VALUE)
 		psi->hStdInput = in;
 	if (out != INVALID_HANDLE_VALUE)
 		psi->hStdOutput = out;
}

/**
*   Create pipe for two process piped command
*   Create both the processes.
*/
int recursive_pipe(struct command_t* cmd,HANDLE in,HANDLE out)
{
    HANDLE hRead,hWrite;
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa,sizeof(sa));
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle=TRUE;

    if(CreatePipe(&hRead,&hWrite,&sa,0)==0)
        return -1;

    aux1=run_simple_command_nowait(cmd->head[0],in,hWrite);
    CloseHandle(hWrite);
    aux2=run_simple_command_nowait(cmd->head[1],hRead,out);
    CloseHandle(hRead);
    return 1;
}
/**
*  Shell process waits for both of the processes to exit
*/
void recursive_wait(struct command_t* cmd,int wait)
{
    DWORD dwRes;
    if(wait==1)
    {
        if((dwRes=WaitForSingleObject(aux1,INFINITE))!=WAIT_OBJECT_0)
            return;
        GetExitCodeProcess(aux1,&dwRes);
        CloseHandle(aux1);
        if((dwRes=WaitForSingleObject(aux2,INFINITE))!=WAIT_OBJECT_0)
            return;
        GetExitCodeProcess(aux2,&dwRes);
        CloseHandle(aux2);
    }
}

