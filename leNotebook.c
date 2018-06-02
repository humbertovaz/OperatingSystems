#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h> // isdigit
#include <signal.h> // signal
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
    char *buffer = (char *)malloc(MAX_CHAR_LINE);
    int res, i = -1;

    while ((res = read(fd, buffer + (++i), 1)) > 0 && buffer[i] != '\n' && i < MAX_CHAR_LINE - 1)
        ;

    if (res == 0 && i <= 0)
        return NULL;
    buffer[i] = '\0';

    return buffer;
}

//Ler linha a linha à procura de $; quando encontrar executa imprimindo o output no formato acima
//fgets(char * restrict str, int size, FILE * restrict stream);

Estrut initEstrut(Estrut estrut)
{
    estrut = (Estrut)malloc(sizeof(struct estrutura));
    estrut->nrlinhas = 0;
    estrut->data = (char **)malloc(sizeof(char *) * 100); //Alocar o primeiro apontador de todos para a matriz
    return estrut;
}
Command initComs()
{
    Command command;
    command = (Command)malloc(sizeof(struct command));
    command->command = (char **)malloc(sizeof(char *) * 4); // Alocar 4 apontadores para strings
    command->out = (char **)malloc(sizeof(char *) * 100);   //Alocar o primeiro apontador de todos para a matriz
    command->nrcoms = 0;
    return command;
}

Command* addCommand(Command *commands, Command command, int index, int *size)
{
    Command* aux = commands;
    commands = (Command *)malloc(sizeof(Command)*((*size + 1) * 2));

    *size = (*size + 1) * 2;

    int i=0;
    for(i =0; i< index; i++)
    {
        commands[i] = aux[i];
    }

    commands[index] = command;
    free(aux);

    return commands;
}
void sigHandler(){
    exit(0);
}

