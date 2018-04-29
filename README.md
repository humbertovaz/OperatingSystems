# Operating Systems
Work assignment for the Operating Systems class 

executa.c -> código que irá correr todos os comandos da shell
leNotebook -> Lê ficheiro de texto .nb (exemplo do enunciado) e processa o input, isto é, descarta texto "corrido" e envia comandos individualmente para o ficheiro executa.out


Para compilar:

make leNotebook
gcc executa.c -O executa.out


### Processamento de Notebooks
- [x] executa.c ("shell" para comandos (fork + exec))
- [x] leNotebook sem "pipes"
- [ ] leNotebook com "pipes