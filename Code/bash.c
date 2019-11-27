/*============================================================================*/
/*================== INSTITUTO POLITÉCNICO NACIONAL ==========================*/
/*=================== ESCUELA SUPERIOR DE CÓMPUTO ============================*/
/*============================================================================*/
/*================= Ivan Aldavera Gallaga ====================================*/
/*================= Erick Francisco Vázquez Nuñez ============================*/
/*================= Laura Andrea Morales López ===============================*/
/*============================================================================*/

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <fcntl.h>
  
#define MAXCOM 1000                                                         // Número máximo de letras
#define MAXLIST 100                                                         // Número máximo de comandos 
#define TAM 60


//======================= Mensaje de bienvenida ======================================================================
void init_shell() 
{ 
    printf("\n\n\n\n================================================"); 
    printf("\n\n\n\t****MINIBASH****"); 
    printf("\n\n\n\n================================================"); 

    char* username = getenv("USER"); 
    printf("\n"); 
    sleep(1); 
} 
//======================= Manejo de archivos "<" y " >" ======================================================================

void redirSal(char cad[TAM]){
    char *cadPtr;
    cadPtr=cad;                                                             //puntero a la cadena
    close(1);                                                               //cerramos la salida estándar
    open(cadPtr,O_CREAT | O_WRONLY,0777);                                   //Se asigna la salida al fichero, tambien se le asignan permisos totales

}
void redirEnt(char cad[TAM]){
    char *cadPtr;
    int f;
    cadPtr = cad;                                                           //puntero a la cadena 
    f=open(cadPtr,O_RDONLY);                                                // se asigna la salida al fichero
    close(0);                                                               //cerramos la salida estándar
    dup(f);
}
//======================= Lectura de entrada  ======================================================================

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
//======================= Imprime la dirección ======================================================================

void printDir() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd); 
} 
//======================= Ejecuta comandos ======================================================================


void execArgs(char** parsed) 
{ 
    
    pid_t pid = fork();  
    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else {                                                    // Esperando al hijo
        wait(NULL);  
        return; 
    } 
} 
//======================= Ejecuta comandos en tuberías ======================================================================

void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
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
                                                                                 // Hijo 1
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 

    } else { 
                                                                                 //Código del padre
        p2 = fork(); 
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
                                                                                // Hijo 2
       
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
                                                                                //Espera a los hijos
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 
//======================= Manejo de tuberias ======================================================================
void pipeline(char ***cmd)
{
    int fd[2];
    pid_t pid;
    int fdd = 0;               

    while (*cmd != NULL) {
        pipe(fd);             
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
            wait(NULL);        
            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}

//======================= Manejo de comandos ======================================================================

int ownCmdHandler(char** parsed) 
{ 
    int NoOfOwnCmds = 2, i, switchOwnArg = 0; 
    char* ListOfOwnCmds[NoOfOwnCmds]; 
    char* username; 
    ListOfOwnCmds[0] = "exit"; 
    ListOfOwnCmds[1] = "cd"; 
    for (i = 0; i < NoOfOwnCmds; i++) { 
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
            switchOwnArg = i + 1; 
            break; 
        } 
    } 
    switch (switchOwnArg) { 
    case 1: 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
 
    default: 
        break; 
    } 
    return 0; 
} 

//======================= Manejo de cadena (tubería) ======================================================================

int parsePipe(char* str, char** strpiped) 
{ 
    int i = 0; 

    while( (strpiped[i] = strsep(&str,"|")) != NULL ){

        i++;
    }

    if (strpiped[1] == NULL) 
        return 0;                                                                 //Regresa cero si no encuntra una tubería
    else { 
        return i; 
    } 
} 

 //======================= Manejo de cadena espacios ======================================================================

void parseSpace(char* str, char** parsed) 
{ 
    int i;
    char *aux; 
    for (i = 0; i < MAXLIST; i++) { 
        aux = strsep(&str, " "); 
        if (aux == NULL){ 
            parsed[i] = aux;
      
            break;
        }
        else if (strlen(aux) == 0)
            i--; 
        else{
            parsed[i] = aux;
        }
    }
} 
//======================= Manejo de cadena espacios tubería ======================================================================

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
            return parsed2; 
        }
        else if (strlen(aux) == 0)
            i--; 
        else{
            parsed[i] = aux;
            parsed2[i] = aux;
        }
    }
} 



//======================= Procesar la cadena PRINCIPAL======================================================================

