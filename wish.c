#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

char* paths[10];
char* userInput = NULL;
size_t linecap = 0;
char error_message[30] = "An error has occurred\n";

char* checkRedirection(char* input){
    
    char * command;
    char* file;
    if(input != NULL){

        int len = strlen(input);
        int found = 0;
        for (int i = 0; i < len; i++){
            if (input[i] == '>'){
                found++;
            }
        }

        if (found == 0){
            return input;
        }
        if (found > 1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        // attempting to split the arguments by the > char
        // command will be the args before >
        char* inputCpy = strdup(input);

        if((command = strtok(input, ">")) != NULL){

            // file will be the args after the >
            file = strtok(NULL, ">");
            if (file == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            char* fullFile = NULL;
            strtok_r(inputCpy, ">", &fullFile);
            char* fileCpy = strdup(fullFile);
            char* fileCpyPtr = NULL;
            fileCpyPtr = strtok(fileCpy, " ");
            fileCpyPtr = strtok(NULL, " ");

            if (fileCpyPtr != NULL){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }

            if (file != NULL){

                char* noSpaceFile = strtok(file, " ");
                if (noSpaceFile != NULL){
                    // opens file
                    int fd =  open(noSpaceFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd < 0){
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                        exit(1);
                    }
                    // redirecting stdout and stderr to file
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    if (close(fd) < 0){
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                        exit(1);
                    }
                    // returns remaning args between exec name and > char
                    strsep(&command, " ");
                    return(command);
            
                }else {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
            }
        }   
    }   
    return input;
}

void runExec(char* command, char* args){
    
    int foundBinary = 0;
    int i = 0;
    char* currentPath = NULL;
    char* redirection = NULL;
    char* finalPath = NULL;
    if (args != NULL){

        char* fullCommand = malloc(strlen(command) + strlen(args) + 2);
        if (fullCommand == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }

        sprintf(fullCommand, "%s %s", command, args);
        
        if (args != NULL){
            // redirection check if the ">" char is in the command
            // if it is then it sets up redirection and returns
            // the arguments prior to the > char 
            redirection = checkRedirection(fullCommand);
              // NULL result means > was found but no arguments after the exe
            if (redirection == NULL){
                args = NULL;
                // if > is not found, then redirection returns the given arguments
            }else if(strcmp(redirection, fullCommand) != 0){
                args = redirection;
            }
        }
    }   
    
    while((i < 10) & (foundBinary == 0) & (paths[i] != NULL) ){
         
        // searching paths in paths[] for ls binary
        currentPath = malloc(sizeof(paths[i] + strlen(command) + 2)); 
        if (currentPath == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        strcpy(currentPath, paths[i]);
        strcat(currentPath, "/");
        strcat(currentPath, command);
        // checks if binary is found
        if (access(currentPath, X_OK) >= 0){ 
            foundBinary = 1;
            finalPath = currentPath;
        }else {
            i++;
        }   
    }   

    if (foundBinary == 0){ 
        write(STDERR_FILENO, error_message, strlen(error_message)); 

    } else{

        char* arglist[10];
        arglist[0] = strdup(command);
        if (arglist[0] == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message));    
        }

        i = 1;
        char* currentArg;
        // iterates through and adds arguments for the exe
        if (args != NULL){
            // parsing arguments
            currentArg = strtok(args, " ");
            if (currentArg != NULL){
                arglist[i] = strdup(currentArg);
                if (arglist[i] == NULL){
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }
                i++;
            }
            // parsing arguments
            while((currentArg = strtok(NULL," ")) != NULL){
                arglist[i] = strdup(currentArg);
                if (arglist[i] == NULL){
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }
                i++;
            }
        }
        //sets the last arg to NULL
        arglist[i] = NULL;

        if ((execv(finalPath, arglist)) < 0){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
    }
    exit(1);
}

void loopCommand(char* command){
 
    if (command == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    char* addrSave = NULL;
    char* currArg = strtok_r(command, " ", &addrSave);
    char* extraStr = NULL;
    // parsing the string number to an int
    int loop = strtol(currArg, &extraStr ,10);
    if (loop <= 0){
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    char* args[10];
    char* cmdCpy = strdup(addrSave);
    char* exec = NULL;
    exec = strtok(cmdCpy, " ");
    int loopIndex = -1;
    if (strcmp(exec, "$loop") == 0){
        loopIndex = 0;
    }

    int totalSize = 0;
    int numArgs = 0;
    int i = 1;
    while((currArg = strtok(NULL, " ")) != NULL){

        if (strcmp(currArg, "$loop") == 0){
                loopIndex = i;
        }
        args[i] = strdup(currArg);
        totalSize = totalSize + (strlen(currArg) + 1);
        numArgs++;
        if (args[i] == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            }
        i++;
    }
    totalSize++;
    
    int currLoopNum = 1;
    char* currLoopStr = malloc(20);

    for(int i = 1; i < (loop+1); i++){
            
        // updates the $loop var if specified
        if (loopIndex >= 0){
            sprintf(currLoopStr, "%d", currLoopNum);       
            args[loopIndex] = strdup(currLoopStr);
            if (args[loopIndex] == NULL){
                write(STDERR_FILENO, error_message, strlen(error_message)); 
            }
            if (loopIndex == 0){
                exec = strdup(currLoopStr);
            }
            currLoopNum = currLoopNum + 1;
        }

        char* execvArgs = malloc(totalSize + 2);
        for (int i = 0; i < numArgs; i++){
            char* currArgCpy = strdup(execvArgs);
            sprintf(execvArgs, "%s %s", currArgCpy, args[i+1]);
        }
        int rc = fork();

        if (rc == 0){
            runExec(exec, execvArgs);
        }else if (rc > 0){
            wait(NULL);
        }else{
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

int main(int argc,char* argv[]){
    
    paths[0] = strdup("/bin");
    if (paths[0] == NULL){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    char* sepCommand;
    int rc;
    int mode = 0;
    
    if (argv[1] != NULL){
        if (argv[2] != NULL){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            exit(1);
        }
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        if (close(fd) < 0){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        mode = 1;
    }

    while(1){
        
        if (mode == 0){    
            printf("> ");
        }

        if (getline(&userInput, &linecap, stdin) >= 0){

            char* inputCpy = strdup(userInput);
            char* checkEmpty = strtok(inputCpy, " ");

            if ((strcmp(userInput, "\n") != 0) & (strcmp(checkEmpty, "\n") != 0)){
                // removing newline char from input
                char* editInput = malloc(strlen(userInput));
                if (editInput == NULL){
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                }
                snprintf(editInput, strlen(userInput), "%s", userInput);
                char* inputCopy1 = strdup(editInput);
                char* saveAddr = NULL;
                strtok_r(inputCopy1, " ", &saveAddr);
                sepCommand = strtok(editInput, " ");
                char* addrCpy = strdup(saveAddr);
                editInput = saveAddr;
          
                if (strtok(addrCpy, " ") == NULL){
                    editInput = NULL; 
                } else if (saveAddr != NULL){
                    int i = 0;
                    while(saveAddr[i] == ' '){
                        editInput = &saveAddr[i+1];
                        i++;
                    }
                }
          
                if (strcmp(sepCommand, "exit") == 0){
               
                    if (editInput != NULL){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }else{
                        wait(NULL);
                        exit(0);
                    }

                } else if(strcmp(sepCommand, "cd") == 0){
                
                    int runCommand = 1;
                    if (editInput == NULL){
                        runCommand = 0;
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                    }else {
 
                        char* newDir = strtok(editInput, " ");
                        if (newDir == NULL){
                            write(STDERR_FILENO, error_message, strlen(error_message)); 
                            runCommand = 0;
                        }      
                        if ((runCommand = 1) & (strtok(NULL, " ") != NULL)){
                            write(STDERR_FILENO, error_message, strlen(error_message)); 
                            runCommand = 0;
                        }
                        if (runCommand == 1){
                        
                            int ret = chdir(newDir);
                            if(ret < 0){
                            write(STDERR_FILENO, error_message, strlen(error_message)); 
                        }
                    }
                }

                }else if (strcmp(sepCommand, "path") == 0){
                    // Removing all old paths
                    for(int i = 0; i < 10; i++){
                        paths[i] = NULL;
                    }

                    char* currentArg = NULL;
                    int i = 0;
                    // checking argument is non null   
                    if (editInput != NULL){
                        currentArg = strtok(editInput, " ");
                        // adding new paths
                        while(currentArg != NULL){
                            paths[i] = strdup(currentArg);
                            if (paths[i] == NULL){
                                write(STDERR_FILENO, error_message, strlen(error_message)); 
                            }   
                        
                            currentArg = strtok(NULL, " ");
                            i++;
                        }
                    }
         
                }else if (strcmp(sepCommand, "loop") == 0){
                    loopCommand(editInput);
                
                }else{
                
                    rc = fork();

                    if (rc == 0){
                        runExec(sepCommand, editInput);
                    }else if(rc < 0){
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                    }else{
                        wait(NULL);
                    }
                }
            }

        } else{
            exit(0);
        }
    }

}
