#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
Answers can be either in Portuguese or in English.
Respostas podem sem tanto em português como em inglês.
*/

/*
1. Term of commitment

The group members declare that all code developed for this project is their own.
The group members declare that they have not copied material from the Internet
  nor obtained code from third parties.

2. Group members and allocation of effort

Fill in the lines below with the name and email of the group members.
Replace XX with the contribution of each group member in the development of the work.

Mateus Cursino Gomes Costa <mateuscgcosta@ufmg.br> 50%
Lucas Emanuel Elias Alvees <lucaseliasalves@ufmg.br> 50%

3. Solutions

Briefly describe the solutions implemented for this project and justify their choices.

-TASK 1
Simplesmente chamamos a função fork() que por meio de uma chamada de sistema duplica
o processo atual, retornando para a variável id. Se algum erro resultar da chamada,
saímos do programa, caso o contrário, retornamos o id do processo filho.

-TASK 2
Chamamos a função execvp() que por meio de uma chamada de sistema substitui o processo atual.
No primeiro parâmetro, passamos o nome do programa e como segundo parâmetro o vetor de argumentos
para o programa a ser substituído.

-TASK 3
Inicialmente um arquivo é aberto para leitura ou escrita como parametrizado na função,
então é feito um switch entre "<" e ">", depois a tabela de arquivos do processo é alterada 
usando a funcao dup2 como visto em sala. Após o endereço de entrada ou saida do processo ser 
corretamente alterado o comando é executado usando runcmd(). Testes de erro são feitos após
abertura de arquivos e no switch.

-TASK 4
Um pipe é criado usando a função pipe() e o array passado por parâmetro, então um fork
é feito para cada comando do pipe e dentro delas o processo da funcão da esquerda tem seu 
indice de saida padrão alterado para extremidade de escrita do pipe e o processo da função 
da direita tem seu indice de entrada alterado para extremidade de leitura. Extremidades do pipe 
não utilizadas pelos processos são devidamente fechadas para criar um fluxo de dados unico. Os
respectivos comandos são executados no processo da direita e da esquerda e o processo pai espera pelo
fim dos processos filhos ao final da execução.

4. Bibliographic references

Abraham Silberschatz, Peter B. Galvin, Greg Gagne: Operating System Concepts Eighth (8th) Edition 

*/

/****************************************************************
 * Simplified xv6 Shell
 *
 * This code was adapted from the UNIX xv6 code and material from
 * the MIT Operating Systems course (6.828).
 ***************************************************************/

#define MAXARGS 10

/* Every command has a type. After identifying the command's type,
    the code converts a *cmd into the specific command type. */
struct cmd {
    int type; /* ' ' (exec)
                 '|' (pipe)
                 '<' or '>' (redirection) */
};

struct execcmd {
    int type;             // ' ' (exec)
    char *argv[MAXARGS];  // Arguments for the command to be executed
};

struct redircmd {
    int type;         // < or > (redirection)
    struct cmd *cmd;  // The command to execute (e.g., an execcmd)
    char *file;       // The input or output file
    int mode;         // The mode in which the file should be opened
    int fd;           // The file descriptor number to be used
};

struct pipecmd {
    int type;           // | (pipe)
    struct cmd *left;   // Left side of the pipe
    struct cmd *right;  // Right side of the pipe
};

int fork1(void);                                        // Fork but exit if an error occurs
struct cmd *parsecmd(char *);                           // Process the command line
void handle_simple_cmd(struct execcmd *ecmd);           // Handle simple commands
void handle_redirection(struct redircmd *rcmd);         // Handle redirection
void handle_pipe(struct pipecmd *pcmd, int *p, int r);  // Handle pipes

/* Execute the command cmd. It never returns. */
void runcmd(struct cmd *cmd) {
    int p[2], r;
    struct execcmd *ecmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0)
        exit(0);

    switch (cmd->type) {
        default:
            fprintf(stderr, "Unknown command type\n");
            exit(-1);

        case ' ':
            ecmd = (struct execcmd *)cmd;
            if (ecmd->argv[0] == 0)
                exit(0);
            handle_simple_cmd(ecmd);
            break;

        case '>':
        case '<':
            rcmd = (struct redircmd *)cmd;
            handle_redirection(rcmd);
            runcmd(rcmd->cmd);
            break;

        case '|':
            pcmd = (struct pipecmd *)cmd;
            handle_pipe(pcmd, p, r);
            break;
    }
    exit(0);
}

int fork1(void) {
    /* Task 1: Implement the fork1 function.
    The function is supposed to create a new process using the `fork()` system call.
    It should print a message if the fork fails, otherwise return the process ID of the child process (or -1 if the fork fails).
    */
    pid_t id = fork();
    if (id == -1){
        fprintf(stderr, "ERROR: fork failed\n");
        exit(-1);
    }
    return id;
    /* END OF TASK 1 */
}

void handle_simple_cmd(struct execcmd *ecmd) {
    /* Task 2: Implement the code below to execute simple commands. */
    execvp(ecmd->argv[0], ecmd->argv);
    /* END OF TASK 2 */
}

void handle_redirection(struct redircmd *rcmd) {
    /* Task 3: Implement the code below to handle input/output redirection. */
    int newFile = open(rcmd->file,rcmd->mode,777);
    if (dup2(newFile, rcmd->fd) < 0)
        fprintf(stderr, "ERROR: open file failed\n");
    else{
        switch (rcmd->type) {

            case '<':
                dup2(newFile,STDIN_FILENO);
                close(newFile);
                break;
    
            case '>':
                dup2(newFile,STDOUT_FILENO);
                close(newFile);
                break;
            
            default:
                fprintf(stderr, "ERROR:A redirect failed\n");
                exit(-1);
        }
        runcmd(rcmd->cmd);
    }
    /* END OF TASK 3 */
}