int processString(char* str, char** parsed, char** parsedpipe) 
{ 
    char* strpiped[100]; 
    int piped = 0, i = 0, y, k = 0, ejecutar = 0; 
    piped = parsePipe(str, strpiped); 
    char entrada[MAXLIST], salida[TAM];

    char ** cmd[(piped+1)];

    if (piped) { 

        for(int j = 0; j < piped; j++){
        i = 0;
        while(i <= strlen(strpiped[j])){

             
            if(strpiped[j][i] =='<'){                                    // si encuentra un simbolo de redireccionamiento de entrada, entrara en el if 
                    strpiped[j][i] = ' ';
                    i++;
                    if(strpiped[j][i] !=' '){                           //debe de haber separaciones entre cada letra o simbolo
                        ejecutar=1;
                    }else{
                        i++;                                            //se lee despues del espacio
                            for(y = 0; strpiped[j][i] !='\0' && strpiped[j][i] !=' ' && strpiped[j][i] !='|' && strpiped[j][i] !='>'; y++){
                                entrada[y] = strpiped[j][i];                 //vamos formando el argumento 
                                strpiped[j][i] = ' ';
                                i++;
                            }
                            entrada[y]='\0';                            // se asigna un terminador de cadena 
                            if(strpiped[j][i]!='\0') i++;               //avanzamos a lo que sigue del comando
                            redirEnt(entrada);                          //mandamos el argumento a la funcion para que se pueda procesar el fichero que se abrirá
                        }
            }

            if (strpiped[j][i] == '>') {                                 // si encuentra un > corta la cadena que será el fichero que se usará para la salida
                    strpiped[j][i] = ' ';
                    i++;
                    if (strpiped[j][i] != ' '){
                        ejecutar=1;                                     //esto es para confirmar posteriormente que existe un erros de sintaxis
                    }else{
                            i++;                                        // se lee despues del espacio
                                for(y = 0; strpiped[j][i] != '\0';y++){
                                    salida[y] = strpiped[j][i];          // vamos formando el argumento de la funcion redirSal que será el fichero de salida 
                                    strpiped[j][i] = ' ';
                                    i++;
                                }
                        salida[y] = '\0';                               // se le asigana el terminador de cadena 
                        redirSal(salida);                               //mandamos el argumento creado a la funcion para que sea tomado como el fichero de salida 
                    }
            }

            i++;

        }
    }    

    if(ejecutar!=0) printf("Error en la sintáxis\n");

        parseSpace(strpiped[0], parsed);


        for(i = 0; i < piped; i++){
            cmd[i] = parseSpacePipes(strpiped[i], parsedpipe);
           
        }
        cmd[i] = NULL;
        
  
    } else { 

        i = 0;
        while(i <= strlen(str)){
            if(str[i] =='<'){                                            // si encuentra un simbolo de redireccionamiento de entrada, entrara en el if 
                    str[i] = ' ';
                    i++;
                    if(str[i] !=' '){                                    //debe de haber separaciones entre cada letra o simbolo
                        ejecutar=1;
                    }else{
                        i++;                                             //se lee despues del espacio
                            for(y = 0; str[i] !='\0' && str[i] !=' ' && str[i] !='|' && str[i] !='>'; y++){
                                entrada[y] = str[i];                    //vamos formando el argumento 
                                str[i] = ' ';
                                i++;
                            }
                            entrada[y]='\0';                            // se asigna un terminador de cadena 
                            if(str[i]!='\0') i++;                       //avanzamos a lo que sigue del comando
                            redirEnt(entrada);                          //mandamos el argumento a la funcion para que se pueda procesar el fichero que se abrirá
                        }
            }

            if (str[i] == '>') {                                        // si encuentra un > corta la cadena que será el fichero que se usará para la salida
                    str[i] = ' ';
                    i++;
                    if (str[i] != ' '){
                        ejecutar=1;                                    //esto es para confirmar posteriormente que existe un erros de sintaxis
                    }else{
                            i++;                                       // se lee despues del espacio
                                for(y = 0; str[i] != '\0';y++){
                                    salida[y] = str[i];                  // vamos formando el argumento de la funcion redirSal que será el fichero de salida 
                                    str[i] = ' ';
                                    i++;
                                }
                        salida[y] = '\0';                               // se le asigana el terminador de cadena 
                        redirSal(salida);                               //mandamos el argumento creado a la funcion para que sea tomado como el fichero de salida 
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
        close(1);                                                       // Se cierra la salida que tenga 
        dup(stdout);
            close(0);                                                   //Se cierra la salida, tambien cierra el fichero cuando se ha guardado en el
        dup(stdin);
       
        printDir();                                                     //imprime
       
        if (takeInput(inputString)) 
            continue; 
      
        execFlag = processString(inputString, parsedArgs, parsedArgsPiped); 
     
        if (execFlag == 1) 
            execArgs(parsedArgs);                                     //Ejecuta si hay comandos
    
    } 
    return 0; 
}