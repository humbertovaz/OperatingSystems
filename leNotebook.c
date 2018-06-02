#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#define WRITE_END 1
#define READ_END 0
#define STDOUT_FILENO 1
#define STDIN_FILENO 0
#define MAX_CHAR_LINE 100

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

Estrut initEstrut(Estrut estrut)
{
    estrut = (Estrut)malloc(sizeof(struct estrutura));
    estrut->nrlinhas = 0;
    estrut->data = (char **)malloc(sizeof(char *) * 100);
    return estrut;
}
Command initComs()
{
    Command command;
    command = (Command)malloc(sizeof(struct command));
    command->command = (char **)malloc(sizeof(char *) * 4);
    command->out = (char **)malloc(sizeof(char *) * 100); 
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

int main(int argc, char const *argv[])
{
    char **str = malloc(sizeof(char *) * 20); // Alocar 20 apontadores
    char temp[100];
    char *token = malloc(sizeof(char) * 100);
    char *fileName;
    if(argc == 2)
    {
        fileName = strdup(argv[1]);
    }
    else
    {
        char errorMsg[260] = "file name missing, please use ";
        strcat(errorMsg, argv[0]);
        strcat(errorMsg, " fileName");
        perror(errorMsg);
        exit(1);
    }
    FILE *file = fopen(fileName, "r");
    int x, k;
    char *linha;
    char *readFile = NULL;
    char *readPipe = (char *)malloc(MAX_CHAR_LINE);
    int sizeCommands = 0;
    int indexCommands = 0;
    int line;
    int ignoreLines = 0;
    Estrut estrut = NULL;
    estrut = initEstrut(estrut);
    Command *commands = (Command *)malloc(sizeof(struct command));

    while ((readFile = fgets(temp, 100, file)) != NULL)
    {
        temp[strlen(temp) - 1] = '\0';
        linha = strdup(temp);
        char *s = strdup(" ");
        token = strtok(temp, s);
        int i = 0;
        int j = 0;

        while (token != NULL)
        {
            str[i] = strdup(token);
            token = strtok(NULL, s);
            i++;
        }

        str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
        if(strcmp(linha, ">>>")==0)
        {
            ignoreLines = 1;
        }

        if (str[0] != NULL && str[0][0] != '$')
        { 
            if(ignoreLines == 0)
            {
                estrut->data[estrut->nrlinhas++] = strdup(linha);
            }
        }
        else if(str[0] != NULL)
        {
            Command addingCommand;
            addingCommand = initComs();

            for (j = 0; str[j] != NULL; j++)
            {
                addingCommand->command[j] = strdup(str[j]);
            }
            addingCommand->command[j] = NULL;
            commands = addCommand(commands, addingCommand, indexCommands++, &sizeCommands);

            estrut->data[estrut->nrlinhas++] = strdup(linha);
        }

        if(strcmp(linha, "<<<")==0)
        {
            ignoreLines = 0;
        }
    }

    for (k = 0; k < indexCommands; k++)
    {
        int fd[2];
        int r = pipe(fd);
        if (r < 0)
            perror("Pipe failed\n");
        x = fork();
        if (x == 0 && commands[k]->command[0][1] != '|' && (isdigit(commands[k]->command[0][1]) == 0)) 
        {
            close(fd[0]);
            dup2(fd[WRITE_END], STDOUT_FILENO);
            execvp(commands[k]->command[1], &(commands[k]->command[1]));
            perror("Exec failed\n");
            exit(-1); 
        }

        if (x == 0 && (commands[k]->command[0][1] == '|' || isdigit(commands[k]->command[0][1])))
        {
            int previousCommand = 1;
            int hasDigit = 0;
            char tempNumber[5];
            int index = 1;
            char **args;
            int argCounter = 0;
            int j;

            while (isdigit(commands[k]->command[0][index]) != 0)
            {
                tempNumber[index - 1] = commands[k]->command[0][index];
                hasDigit = 1;
                index++;
            }
            
            args = (char **)malloc(sizeof(char *) * 20);
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

            close(fd[0]);
            int fdPipe[2];
            pipe(fdPipe);

            int y;
            y = fork();

            if (y == 0)
            {
                int commandIndex = k;
                close(fdPipe[0]);
                dup2(fdPipe[WRITE_END], STDOUT_FILENO);

                if (hasDigit == 1)
                {
                    previousCommand = atoi(tempNumber);
                    if (previousCommand > 0 && previousCommand <= indexCommands)
                        commandIndex = commandIndex - previousCommand;
                    else
                    {
                        perror("Comando inválido");
                        exit(-1);
                    }
                }
                else
                {
                    commandIndex = commandIndex - previousCommand;
                }

                int l;
                for (l = 0; commands[commandIndex]->out[l] != NULL; l++)
                {
                    write(fdPipe[WRITE_END], commands[commandIndex]->out[l], strlen(commands[commandIndex]->out[l]));
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
               
                execvp(args[0], args);
                perror("Exec failed\n");
                exit(-1);
            }
        }
        else
        {
            close(fd[WRITE_END]);
            int status;
            wait(0);
            WEXITSTATUS(status);
            if (status == 0) 
            {
                line = 0;
                while ((readPipe = lerLinha(fd[READ_END])) != NULL)
                {
                    commands[k]->out[line++] = strdup(readPipe);
                }
            }
            else
            {
                perror("Error on waiting for child process\n");
                close(fd[READ_END]);
                exit(1);
            }
            close(fd[READ_END]);
        }
    }

    fclose(file);
    int f = open(fileName, O_TRUNC | O_WRONLY, 0666);

    int comandoAtual = 0;
    for (int i = 0; i < estrut->nrlinhas; i++)
    {
        if (estrut->data[i][0] == '$')
        {
            int j = 0;
            write(f, estrut->data[i], strlen(estrut->data[i])); 
            write(f, "\n>>>\n", 5);

            while (commands[comandoAtual]->out[j] != NULL)
            {
                write(f, commands[comandoAtual]->out[j], strlen(commands[comandoAtual]->out[j]));
                write(f, "\n", 1);
                j++;
            }
            comandoAtual++;
            write(f, "<<<\n", 4);
        }
        else
        {
            write(f, estrut->data[i], strlen(estrut->data[i]));
            write(f, "\n", 1);
        }
    }

    close(f);

    return 0;
}
