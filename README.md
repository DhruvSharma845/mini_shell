# mini_shell
It is interactive text-based command interpreter for windows written in c language.
It uses Win32 API to invoke Windows OS services.

Features:
1.It can run any executable file (.exe file).
  If relative pathname is used, then the file will be searched in current working directory.If not found,then it will be searched in
  all of the directories specified in PATH environment variable.
2.It has some in-built commands:
    exit/quit
    cd: to change the current working directory
    dir: to list filenames and directory names present in the current working directory.
    set var_name=var_value: sets the value of environment variable
    get var_name: gets the value of specified enivronment variable
3.We can redirect the standard input and output of the new commands.
  > file_name : changes the standard output
  < file_name : changes the standard input
4.It can execute two process piped command i.e. two commands connected by anonymous pipe.
