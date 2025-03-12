#define _GNU_SOURCE // Requerido por gettid.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <dirent.h> // Requerido para manipulação e leitura de diretórios, usada por opendir(), readdir() e closedir().
#include <threads.h> // Requerido pela biblioteca de threads da C11 (Linguagem C versão 2011). Requerido por thrd_create() e thrd_exit().
#include <sys/types.h> // Requerido por pid_t e tid_t
#include <sys/wait.h> // Requerido por waitpid();
#include <unistd.h> // Requerido por getpid(), chdir() e fork()
#include <ctype.h> // Requerido para manipulação de caracteres, usada por isspace().
#include <errno.h> // Requerida para acesso a códigos de erro.
#include "clsc.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERR_INVALID_PARAMETER, argv[0]);
        return EXIT_FAILURE;
    }
    return clsc(argv[1]);
}

int clsc(const char* arg) {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    
    pid_t pid;

    int file_dir = fileOrDir(arg); // Analisa se o parâmetro é diretório ou arquivo.

    if (file_dir == -1) {
        printf(ERR_INVALID_PARAMETER, arg);
        return EXIT_FAILURE;
    }
    
    pid = fork();

    if (pid == -1) {
        puts(ERR_CREATE_PROCESS);
        return EXIT_FAILURE;
    }

    if (pid == 0)
        childProcess(arg, file_dir);
    else {
        int status;
        waitpid(pid, &status, 0);
        return EXIT_SUCCESS;
    }    
}

int childProcess(const char *arg, int file_dir) {      
    Files *f = NULL;
    Time t;
    time_t start, end; // Armazenará o tempo inicial e final.
    LineCount result = {0, 0, 0}; // Armazenará os resultados das somas das linhas de todos os arquivos.
    
    // Captura o tempo de início.
    start = getCurrentTime();

    if (file_dir == 2) // Caso o parâmetro seja um diretório.  
        f = executeThreadDirectory(arg);
    
    else // Caso o parâmetro seja um arquivo.  
        f = executeThreadFiles(arg);

    // Soma das Linhas.
    sumLines(f, &result);
    
    // Captura o tempo de término.
    end = getCurrentTime();

    calculateTime(&t, start, end);
    displayReport(*f, result, t);    
    freeMemory(f);
    return EXIT_SUCCESS;
}

Files* executeThreadDirectory(const char *dir) {
    int amountOfFiles = countCFiles(dir);
    thrd_t tids[amountOfFiles];
    ThreadArgs args[amountOfFiles];

    Files *f = initializeStructFiles(f, amountOfFiles); 
    
    readDirectory(f, dir);
    
    if (!f->amount_files) {
        printf(EMPTY_DIR);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < f->amount_files; i++) {
        /* Atribui o caminho do arquivo atual ao campo file_path da estrutura ThreadArgs correspondente.
           Garantindo que cada thread terá acesso ao caminho do arquivo específico que deve processar.
        */
        args[i].file_path = f->file_path[i]; 

        /* Associa o ponteiro para o elemento correspondente de array_files à estrutura ThreadArgs.
           Assim, cada thread poderá armazenar os resultados da contagem diretamente na posição correta do array.
        */
        args[i].array_files = &f->array_files[i]; 

        if (thrd_create(&tids[i], threadCountLines, &args[i]) != thrd_success) { 
            puts(ERR_CREATE_THTREAD);
            exit(EXIT_FAILURE); 
        }
    }
    
    // Aguarda as threads finalizarem.
    for (int i = 0; i < f->amount_files; i++) 
        thrd_join(tids[i], NULL);

    return f;
}

Files* executeThreadFiles(const char *file) {
    if (isCFile(file) == 0) {
        puts(ERR_INVALID_FILE);
        exit(EXIT_FAILURE);
    }        

    thrd_t tid;
    int amountOfFiles = 1;
    ThreadArgs args[amountOfFiles];

    Files *f = initializeStructFiles(f, amountOfFiles);

    f->file_path[0] = strdup(file);  // Duplica o nome do arquivo (parâmetro) para armazená-lo na estrtura Files.

    // Atribui o caminho do primeiro (e único) arquivo ao campo file_path da estrutura ThreadArgs na posição 0.
    args[0].file_path = f->file_path[0];

    // Atribui o ponteiro para o primeiro elemento de array_files à estrutura ThreadArgs na posição 0.
    args[0].array_files = &f->array_files[0]; 
    
    if (thrd_create(&tid, threadCountLines, &args[0]) != thrd_success) {
        puts(ERR_CREATE_THTREAD);
        exit(EXIT_FAILURE);
    }
    
    thrd_join(tid, NULL); 
    return f;
}