int main(int argc, char const *argv[])
{
    signal (SIGINT,sigHandler);
    char **str = malloc(sizeof(char *) * 20); // Alocar 20 apontadores
    char temp[100];
    char *token = malloc(sizeof(char) * 100);
    FILE *file = fopen("teste.nb", "r+");
    int x, k;
    char *linha;
    char *readFile = NULL;
    char *readPipe = (char *)malloc(MAX_CHAR_LINE);
    int sizeCommands = 0;
    int indexCommands = 0;
    int line;
    int ignoreLines = 0; // >>> and <<<
    // Criacao de estruturas auxiliares
    Estrut estrut = NULL;
    estrut = initEstrut(estrut);
    Command *commands = (Command *)malloc(sizeof(struct command));

    while ((readFile = fgets(temp, 100, file)) != NULL)
    {
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

        if(strcmp(linha, ">>>")==0)
        {
            ignoreLines = 1;
        }
        /// Fim do STRTOK
        str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
        if (str[0][0] != '$')
        { // Não é comand, imprime
            // Imprime a linha "não modificada" pelo strtok (temp);
            if(ignoreLines == 0)
            {
                estrut->data[estrut->nrlinhas++] = strdup(linha);
            }
        }
        else
        {
            // Adiciona comando
            Command addingCommand;
            addingCommand = initComs();
            // ADICIONAR COMANDO COMPLEXO
            for (j = 0; str[j] != NULL; j++)
            {
                addingCommand->command[j] = strdup(str[j]);
            }
            addingCommand->command[j] = NULL;
            commands = addCommand(commands, addingCommand, indexCommands++, &sizeCommands);

            estrut->data[estrut->nrlinhas++] = strdup(linha);
        }
    }

    for (k = 0; k < indexCommands; k++)
    {
        int fd[2];
        int r = pipe(fd);
        if (r < 0)
            perror("Erro no Pipe\n");
        x = fork();
        if (x == 0 && commands[k]->command[0][1] != '|') // É comand mas não tem pipe
        {
            // Fecha pipe entrada
            close(fd[0]);
            dup2(fd[WRITE_END], STDOUT_FILENO);
            execvp(commands[k]->command[1], &(commands[k]->command[1]));
            perror("Não devia imprimir isto\n");
            exit(-1); //Exec correu mal
        }
        if (x == 0 && commands[k]->command[0][1] == '|') // É command tem pipe
        {

            // Fecha pipe entrada
            close(fd[0]);
            char **args;
            int argCounter = 0;
            int j;
            int fdPipe[2];
            pipe(fdPipe);

            int y;
            y = fork();
            if (y == 0)
            {
                close(fdPipe[0]);
                dup2(fdPipe[WRITE_END], STDOUT_FILENO);

                int l;
                for (l = 0; commands[k - 1]->out[l] != NULL; l++)
                {
                    write(fdPipe[WRITE_END], commands[k - 1]->out[l], strlen(commands[k - 1]->out[l]));
                    write(fdPipe[WRITE_END], "\n", 1);
                }

                close(fdPipe[WRITE_END]);
                exit(0);
            }
            else
            {
                close(fdPipe[1]);
                dup2(fdPipe[READ_END], STDIN_FILENO);
                dup2(fd[WRITE_END], STDOUT_FILENO);
                args = (char **)malloc(sizeof(char *) * 20); // Alternativa ao realloc
                for (j = 0; commands[k]->command[j + 1] != NULL; j++)
                {
                    if (argCounter == j)
                    {
                        args = (char **)realloc(args, (argCounter + 1) * 2);
                        argCounter = (argCounter + 1) * 2;
                    }

                    args[j] = strdup(commands[k]->command[j + 1]);
                }
                args[j] = NULL;

                execvp(args[0], args);
                perror("Não devia imprimir isto\n");
                exit(-1); //Exec correu mal
            }
        }
        else
        { // Pai
            close(fd[WRITE_END]);
            int status;
            char buf[100];
            int counter = 0;
            wait(0);
            WEXITSTATUS(status);
            if (status == 0) // SUCCESS
            {
                // CONTROLO DE ERROS
                line = 0;
                while ((readPipe = lerLinha(fd[READ_END])) != NULL)
                {
                    // Ouvir o que cada filho diz e escrever na estrutura Commands
                    commands[k]->out[line++] = strdup(readPipe);
                }
                int i;
            }
            else // ERROR
            {
                perror("Error on waiting for child process\n");
            }
            // Talvez seja necessário fechar descritores
            close(fd[READ_END]);
        }
    }

    // Escrever no ficheiro original
    //fclose(file);
    int f = open("X.nb", O_RDWR | O_CREAT, 0666);
    int comandoAtual = 0;
    int numCom;
    for (int i = 0; i < estrut->nrlinhas; i++)
    {
        numCom = -1;    // Funcionalidade avançada   ex: $1|
        if (estrut->data[i][0] == '$')
        {
            int j = 0;
            write(f, estrut->data[i], strlen(estrut->data[i])); // Imprime comando -> TESTE- > subst por estrut
            write(f, "\n>>>\n", 5);
            char tmp = estrut->data[i][1];
            
            // Funcionalidade avançada $1 -> Comando 1
            if (isdigit(tmp)){
                numCom = atoi(&tmp);
                if (numCom > 0 && numCom < indexCommands) 
                    comandoAtual = comandoAtual - numCom;
                else {
                    perror("Comando inválido");
                    exit(-1);
                    }
            }

            while (commands[comandoAtual]->out[j] != NULL)
            {
                write(f, commands[comandoAtual]->out[j], strlen(commands[comandoAtual]->out[j])); // Imprime out para o ficheiro
                write(f, "\n", 1);
                j++;
            }
            comandoAtual++;
            write(f, "<<<\n", 4);
        }
        else
        {
            write(f, estrut->data[i], strlen(estrut->data[i])); // Imprime linha "não comando"
            write(f, "\n", 1);
                     
        }
        
        
    }
    rewind(file);
    fclose(file);
    close(f);
    system("rm teste.nb"); 
    system("mv X.nb teste.nb");
    
    return 0;
}