void handle_pipe(struct pipecmd *pcmd, int *p, int r) {
    /* Task 4: Implement the code below to handle pipes. */

    if (pipe(p) < 0) { //caso de erro
        fprintf(stderr,"Pipe failed");
        exit(-1);
    }


    if (fork1() == 0){ //filho 1
        close(p[0]);
        dup2(p[1],STDOUT_FILENO);
        runcmd(pcmd->left);
    }
    
    if(fork1() ==0 ){ //filho 2
        close(p[1]);
        dup2(p[0],STDIN_FILENO);
        runcmd(pcmd->right);
        
    }

    close(p[0]);
    close(p[1]);

    wait(&r);
    wait(&r);

    /* END OF TASK 4 */
}

int getcmd(char *buf, int nbuf) {
    if (isatty(fileno(stdin)))
        fprintf(stdout, "$ ");
    memset(buf, 0, nbuf);
    fgets(buf, nbuf, stdin);
    if (buf[0] == 0)  // EOF
        return -1;
    return 0;
}

int main(void) {
    static char buf[100];
    int r;

    // Read and execute commands.
    while (getcmd(buf, sizeof(buf)) >= 0) {
        /* Task 5: Explain the purpose of the if statement below and correct the error message.
        Why is the current error message incorrect? Justify the new message. */
        /* Answer:
            O if abaixo tem como propósito garantir que, no caso em que o comando realizado no terminal
            é "cd", o path fornecido como argumento realmente existe. A mensagem de erro original estava
            incorreta pois ela dizia que o processo não existia mas o erro não esta relacionado de formal 
            alguma a processos inexistentes e sim a ausencia do caminho fornecido como argumento. Desta forma,
            a nova mensagem está correta e informa o corretamente que o caminho de arquivos não existe. 
         */
        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
            buf[strlen(buf) - 1] = 0;
            if (chdir(buf + 3) < 0)
                fprintf(stderr, "file path does not exist\n");
            continue;
        }
        /* END OF TASK 5 */

        if (fork1() == 0)
            runcmd(parsecmd(buf));
        wait(&r);
    }
    exit(0);
}

/****************************************************************
 * Helper functions for creating command structures
 ***************************************************************/

struct cmd *
execcmd(void) {
    struct execcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = ' ';
    return (struct cmd *)cmd;
}

struct cmd *
redircmd(struct cmd *subcmd, char *file, int type) {
    struct redircmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->mode = (type == '<') ? O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC;
    cmd->fd = (type == '<') ? 0 : 1;
    return (struct cmd *)cmd;
}

struct cmd *
pipecmd(struct cmd *left, struct cmd *right) {
    struct pipecmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = '|';
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

/****************************************************************
 * Command Line Processing
 ***************************************************************/

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int gettoken(char **ps, char *es, char **q, char **eq) {
    char *s;
    int ret;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    if (q)
        *q = s;
    ret = *s;
    switch (*s) {
        case 0:
            break;
        case '|':
        case '<':
            s++;
            break;
        case '>':
            s++;
            break;
        default:
            ret = 'a';
            while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
                s++;
            break;
    }
    if (eq)
        *eq = s;

    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return ret;
}

int peek(char **ps, char *es, char *toks) {
    char *s = *ps;
    while (s < es && strchr(whitespace, *s)) s++;
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parsepipe(char **, char *);
struct cmd *parseexec(char **, char *);

/* Copy characters from the input buffer, starting from s to es.
 * Place a null terminator at the end to create a valid string. */
char *mkcopy(char *s, char *es) {
    int n = es - s;
    char *c = malloc(n + 1);
    assert(c);
    strncpy(c, s, n);
    c[n] = 0;
    return c;
}

struct cmd *
parsecmd(char *s) {
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    if (s != es) {
        fprintf(stderr, "leftovers: %s\n", s);
        exit(-1);
    }
    return cmd;
}

struct cmd *
parseline(char **ps, char *es) {
    struct cmd *cmd;
    cmd = parsepipe(ps, es);
    return cmd;
}

struct cmd *
parsepipe(char **ps, char *es) {
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|")) {
        gettoken(ps, es, 0, 0);
        cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
}

struct cmd *
parseredirs(struct cmd *cmd, char **ps, char *es) {
    int tok;
    char *q, *eq;

    while (peek(ps, es, "<>")) {
        tok = gettoken(ps, es, 0, 0);
        if (gettoken(ps, es, &q, &eq) != 'a') {
            fprintf(stderr, "missing file for redirection\n");
            exit(-1);
        }
        switch (tok) {
            case '<':
                cmd = redircmd(cmd, mkcopy(q, eq), '<');
                break;
            case '>':
                cmd = redircmd(cmd, mkcopy(q, eq), '>');
                break;
        }
    }
    return cmd;
}

struct cmd *
parseexec(char **ps, char *es) {
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    ret = execcmd();
    cmd = (struct execcmd *)ret;

    argc = 0;
    ret = parseredirs(ret, ps, es);
    while (!peek(ps, es, "|")) {
        if ((tok = gettoken(ps, es, &q, &eq)) == 0)
            break;
        if (tok != 'a') {
            fprintf(stderr, "syntax error\n");
            exit(-1);
        }
        cmd->argv[argc] = mkcopy(q, eq);
        argc++;
        if (argc >= MAXARGS) {
            fprintf(stderr, "too many args\n");
            exit(-1);
        }
        ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0;
    return ret;
}
