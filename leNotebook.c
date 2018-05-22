#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
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
    char *command;
    char **out;
} * Command;
//Ler linha a linha à procura de $; quando encontrar executa imprimindo o output no formato acima
//fgets(char * restrict str, int size, FILE * restrict stream);

Estrut initEstrut(Estrut estrut)
{
    estrut = (Estrut)malloc(sizeof(struct estrutura));
    estrut->nrlinhas = 0;
    estrut->data = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
    return estrut;
}
Command initComs(Command command)
{
    command= (Command)malloc(sizeof(struct command));
    command->command = (char *)malloc(sizeof(char));
    command->out = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
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
    int i, x;
    char *linha;
    char *readFile = NULL;
    char *readPipe = NULL;
    int sizeCommands = 0;
    int indexCommands = 0;
    // Criacao de estruturas auxiliares
    Estrut estrut;
    estrut = initEstrut(estrut);
    Command *commands = (Command*)malloc(sizeof(struct command));
    printf("Passei aqui1\n");
    // Cria pipe
    int fd[2];
    int r = pipe(fd);
    if (r < 0)
        perror("Erro no Pipe\n");

    while (1)
    {
        printf("antes fgets\n");
        readFile = fgets(temp, 100, file); //fgets já coloca o \0 no fim da string
        printf("depois fagets\n");
        if (readFile == NULL)              // Chegamos ao final do ficheiro
        {
            
           
            printf("Passei aqui3\n");
            //e  Alterar o ficheiro

            exit(0);
        }
        else
        {
            /// STRTOK
            temp[strlen(temp) - 1] = '\0';
            linha = strdup(temp);  // guardar linha para imprimir mais tarde
            char *s = strdup(" "); // separator
            /* get the first token */
            token = strtok(temp, s);
            //token[strlen(token)] = '\0';
            i = 0;

            /* walk through other tokens */
            while (token != NULL)
            {
                str[i] = strdup(token);
                //token[strlen(token)] = '\0';
                token = strtok(NULL, s);
                i++;
                //printf("TOKENISER: str[%d]=%s\n",i,str[i - 1]);
            }

            /// Fim do STRTOK
            str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
            if (str[0][0] !=  '$')
            { // Não é comand, imprime
                // Imprime a linha "não modificada" pelo strtok (temp);
                printf("%s\n", linha);
                estrut->data[estrut->nrlinhas++] = strdup(linha);
                
            }
            else
            {
                //Corre o programa "executa" para correr o programa em causa
                // Fazer apontar o stdout para o pipe    
                dup2(fd[1], 1); // vou escrever para o pipe
                // Adiciona comando
                Command addingCommand;
                addingCommand = initComs(addingCommand);
                addingCommand->command = strdup(linha);     
                addCommand(commands, addingCommand, indexCommands++, &sizeCommands);
                
                
                x = fork();
                
                
                if (x == 0 && str[0][1] != '|') // É comand mas não tem pipe
                {
                    // Fecha pipe entrada
                    close(fd[0]);
                    printf("%s\n", linha);
                    printf(">>>\n");
                    execvp(str[1], &(str[1]));
                    printf("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                }
                else if (x == 0 &&  str[0][1] == '|') // É command tem pipe
                {
                    // Fecha pipe entrada
                    close(fd[0]);

                    char **args;
                    int argCounter = 0;
                    int j;

                    // CONCAT CURRENT COMMAND AND ITS ARGS
                    for(j=0; str[j + 1] != NULL; j++)
                    {
                        if(argCounter == j)
                        {
                            args =(char**)realloc(args, (argCounter+1)*2);
                            argCounter = (argCounter + 1) * 2;
                        }

                        args[j] = strdup(str[j + 1]);
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

                    //printf("HEY%s\n", linha);
                    printf(">>>\n");
                    execvp(args[0], args);
                    printf("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                }
                else
                { // P
                    // Redirecionar o descritor do pipe para o descritor de leitura
                    close(fd[1]);   //Fecha o stdout do pipe
                    dup2(fd[0], 0); //Vou Ler do pipe
                    int status;
                    wait(0);
                    char buf[100];
                    printf("<<<\n");
                    // CONTROLO DE ERROS
                    WEXITSTATUS(status);
                    if(status == 0) // SUCCESS
                    {
                        strcpy(linha,""); // limpar conteudo anterior
                        int line = 0;
                        while ((readPipe = gets(linha)) != NULL)
                        {
                            // estrut->data[estrut->nrlinhas] = (char *)malloc(sizeof(char) * estrut->nrcols); //alocar um apontador com #nrcols carateres
                            // strcpy(estrut->data[estrut->nrlinhas], linha);
                            // estrut->nrlinhas++;
                            // Ouvir o que cada filho diz e escrever na estrutura Commands
                            Command currentCommand = commands[indexCommands -1];
                            currentCommand->out[line++] = strdup(linha); // Modifica o output do comando
                            commands[indexCommands - 1] = currentCommand; //Coloca-o na estrutura
                            


                        }
                    }
                    else // ERROR
                    {
                        perror("Error on waiting for child process\n");
                    }
                    printf("Esperei pelo meu filho! status = %d\n", status);
                    //exit(0);
                    
                }
            }
        }
    }
    return 0;
}
