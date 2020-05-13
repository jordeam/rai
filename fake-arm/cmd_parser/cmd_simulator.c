// INCLUDE <>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// INCLUDE ""
#include "generic_key_tree/rb_tree.h"

// DEFINE
#define CMD_LENGHT 32


// Contem os dados necessarios pra organizar e executar um comando
// exec     - funcao para chamar quando o comando for encontrado
// context  - ponteiro para dados relevantes para execucao da funcao (por exemplo, um ponteiro para o frame buffer de um display numa funcao de desenho)
typedef struct {
    void (*exec)(void *data, char *params);
    void *context;
    
}command_t;

// Executa toda a pipeline de processamento do comando
void run_cmd(RB_TREE *tree, char *args);


// Limpa o comando de caracteres estranhos e separa entre comando e argumentos
void sanitizer(char *in, char *cmd, char *args);


// comando aleatorios que eu inventei pro exemplo
void start_display_func(void *data, char *params);
void send_logs_func(void *data, char *params);
void end_program_func(void *data, char *params);


int main(){
    RB_TREE commands;       // cria a arvore de comandos
    rb_create(&commands);   // inicializa a arvore de busca
    commands.comparator = rb_comparator_string; // define que o tipo de chave vai ser string
    
    // enquanto for true o programa fica executando comandos
    uint_fast8_t program_running = 1;
    
    // CRIANDO ESTRUTURAS
    // display (disp_on)
    RB_NODE start_display;                          // o node eh inserido na arvore
    command_t start_display_struct;                 // a struct fica dentro do node, ela que recuperamos na busca
    
    start_display.data = &start_display_struct;     // logo salvamos o ponteiro da struct no node
    start_display.key = "disp_on";                  // a chave que quando selecionada retorna a struct
    
    start_display_struct.exec = start_display_func; // a funcao para ser chamada
    start_display_struct.context = NULL;            // o contexto da funcao
    
    
    // logs (broadcast)
    RB_NODE send_logs;
    command_t send_logs_struct;
    
    send_logs.data = &send_logs_struct;
    send_logs.key = "broadcast";
    
    send_logs_struct.exec = send_logs_func;
    send_logs_struct.context = (void *)stdout;
    
    // end program (quit)
    RB_NODE end_program;
    command_t end_program_struct;
    
    end_program.data = &end_program_struct;
    end_program.key = "quit";
    
    end_program_struct.exec = end_program_func;
    end_program_struct.context = &program_running;
    
    
    
    // inserir comandos
    // arg1: ponteiro pra arvore
    // arg2: ponteiro pro node
    rb_insert(&commands, &start_display);
    rb_insert(&commands, &send_logs);
    rb_insert(&commands, &end_program);
    
    // fica lendo o terminal pra detetar o comando
    char command_input[CMD_LENGHT];
    while(program_running){
        printf("Command: ");
        //scanf("%[^\n]s", command_input);
        fflush(stdin);
        fgets(command_input, CMD_LENGHT * sizeof(char), stdin);
        sscanf(command_input, "%[^\n]", command_input);
        printf("Raw input: %s\n", command_input);
        run_cmd(&commands, command_input);
        
    }
    
    
    return 0;
}


// A funcao responsavel por interpretar os comandos e executalos
void run_cmd(RB_TREE *tree, char *args){
    char clean_cmd[CMD_LENGHT];
    char clean_args[CMD_LENGHT];
    
    printf("Input: %s\n", args);
    sanitizer(args, clean_cmd, clean_args); // separa entre comando e argumentos e (atualmente nao) limpa 
    
    printf("CMD:   %s\n", clean_cmd);
    printf("Args:  %s\n", clean_args);
    
    // ONDE A MAGICA ACONTECE
    // essa funcao usa uma busca muito rapida e eficiente
    command_t *match = (command_t *)rb_get_data(tree, clean_cmd);
    
    // se nao encontrar nada, match fica como nullptr
    if(match){
        printf("----- Executing command -----\n\n");
        
        match->exec(match->context, clean_args);    // Chama a funcao do comando, com o contexto dela e os argumentos
        
        printf("\n----- Done -----\n\n");
    }
    else{
        printf("Command not found!\n");
    }
    
}

// atualmente so separa os tokens
void sanitizer(char *in, char *cmd, char *args){
    char *str;
    str = strtok(in, " \0");
    if(str)
        strcpy(cmd,  str);
    
    str = strtok(NULL, "\0");
    if(str)
        strcpy(args, str);
}

// printa na tela uma funcao padrao
void start_display_func(void *data, char *params){
    printf("Starting display!\n");
}

// printa no arquivo do contexto os argumentos, nesse exemplo eh o stdout
void send_logs_func(void *data, char *params){
    fprintf((FILE *)data, "%s\n", (char *)params);
}

// limpa a flag que segura o loop principal do programa
// a flag vem pelo contexto
void end_program_func(void *data, char *params){
    uint_fast8_t *ptr = (uint_fast8_t *)data;
    *ptr = 0;
}



 