int threadCountLines(void *arg) {    
    ThreadArgs *args = (ThreadArgs*)arg; 

    FILE *file = fopen(args->file_path, "r");
    
    if (!file) {
        printf(ERR_OPEN_FILE, args->file_path);
        return EXIT_FAILURE;
    }

    Files *f = initializeStructFiles(f, 1);
    
    char line[MAX_LINE_LENGTH];
    int in_block_comment = 0; // Flag para identificar comentário de bloco.

    while (fgets(line, sizeof(line), file)) {
        int has_code = 0;    // Flag para detectar código na linha.
        int has_comment = 0; // Flag para detectar comentário na linha.
        int i = 0;           // Índice para percorrer a linha.

        // Ignora espaços em branco no início da linha.
        while (isspace(line[i])) i++;

        // Verifica se a linha é completamente vazia.
        if (line[i] == END_OF_LINE) {
            if (in_block_comment) 
                has_comment = 1; // Linhas vazias dentro de comentários de bloco são consideradas comentários.
            else {
                args->array_files->empty_lines++;
                continue;
            }
        }

        // Percorre a linha caracter por caracter.
        for (; line[i] != END_OF_LINE; i++) {
            if (in_block_comment) {
                // Detecta fim de comentário de bloco "*/".
                if (line[i] == ASTERISK && line[i + 1] == SLASH) {
                    in_block_comment = 0;
                    i++; // Avança para depois do fechamento "*/".
                }
                has_comment = 1; // Ainda dentro do comentário.
                continue;
            }

            // Detecta início de comentário de bloco "/*".
            if (line[i] == SLASH && line[i + 1] == ASTERISK) {
                in_block_comment = 1;
                has_comment = 1;
                i++; // Avança para depois do início "/*".
                continue;
            }

            // Detecta comentário de linha "//".
            if (line[i] == SLASH && line[i + 1] == SLASH) {
                has_comment = 1;
                break; // Ignora o restante da linha.
            }

            // Qualquer caractere não vazio e não comentário é código (Caso Padrão).
            if (!isspace(line[i])) {
                has_code = 1;
            }
        }

        // Classifica a linha com base nas flags.
        if (has_code) 
            args->array_files->code_lines++;

        else if (has_comment) 
            args->array_files->comment_lines++;

        else 
            args->array_files->empty_lines++;
    }

    fclose(file);
    thrd_exit(EXIT_SUCCESS);
}

int fileOrDir(const char *arg) { // Valida parâmetro.
    DIR* directory = opendir(arg);

    if (directory != NULL) {
        closedir(directory);
        return 2; // O argumento é um diretório.
    }

    if (errno == ENOTDIR) // ENOTDIR significa "Not a directory".
        return 1;

    return -1; // Erro.
}

Files* initializeStructFiles(Files *f, int amountOfFiles) {
    f = (Files*) calloc(1, sizeof(Files));
    f->file_path = (char**) calloc(amountOfFiles, sizeof(char*)); 
    f->array_files =  (LineCount*) calloc(amountOfFiles, sizeof(LineCount)); 
    f->amount_files = amountOfFiles;

    for(int i = 0; i < amountOfFiles; i++)
        f->file_path[i] = (char*) calloc(1, MAX_PATH_LENGTH * sizeof(char));

    return f;
}

void readDirectory(Files* f, const char *dirPath) { 
    static int index = 0; 
    struct dirent *entry; 

    DIR* dir = opendir(dirPath); 
    
    if (!dir) { 
        printf(ERR_OPEN_FILE, dirPath); 
        exit(EXIT_FAILURE); 
    } 
    
    while ((entry = readdir(dir)) != NULL) { 
        
        if (entry->d_type == DT_REG && isCFile(entry->d_name)) 
            snprintf(f->file_path[index++], MAX_PATH_LENGTH, "%s/%s", dirPath, entry->d_name); 

        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) { 
            char subDir[MAX_PATH_LENGTH]; 
            snprintf(subDir, sizeof(subDir), "%s/%s", dirPath, entry->d_name); 
            readDirectory(f, subDir); // Chamada recursiva para subdiretórios.
        } 
    } 
            
    closedir(dir); 
}

int countCFiles(const char *dirPath) {
    int quantity = 0;
    struct dirent *entry;
    
    DIR* dir = opendir(dirPath);

    if (!dir) {
        printf(ERR_OPEN_DIR, dirPath);
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_REG && isCFile(entry->d_name)) 
            quantity++;  

        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subDir[MAX_PATH_LENGTH];
            snprintf(subDir, sizeof(subDir), "%s/%s", dirPath, entry->d_name);
            quantity += countCFiles(subDir); // Chamada recursiva para subdiretórios.
        } 
    }
    closedir(dir);
    return quantity;
}

int isCFile(const char *filename) {
    // Verifica se o nome começa com '.' (arquivos ocultos).
    if (filename[0] == '.') 
        return 0;

    // Encontra a última ocorrência de '.' na string.
    const char *extension = strrchr(filename, '.'); 

    return (extension && strcmp(extension, C_FILE_EXTENSION) == 0);
}

void sumLines(Files *f, LineCount *result) {
    for (int i = 0; i < f->amount_files; i++) {
        result->empty_lines += f->array_files[i].empty_lines;
        result->comment_lines += f->array_files[i].comment_lines;
        result->code_lines += f->array_files[i].code_lines;
    }
}

time_t getCurrentTime() {
    time_t currentTime;

    // Obtém e retorna o tempo atual.
    return time(&currentTime);    
}

void calculateTime(Time *t, time_t start, time_t end) {
    // Formata o tempo para hora:minuto:segundo.
    strftime(t->start_time, sizeof(t->start_time), "%H:%M:%S", localtime(&start));
    strftime(t->end_time, sizeof(t->end_time), "%H:%M:%S", localtime(&end));

    // Calcula a duração corretamente (em segundos).
    t->duration = difftime(end, start);
}

void displayReport(Files f, const LineCount result, Time t) {
    printf(BLUE "\n\t- Código-fonte C\n" RESET);
    printf("%28s = %d\n", "Nº de Arquivos", f.amount_files);
    printf("%26s = %d\n", "Linhas vazias", result.empty_lines);
    printf("%35s = %d\n", "Linhas de comentários", result.comment_lines);
    printf("%35s = %d\n", "Linhas de instruções", result.code_lines);
    
    printf(BLUE "\n\t- Tempo\n" RESET);
    printf("%20s : %s\n", "Início", t.start_time);
    printf("%21s : %s\n", "Término", t.end_time);
    printf("%22s : %d segundos\n\n", "Duração", t.duration);
}

void freeMemory(Files *f) {
    for (int i = 0; i < f->amount_files; i++) 
        free(f->file_path[i]);

    free(f->array_files);
    free(f);
}
