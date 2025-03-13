# Counter-of-lines-of-source-code
Programa multithread chamado clsc (counter of lines of source code) que conta o número de linhas de código-fonte de programas escritos em Linguagem C, considerando linhas vazias, comentários e instruções


```markdown
# CLSC - Contador de Linhas de Código C

O projeto **CLSC** é uma ferramenta de contagem de linhas de código para arquivos fonte C. Ele pode contar o número de linhas de código, linhas de comentário e linhas vazias em arquivos `.c`, e também pode percorrer diretórios e subdiretórios para processar vários arquivos de uma vez.

## Funcionalidades

- Contagem de linhas de código:
  - Linhas de código (código executável).
  - Linhas de comentário (comentários de linha e de bloco).
  - Linhas vazias.

- Suporte para arquivos individuais ou diretórios inteiros contendo arquivos `.c`.
- Relatório com o total de arquivos processados e a contagem detalhada de linhas.
- Utiliza processos e threads para otimizar o processamento, especialmente para grandes volumes de arquivos.

## Requisitos

- **Compilador C** (como GCC).
- **Biblioteca C11** (para uso de threads).

## Como Usar

1. Clone este repositório para o seu computador:
   ```bash
   git clone https://github.com/SEU_USUARIO/clsc.git
   ```

2. Compile o código:
   ```bash
   gcc -o clsc clsc.c -pthread
   ```

3. Execute o programa, passando como argumento o caminho de um arquivo ou diretório:
   - Para um único arquivo:
     ```bash
     ./clsc arquivo.c
     ```
   - Para um diretório:
     ```bash
     ./clsc /caminho/para/diretorio
     ```

   O programa irá processar os arquivos `.c` dentro do diretório e seus subdiretórios, e irá exibir um relatório com a contagem das linhas de código, comentários e linhas vazias.

## Exemplo de Saída

```text
        - Código-fonte C
             Nº de Arquivos = 5
             Linhas vazias = 120
             Linhas de comentários = 320
             Linhas de instruções = 800

        - Tempo
             Início : 10:05:45
             Término : 10:06:10
             Duração : 25 segundos
```

## Estrutura do Projeto

- `clsc.h`: Cabeçalho com as definições e declarações de funções.
- `clsc.c`: Arquivo principal contendo o código-fonte do programa.


