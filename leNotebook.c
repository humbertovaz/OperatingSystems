#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
/* 
Lê ficheiro .nb (formato ficticio);  sempre que encontra um $, tem de executar o comando 
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
typedef struct comandos
{
     int nrlinhas;
    char **comando;
    char **data;
} * Coms;
//Ler linha a linha à procura de $; quando encontrar executa imprimindo o output no formato acima
//fgets(char * restrict str, int size, FILE * restrict stream);

Estrut initEstrut(Estrut estrut)
{
    estrut = (Estrut)malloc(sizeof(struct estrutura));
    estrut->nrlinhas = 0;
    estrut->data = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
    return estrut;
}
Coms initComs(Coms comandos)
{
    comandos = (Coms)malloc(sizeof(struct comandos));
    comandos->nrlinhas = 0;
    comandos->comando = (char **)malloc(sizeof(char*)*100);
    comandos->data = (char **)malloc(sizeof(char*)*100); //Alocar o primeiro apontador de todos para a matriz
    return comandos;
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
    // Criacao de estruturas auxiliares
    Estrut estrut;
    estrut = initEstrut(estrut);
    Coms comandos;
    comandos = initComs(comandos);
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
            
            // Redirecionar o descritor do pipe para o descritor de leitura
            close(fd[1]);   //Fecha o stdout do pipe
            dup2(0, fd[0]); //Põe o fd[0] a receber do stdin para o gets ler do pipe
            // Passar para a estrutura
            while ((readPipe = gets(linha)) != NULL)
            {
                estrut->data[estrut->nrlinhas] = (char *)malloc(sizeof(char) * estrut->nrcols); //alocar um apontador com #nrcols carateres
                strcpy(estrut->data[estrut->nrlinhas], linha);
                estrut->nrlinhas++;
            }
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
                printf("TOKENISER: str[%d]=%s\n",i,str[i - 1]);
            }

            /// Fim do STRTOK
            str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
            if (str[0][0] !=  '$')
            { // Não é comando, imprime
                // Imprime a linha "não modificada" pelo strtok (temp);
                printf("%s\n", linha);
                estrut->data[estrut->nrlinhas++] = strdup(linha);
                printf("fim do assing ao data\n");
            }
            else
            {
                //Corre o programa "executa" para correr o programa em causa
                x = fork();
                if (x == 0 && str[0][1] != '|') // É comando mas não tem pipe
                {
                    estrut->data[estrut->nrlinhas++] = strdup(linha);
                    comandos->comando[comandos-> nrlinhas++] = strdup(str[0]);
                    printf("%s\n", linha);
                    printf(">>>\n");
                    execvp(str[1], &(str[1]));
                    printf("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                }
                else if (x == 0 &&  str[0][1] == '|') // É comando e tem pipe
                {
                    // Fecha pipe entrada
                    close(fd[0]);
                    // Fazer apontar o stdout para o pipe
                    dup2(fd[1], 1);

                    // estrut->data[estrut->nrlinhas] = (char *)malloc(sizeof(char) * estrut->nrcols); //alocar um apontador com #nrcols carateres
                    // strcpy(estrut->data[estrut->nrlinhas], linha);
                    // estrut->nrlinhas++;
                    
                    comandos->comando[comandos->nrlinhas++] = strdup(linha);
                    
                    printf("%s\n", linha);
                    printf(">>>\n");
                    execvp(str[1], &(str[1]));
                    printf("Não devia imprimir isto\n");
                    exit(-1); //Exec correu mal
                }
                else
                { // P
                    int status;
                    wait(0);
                    char buf[100];
                    printf("Aqui\n");
                    printf("<<<\n");
                    // CONTROLO DE ERROS
                    WEXITSTATUS(status);
                    printf("Esperei pelo meu filho! status = %d\n", status);
                    //exit(0);
                    int i = 0;
                    for(i = 0; i < estrut->nrlinhas; i++){
                        printf("Olá %s\n",estrut->data[i]);
                    }
                }
            }
        }
    }
    return 0;
}
