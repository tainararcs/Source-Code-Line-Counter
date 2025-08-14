/* Objetivos do programa clsc: 
   - Identificar e contar linhas vazias, linhas de comentários e linhas de código em arquivos de código-fonte C.
   - Gerar um relatório com as estatísticas coletadas.
*/

#pragma once

#include <time.h> // Requerida para manipulação de tempo, usadad por time(), localtime() e strftime().

// Tamanho máximo para o caminho de um arquivo ou diretório.
#define MAX_PATH_LENGTH 1024

// Tamanho máximo de uma linha lida de um arquivo.
#define MAX_LINE_LENGTH 1024

// Tamanho do formato de hora (hh:mm:ss).
#define MAX_HOUR_SIZE 9

// Definição para facilitar a verificação de arquivos com extensão .c.
#define C_FILE_EXTENSION ".c"

// Defiições para facilitar comparações no código.
#define SLASH '/'
#define ASTERISK '*'
#define END_OF_LINE '\0'

#define BLUE "\033[34m" // Sequência de escape ANSI para a cor azul.
#define RESET "\033[0m" // Sequência para resetar o estilo.

// Armazena a contagem de linhas de um arquivo ou conjunto de arquivos.
typedef struct {
   int empty_lines;   
   int comment_lines; 
   int code_lines; 
} LineCount;

/* Armazena os caminhos dos arquivos e suas contagens de linhas.
   Utilizada para gerenciar múltiplos arquivos e armazenar os resultados processados.
*/
typedef struct {
   char **file_path;       // Vetor de caminhos (nomes) dos arquivos.
   int amount_files;       // Quantidade de arquivos.
   LineCount *array_files; // Resultados de contagem por arquivo.
} Files;

// Armazena argumentos individuais passados para cada thread.
typedef struct { 
   LineCount *array_files; // Vetor de resultados de contagem.
   char *file_path;        // Caminho do arquivo que a thread irá processar.
} ThreadArgs;

// Armazena informações de tempo de execução.
typedef struct {
   char start_time[MAX_HOUR_SIZE];
   int duration;
   char end_time[MAX_HOUR_SIZE];
} Time;

// Mensagens de erro
#define ERR_INVALID_PARAMETER "\nErro: parâmetro inválido.\nInforme um arquivo ou diretório válido.\nUso: %s <arquivo_ou_diretorio>\n\n"
#define ERR_INVALID_FILE "\nErro: arquivo inválido.\nInforme um arquivo com extensão \'.c\'.\n"
#define ERR_CREATE_PROCESS "\nErro ao criar processo.\n"
#define ERR_CREATE_THTREAD "\nErro ao criar thread.\n"
#define ERR_OPEN_FILE "\nErro: não foi possível abrir o arquivo %s.\n"
#define ERR_OPEN_DIR "\nErro: não foi possível abrir o diretório %s.\n"
#define EMPTY_DIR "\nNenhum arquivo `.c` encontrado no diretório.\n\n"

/* Função principal do programa `clsc`.
   Lê o nome do arquivo ou diretório, cria o processo filho e aguarda até a contagem terminar.
 */
int clsc(const char*);

/* Responsável por criar o processo filho.
   Calcula as contagens de linhas de arquivos de código.
   Obtém os tempos inicial e final da execução e calcula a duração total.
*/
int childProcess(const char*arg, int file_dir);

/* Executa threads para processar arquivos em um diretório.
   Retorna uma estrutura contendo os resultados de contagem.
 */
Files* executeThreadDirectory(const char *dir);

/* Executa uma thread para processar um único arquivo.
   Retorna uma estrutura contendo os resultados de contagem.
 */
Files* executeThreadFiles(const char *file);

/* Função executada por cada thread para processar um arquivo C passado como parâmetro.
   - Realiza a contagem de linhas vazias, linhas de comentários e linhas de código.
   - Escreve os resultados na área de memória compartilhada.
   - Retorna se a thread foi executada com sucesso.
*/
int threadCountLines(void *file);

/* Verifica se o parâmetro fornecido pelo usuário é um arquivo ou diretório.
   Retorna 1 para arquivo, 2 para diretório ou 0 para inválido.
*/
int fileOrDir(const char *arg);

/* Inicializa a estrutura Files e aloca memória para os vetores internos.
   Retorna um ponteiro para a estrutra inicializada.
*/
Files* initializeStructFiles(Files *f, int amountOfFiles);

/* Processa todos os arquivos C dentro de um diretório.
   Armazena o nome dos arquivos econtrados no vetor file_path da estrutura Files. 
*/
void readDirectory(Files* f, const char *dir_path);

/* Conta o número de arquivos C dentro de um diretório.
   Retorna a quantidade de arquivos com extensão ".c".
*/
int countCFiles(const char *dir_path);

/* Verifica se um arquivo tem a extensão ".c".
   Retorna 1 em caso positivo, 0 caso contrário.
*/
int isCFile(const char *filename);

/* Percorre o vetor de estruturas que contém a quantidade de linhas de cada arquivo, somando os resultados.
   Armazena o resultado na estrutura result.
*/
void sumLines(Files *f, LineCount *result);

// Exibe o relatório no console com as contagens totais.
void displayReport(Files f, const LineCount result, Time t);

// Obtém o tempo atual e retorna um valor do tipo time_t representando o horário atual.
time_t getCurrentTime();

// Calcula e formata o tempo inicial, final e a duração em segundos entre dois timestamps (start e end).
void calculateTime(Time *t, time_t start, time_t end);

// Recebe um ponteiro para a struct Files e libera a região de momória.
void freeMemory(Files *f);

