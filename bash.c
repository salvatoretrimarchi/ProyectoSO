// C Program to design a shell in Linux 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <fcntl.h>
  
#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported  
#define TAM 60
// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J")   
// Greeting shell during startup 
char ** cmdGlobal;


void init_shell() 
{ 
    clear(); 
    printf("\n\n\n\n******************"
        "************************"); 
    printf("\n\n\n\t****MY SHELL****"); 
    printf("\n\n\t-USE AT YOUR OWN RISK-"); 
    printf("\n\n\n\n*******************"
        "***********************"); 
    char* username = getenv("USER"); 
    printf("\n\n\nUSER is: @%s", username); 
    printf("\n"); 
    sleep(1); 
    clear(); 
} 
void redirSal(char cad[TAM]){
    char *cadPtr;
    cadPtr=cad;//puntero a la cadena
    close(1);//cerramos la salida estándar
    open(cadPtr,O_CREAT | O_WRONLY,0777);//Se asigna la salida al fichero, tambien se le asignan permisos totales

}
void redirEnt(char cad[TAM]){
    char *cadPtr;
    int f;
    cadPtr = cad; //puntero a la cadena 
    f=open(cadPtr,O_RDONLY); // se asigna la salida al fichero
    close(0); //cerramos la salida estándar
    dup(f);
}

// Function to take input 
int takeInput(char* str) 
{ 
    char* buf; 
    buf = readline(">>> "); 
    if (strlen(buf) != 0) { 
        add_history(buf); 
        strcpy(str, buf); 
        return 0; 
    } else { 
        return 1; 
    } 
} 

// Function to print Current Directory. 
void printDir() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\nDir: %s", cwd); 
} 

// Function where the system command is executed 
void execArgs(char** parsed) 
{ 
    // Forking a child 
    pid_t pid = fork();  
    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else { 
        // waiting for child to terminate 
        wait(NULL);  
        return; 
    } 
} 

// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 

    } else { 
        // Parent executing 
        p2 = fork(); 
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 

void pipeline(char ***cmd)
{
    int fd[2];
    pid_t pid;
    int fdd = 0;                /* Backup */

    while (*cmd != NULL) {
        pipe(fd);               /* Sharing bidiflow */
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {
            dup2(fdd, 0);
            if (*(cmd + 1) != NULL) {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            execvp((*cmd)[0], *cmd);
            exit(1);
        }
        else {
            wait(NULL);         /* Collect childs */
            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}

// Help command builtin 
void openHelp() 
{ 
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Suprotik Dey"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling"); 
    return; 
} 

// Function to execute builtin commands 
int ownCmdHandler(char** parsed) 
{ 
    int NoOfOwnCmds = 4, i, switchOwnArg = 0; 
    char* ListOfOwnCmds[NoOfOwnCmds]; 
    char* username; 
    ListOfOwnCmds[0] = "exit"; 
    ListOfOwnCmds[1] = "cd"; 
    ListOfOwnCmds[2] = "help"; 
    ListOfOwnCmds[3] = "hello"; 
    for (i = 0; i < NoOfOwnCmds; i++) { 
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
            switchOwnArg = i + 1; 
            break; 
        } 
    } 
    switch (switchOwnArg) { 
    case 1: 
        printf("\nGoodbye\n"); 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        openHelp(); 
        return 1; 
    case 4: 
        username = getenv("USER"); 
        printf("\nHello %s.\nMind that this is "
            "not a place to play around."
            "\nUse help to know more..\n", 
            username); 
        return 1; 
    default: 
        break; 
    } 
    return 0; 
} 

// function for finding pipe 
int parsePipe(char* str, char** strpiped) 
{ 
    int i = 0; 
   /* for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } */

    while( (strpiped[i] = strsep(&str,"|")) != NULL ){

       // printf("%s\n",strpiped[i]);
        i++;
    }

    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return i; 
    } 
} 

// function for parsing command words 
void parseSpace(char* str, char** parsed) 
{ 
    int i;
    char *aux; 
    for (i = 0; i < MAXLIST; i++) { 
        aux = strsep(&str, " "); 
        if (aux == NULL){ 
            parsed[i] = aux;
         //   printf("\n----%s\n", parsed[i]);
            //reurn parsed; 
            break;
        }
        else if (strlen(aux) == 0)
            i--; 
        else{
            parsed[i] = aux;
         //   printf("\n----%s\n", parsed[i]);
        }
    }
} 

char ** parseSpacePipes(char* str, char** parsed) 
{ 
    char *aux;
    char **parsed2;
    parsed2 = (char**)malloc(sizeof(char*) * 100); 
    for (int i = 0; i < MAXLIST; i++) { 
        aux = strsep(&str, " "); 
        if (aux == NULL){ 
            parsed[i] = aux;
            parsed2[i] = aux;
           // printf("\n----%s\n", parsed2[i]);
            return parsed2; 
        }
        else if (strlen(aux) == 0)
            i--; 
        else{
            parsed[i] = aux;
            parsed2[i] = aux;
           // printf("\n----%s\n", parsed2[i]);
        }
    }
} 



int processString(char* str, char** parsed, char** parsedpipe) 
{ 
    char* strpiped[100]; 
    int piped = 0, i = 0, y, k = 0, ejecutar = 0; 
    piped = parsePipe(str, strpiped); 
    char entrada[MAXLIST], salida[TAM];

    
   // printf("\nse detectaron %d tuberias\n",piped);



    char ** cmd[(piped+1)];

    //char ** cmdAux[piped+1];

/*
    char *ls[] = {"ls", NULL};
    char *sort[] = {"sort", "-n", NULL};
    char *wc[] = {"wc", NULL};


    char ** comandos[] = {ls, sort, wc, NULL};

    for(i = 0; i < 4; i++){
        cmdAux[i] = comandos[i];
    }

    for(i = 0; i < 3; i++){
            printf("\npalabra %s en la posicion %d de cmdAux\n", cmdAux[i][0], i);
        }
*/
    /*for(i = 0; i < piped; i++){
            printf("\npalabra%sen la posicion %d\n", strpiped[i], i);
        }*/

    if (piped) { 

        for(int j = 0; j < piped; j++){
        i = 0;
        while(i <= strlen(strpiped[j])){

              //  printf("%c\n", strpiped[j][i]);
            if(strpiped[j][i] =='<'){ // si encuentra un simbolo de redireccionamiento de entrada, entrara en el if 
                    strpiped[j][i] = ' ';
                    i++;
                    if(strpiped[j][i] !=' '){ //debe de haber separaciones entre cada letra o simbolo
                        ejecutar=1;
                    }else{
                        i++; //se lee despues del espacio
                            for(y = 0; strpiped[j][i] !='\0' && strpiped[j][i] !=' ' && strpiped[j][i] !='|' && strpiped[j][i] !='>'; y++){
                                entrada[y] = strpiped[j][i]; //vamos formando el argumento 
                                strpiped[j][i] = ' ';
                                i++;
                            }
                            entrada[y]='\0'; // se asigna un terminador de cadena 
                            if(strpiped[j][i]!='\0') i++; //avanzamos a lo que sigue del comando
                            redirEnt(entrada); //mandamos el argumento a la funcion para que se pueda procesar el fichero que se abrirá
                        }
            }

            if (strpiped[j][i] == '>') { // si encuentra un > corta la cadena que será el fichero que se usará para la salida
                    strpiped[j][i] = ' ';
                    i++;
                    if (strpiped[j][i] != ' '){
                        ejecutar=1; //esto es para confirmar posteriormente que existe un erros de sintaxis
                    }else{
                            i++; // se lee despues del espacio
                                for(y = 0; strpiped[j][i] != '\0';y++){
                                    salida[y] = strpiped[j][i]; // vamos formando el argumento de la funcion redirSal que será el fichero de salida 
                                    strpiped[j][i] = ' ';
                                    i++;
                                }
                        salida[y] = '\0'; // se le asigana el terminador de cadena 
                        redirSal(salida); //mandamos el argumento creado a la funcion para que sea tomado como el fichero de salida 
                    }
            }

            i++;

        }
    }    

    if(ejecutar!=0) printf("Error en la sintáxis\n");

        parseSpace(strpiped[0], parsed);


        for(i = 0; i < piped; i++){
           // printf("\npalabras que se van a guardar: %s en la posicion %d\n", *parseSpacePipes(strpiped[i], parsedpipe), i);
            
            cmd[i] = parseSpacePipes(strpiped[i], parsedpipe);
           // printf("\npalabra %s en la posicion %d que es la nueva posicion\n", cmd[j][0], j);
            //if(j > 0)
             //   printf("\npalabra %s en la posicion %d que es la anterior a la actual\n", cmd[j-1][0], j-1);
        }
       // printf("\n\nnum : %d\n\n", i);
        cmd[i] = NULL;
        
      //  printf("\npalabra %s \n", cmd[0][0]);
      //  printf("\npalabra %s \n", cmd[1][1]);
      //  printf("\npalabra %s \n", cmd[2][0]);
      //  printf("\npalabra %s \n", cmd[3]);

       /*for(i = 0; i < piped+1; i++){
            printf("\npalabra %s en la posicion %d\n", cmd[i][0], i);
        }*/
       // printf("\n\nsi pasamos de qui como no\n\n");

        //parseSpace(strpiped[1], parsedpipe); 
    } else { 

        i = 0;
        while(i <= strlen(str)){
              //  printf("%c\n", strpiped[j][i]);
            if(str[i] =='<'){ // si encuentra un simbolo de redireccionamiento de entrada, entrara en el if 
                    str[i] = ' ';
                    i++;
                    if(str[i] !=' '){ //debe de haber separaciones entre cada letra o simbolo
                        ejecutar=1;
                    }else{
                        i++; //se lee despues del espacio
                            for(y = 0; str[i] !='\0' && str[i] !=' ' && str[i] !='|' && str[i] !='>'; y++){
                                entrada[y] = str[i]; //vamos formando el argumento 
                                str[i] = ' ';
                                i++;
                            }
                            entrada[y]='\0'; // se asigna un terminador de cadena 
                            if(str[i]!='\0') i++; //avanzamos a lo que sigue del comando
                            redirEnt(entrada); //mandamos el argumento a la funcion para que se pueda procesar el fichero que se abrirá
                        }
            }

            if (str[i] == '>') { // si encuentra un > corta la cadena que será el fichero que se usará para la salida
                    str[i] = ' ';
                    i++;
                    if (str[i] != ' '){
                        ejecutar=1; //esto es para confirmar posteriormente que existe un erros de sintaxis
                    }else{
                            i++; // se lee despues del espacio
                                for(y = 0; str[i] != '\0';y++){
                                    salida[y] = str[i]; // vamos formando el argumento de la funcion redirSal que será el fichero de salida 
                                    str[i] = ' ';
                                    i++;
                                }
                        salida[y] = '\0'; // se le asigana el terminador de cadena 
                        redirSal(salida); //mandamos el argumento creado a la funcion para que sea tomado como el fichero de salida 
                    }
            }

            i++;

        }

         if(ejecutar!=0) printf("Error en la sintáxis\n");


        parseSpace(str, parsed);

    } 
    if (ownCmdHandler(parsed)) 
        return 0;
    else{
        pipeline(cmd);
        return 1 + piped; 
    }
} 

int main() 
{ 

    char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
    char* parsedArgsPiped[MAXLIST]; 
    int execFlag = 0; 
    int stdout = dup(1), stdin = dup(0);
    
    init_shell();

    while (1) { 
        close(1); // Se cierra la salida que tenga 
        dup(stdout);
            close(0); //Se cierra la salida, tambien cierra el fichero cuando se ha guardado en el
        dup(stdin);
        // print shell line 
        printDir(); 
        // take input 
        if (takeInput(inputString)) 
            continue; 
        // process 
        execFlag = processString(inputString, parsedArgs, parsedArgsPiped); 
        // execflag returns zero if there is no command 
        // or it is a builtin command, 
        // 1 if it is a simple command 
        // 2 if it is including a pipe. 
       // printf("\n result: %d\n", execFlag);
        // execute 
        if (execFlag == 1) 
            execArgs(parsedArgs); 
        //if (execFlag > 1) 
          //  execArgsPiped(parsedArgs, parsedArgsPiped); 
    } 
    return 0; 
}