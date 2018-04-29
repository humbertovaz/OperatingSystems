#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
/* 
Lê ficheiro .nb (formato ficticio);  sempre que encontra um $, tem de executar o comando 
que se encontra a seguir e imprimir o output do programa da seguinte forma:
>>>
output
<<<

*/

//Ler linha a linha à procura de $; quando encontrar executa imprimindo o output no formato acima
//fgets(char * restrict str, int size, FILE * restrict stream);
int main(int argc, char const *argv[]){
    char **str = malloc(sizeof(char*)*20); 	// Alocar 20 apontadores
    char temp[100];
    char *token = malloc(sizeof(char)*100);
    FILE* file = fopen("teste.nb", "r");
    int i,x;
    char * linha;
    while(1){ 
			fgets(temp,100,file); //fgets já coloca o \0 no fim da string
			/// STRTOK
			temp[strlen(temp)-1] = '\0';
            linha = strdup(temp); // guardar linha para imprimir mais tarde 
			char *s = strdup(" "); // separator
   			/* get the first token */
   			token = strtok(temp, s);
			//token[strlen(token)] = '\0';
			i = 0;
			
			/* walk through other tokens */
			while( token != NULL) {
				str[i] = strdup(token);
				//token[strlen(token)] = '\0';
				token = strtok(NULL, s);
				i++;
			}

				/// Fim do STRTOK
				str[i] = NULL; // para o execvp saber que chegou ao fim dos argumentos
				if(strcmp(str[0],"$") != 0){
                    // Imprime a linha "não modificada" pelo strtok (temp);
                    printf("%s\n",linha);
                
                }else{
                    //Corre o programa "executa" para correr o programa em causa
                    x = fork();
                    if(x == 0){
                        free(str[0]); // libertar mem. do $ 
                        str[0] = strdup("executa.out");  // colocar o nome do programa executavel
                        execv(str[0],&(str[0]));
                        printf("Não devia imprimir isto\n");
                        exit(-1); //Exec correu mal
                    }else{
                        int status;
                        wait(0);
                        //WEXITSTATUS(status);
                        //printf("Esperei pelo meu filho! status = %d\n",status);
                        exit(0);
                    }
                }
				
			
		}
    return 0;
}
