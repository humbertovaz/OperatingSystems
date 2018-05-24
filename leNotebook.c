#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define WRITE_END 1
#define READ_END 0
#define STDOUT_FILENO 1
#define STDIN_FILENO 0
#define MAX_CHAR_LINE 100

/* 
Lê ficheiro .nb (formato ficticio);  sempre que encontra um $, tem de executar o comand 
que se encontra a seguir e imprimir o output do programa da seguinte forma:
>>>
output
<<<

*/
typedef struct estrutura
{
    int nrlinhas;
    int nrcols;
    char **data;
} * Estrut;
typedef struct command
{
    char **command;
    char **out;
    int nrcoms;
} * Command;
char *lerLinha(int fd)
{
	char *buffer = (char*) malloc(MAX_CHAR_LINE);
	int res, i = -1;

	while( (res=read(fd, buffer + (++i), 1)) > 0 && buffer[i] != '\n' && i < MAX_CHAR_LINE - 1);

	if (res == 0 && i <= 0) return NULL;
	buffer[i]='\0';

	return buffer;
}

//Ler linha a linha à procura de $; quando encontrar executa imprimindo o output no formato acima
//fgets(char * restrict str, int size, FILE * restrict stream);

Estrut initEstrut(Estrut estrut)
{
    estrut = (Estrut)malloc(sizeof(struct estrutura));
    estrut->nrlinhas = 0;
    estrut->data = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
    return estrut;
}
Command initComs()
{
    Command command;
    command = (Command)malloc(sizeof(struct command));
    command->command = (char **)malloc(sizeof(char*)*4); // Alocar 4 apontadores para strings
    command->out = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
    command->nrcoms = 0;
    return command;
}

void addCommand(Command *commands, Command command, int index, int *size)
{
    if(*size == index)
    {
        commands = (Command *)realloc(commands, (*size + 1) * 2);
        *size = (*size + 1) * 2;
    }

    commands[index] = command;
}

int main(int argc, char const *argv[])
{
    char **str = malloc(sizeof(char *) * 20); // Alocar 20 apontadores
    char temp[100];
    char *token = malloc(sizeof(char) * 100);
    FILE *file = fopen("teste.nb", "r");
    int x,k;
    char *linha;
    char *readFile = NULL;
    char *readPipe = (char*) malloc(MAX_CHAR_LINE);
    int sizeCommands = 0;
    int indexCommands = 0;
    int line;
    // Criacao de estruturas auxiliares
    Estrut estrut;
    estrut = initEstrut(estrut);
    Command *commands = (Command*)malloc(sizeof(struct command));

    while ((readFile = fgets(temp, 100, file))!= NULL){
            /// STRTOK
            temp[strlen(temp) - 1] = '\0';
            linha = strdup(temp);  // guardar linha para imprimir mais tarde
            char *s = strdup(" "); // separator
            /* get the first token */
            token = strtok(temp, s);
            int i = 0;
            int j = 0;
            /* walk through other tokens */
            while (token != NULL)
            {
                str[i] = strdup(token);
                token = strtok(NULL, s);
                i++;
            }
            
            /// Fim do STRTOK
            str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
            if (str[0][0] !=  '$')
            { // Não é comand, imprime
                // Imprime a linha "não modificada" pelo strtok (temp);
                estrut->data[estrut->nrlinhas++] = strdup(linha);
            }else{  
                // Adiciona comando
                Command addingCommand;
                addingCommand = initComs();
                // ADICIONAR COMANDO COMPLEXO
                int j;
                for(j = 0; str[j]!= NULL; j++){
                   addingCommand->command[addingCommand->nrcoms++] = strdup(str[j]); 
                }
                addingCommand->command[addingCommand->nrcoms] = NULL;
                addCommand(commands, addingCommand, indexCommands++, &sizeCommands);
                estrut->data[estrut->nrlinhas++] = strdup(linha);
                printf("addingCommand-> %s",addingCommand->command[0]);
                printf("commands[%d]->command[%d] = %s\n",0,0,commands[0]->command[0]);
            
            }
    }
    printf("Cheguei ao fim do ficheiro\n");
    printf("TESTE %d\n",indexCommands);
    printf("commands[0]->command=%c\n",commands[0]->command[0][1]);
    /// É AQUI EM CIMA .. A ESTRUTURA COMMANDS NAO ESTÁ COM O COMANDO
            for(k = 0; k < indexCommands; k++){
                int fd[2];
                int r = pipe(fd);
                if (r < 0) perror("Erro no Pipe\n");
                x = fork();
                
                if (x == 0 && commands[indexCommands]->command[0][1] != '|') // É comand mas não tem pipe
                {
                    // Fecha pipe entrada
                    close(fd[0]);
                    printf("%s\n", linha);
                    printf(">>>\n");
                    printf("TESTE\n");
                    dup2(fd[WRITE_END], STDOUT_FILENO);
                    
                    execvp(commands[indexCommands]->command[1], &(commands[indexCommands]->command[1]));
                    perror("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                }
                if (x == 0 &&  commands[indexCommands]->command[0][1] == '|') // É command tem pipe
                {
                    // Fecha pipe entrada
                    close(fd[0]);
                    char **args;
                    int argCounter = 0;
                    int j;
                    printf("Teste2\n");
                    // CONCAT CURRENT COMMAND AND ITS ARGS
                    for(j=0; commands[indexCommands - 1]->command[j + 1] != NULL; j++)
                    {
                        if(argCounter == j)
                        {
                            args =(char**)realloc(args, (argCounter+1)*2);
                            argCounter = (argCounter + 1) * 2;
                        }

                        args[j] = strdup(commands[indexCommands - 1]->command[j + 1]);
                    }
                    
                    // CONCAT OUTPUT FROM PREVIOUS COMMAND
                    Command previousCommand = commands[indexCommands - 2];
                    int k;
                    for(k = 0; previousCommand->out[k] != NULL; k++)
                    {
                       if(argCounter == j + k)
                        {
                            args =(char**)realloc(args, (argCounter+1)*2);
                            argCounter = (argCounter + 1) * 2;
                        } 

                        args[j + k] = strdup(previousCommand->out[k]); // verificar inserção.. Array ?
                    }

                    printf(">>>\n");
                    dup2(fd[WRITE_END], STDOUT_FILENO);
                    printf("TESTE\n");
                    execvp(args[0], args);
                    perror("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                
                }
                else
                { // P
                    // Redirecionar o descritor do pipe para o descritor de leitura
                    close(fd[WRITE_END]);
                    int status;
                    char buf[100];
                    printf("<<<\n");
                    wait(0);
                    WEXITSTATUS(status);
                    if(status == 0) // SUCCESS
                    {
                        // CONTROLO DE ERROS
                        line=0;
                        while ((readPipe = lerLinha(fd[READ_END])) != NULL )
                        {
                            // Ouvir o que cada filho diz e escrever na estrutura Commands
                            printf("Linha%d\n",line++);
                            printf("%s\n",readPipe);
                        }
                        
                        //printf("debug %s\n",commands[indexCommands -1 ]->command);
                    }
                    else // ERROR
                    {  
                        perror("Error on waiting for child process\n");
                    }
                    
                    printf("Esperei pelo meu filho! status = %d\n", status);
                    //exit(0);
                    
                }
            }
return 0;
}
        

