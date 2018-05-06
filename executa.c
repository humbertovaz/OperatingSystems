#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <sys/types.h>

int main(int argc, char *argv[]){
    printf("%d #debug\n",argc);
    if (argc <= 1){
        perror("Erro no nr de argumentos inseridos\n");
        exit(-1);
    }
    int x;
    x = -1;
    x = fork();
    if (x == 0){
        execvp(argv[1],&(argv[1]));
        perror("Exec falhou!\n");
        exit(-2);
    }
    wait(&x);
    WEXITSTATUS(x);
    exit(x);
    //printf("O meu filho morreu com %d\n",x);
    return 0;
}

