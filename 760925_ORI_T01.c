/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização de Recuperação da Informação
 * Prof. Tiago A. Almeida
 *
 * Trabalho 01 - Indexação
 *
 * RA: 760925
 * Aluno: Gabriela Bergamo dos Santos
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

typedef enum {false, true} bool;

/* Tamanho dos campos dos registros */
/* Campos de tamanho fixo */
#define TAM_ID_USER 12
#define TAM_CELULAR 12
#define TAM_SALDO 14
#define TAM_DATE 9
#define TAM_ID_GAME 9
#define QTD_MAX_CATEGORIAS 3

/* Campos de tamanho variável (tamanho máximo) */
#define TAM_MAX_USER 48
#define TAM_MAX_TITULO 44
#define TAM_MAX_EMPRESA 48
#define TAM_MAX_EMAIL 42
#define TAM_MAX_CATEGORIA 20

#define MAX_REGISTROS 1000
#define TAM_REGISTRO_USUARIO (TAM_ID_USER+TAM_MAX_USER+TAM_MAX_EMAIL+TAM_SALDO+TAM_CELULAR)
#define TAM_REGISTRO_JOGO 256
#define TAM_REGISTRO_COMPRA (TAM_ID_USER+TAM_DATE+TAM_ID_GAME-3)
#define TAM_ARQUIVO_USUARIO (TAM_REGISTRO_USUARIO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGO (TAM_REGISTRO_JOGO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRA (TAM_REGISTRO_COMPRA * MAX_REGISTROS + 1)

#define TAM_RRN_REGISTRO 4
#define TAM_CHAVE_USUARIOS_IDX (TAM_ID_USER + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_JOGOS_IDX (TAM_ID_GAME + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_COMPRAS_IDX (TAM_ID_USER + TAM_ID_GAME + TAM_RRN_REGISTRO - 2)
#define TAM_CHAVE_TITULO_IDX (TAM_MAX_TITULO + TAM_ID_GAME - 2)
#define TAM_CHAVE_DATA_USER_GAME_IDX (TAM_DATE + TAM_ID_USER + TAM_ID_GAME - 3)
#define TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX (TAM_MAX_CATEGORIA - 1)
#define TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX (TAM_ID_GAME - 1)

#define TAM_ARQUIVO_USUARIOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRAS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CATEGORIAS_IDX (1000 * MAX_REGISTROS + 1)

/* Mensagens padrões */
#define SUCESSO                          "OK\n"
#define REGS_PERCORRIDOS                "Registros percorridos:"
#define AVISO_NENHUM_REGISTRO_ENCONTRADO "AVISO: Nenhum registro encontrado\n"
#define ERRO_OPCAO_INVALIDA              "ERRO: Opcao invalida\n"
#define ERRO_MEMORIA_INSUFICIENTE        "ERRO: Memoria insuficiente\n"
#define ERRO_PK_REPETIDA                 "ERRO: Ja existe um registro com a chave %s\n"
#define ERRO_REGISTRO_NAO_ENCONTRADO     "ERRO: Registro nao encontrado\n"
#define ERRO_SALDO_NAO_SUFICIENTE        "ERRO: Saldo insuficiente\n"
#define ERRO_CATEGORIA_REPETIDA          "ERRO: O jogo %s ja possui a categoria %s\n"
#define ERRO_VALOR_INVALIDO              "ERRO: Valor invalido\n"
#define ERRO_ARQUIVO_VAZIO               "ERRO: Arquivo vazio\n"
#define ERRO_NAO_IMPLEMENTADO            "ERRO: Funcao %s nao implementada\n"

/* Registro de Usuario */
typedef struct {
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    double saldo;
} Usuario;

/* Registro de Jogo */
typedef struct {
    char id_game[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char data_lancamento[TAM_DATE];
    double preco;
    char categorias[QTD_MAX_CATEGORIAS][TAM_MAX_CATEGORIA];
} Jogo;

/* Registro de Compra */
typedef struct {
    char id_user_dono[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    char data_compra[TAM_DATE];
} Compra;


/*----- Registros dos índices -----*/

/* Struct para o índice primário dos usuários */
typedef struct {
    char id_user[TAM_ID_USER];
    int rrn;
} usuarios_index;

/* Struct para o índice primário dos jogos */
typedef struct {
    char id_game[TAM_ID_GAME];
    int rrn;
} jogos_index;

/* Struct para índice primário dos compras */
typedef struct {
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    int rrn;
} compras_index;

/* Struct para o índice secundário dos titulos */
typedef struct {
    char titulo[TAM_MAX_TITULO];
    char id_game[TAM_ID_GAME];
} titulos_index;

/* Struct para o índice secundário das datas das compras */
typedef struct {
    char data[TAM_DATE];
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
} data_user_game_index;

/* Struct para o índice secundário das categorias (lista invertida) */
typedef struct {
    char chave_secundaria[TAM_MAX_CATEGORIA];   //string com o nome da categoria
    int primeiro_indice;
} categorias_secundario_index;

/* Struct para o índice primário das categorias (lista invertida) */
typedef struct {
    char chave_primaria[TAM_ID_GAME];   //string com o id do jogo
    int proximo_indice;
} categorias_primario_index;

/* Struct para os parâmetros de uma lista invertida */
typedef struct {
    // Ponteiro para o índice secundário
    categorias_secundario_index *categorias_secundario_idx;

    // Ponteiro para o arquivo de índice primário
    categorias_primario_index *categorias_primario_idx;

    // Quantidade de registros de índice secundário
    unsigned qtd_registros_secundario;

    // Quantidade de registros de índice primário
    unsigned qtd_registros_primario;

    // Tamanho de uma chave secundária nesse índice
    unsigned tam_chave_secundaria;

    // Tamanho de uma chave primária nesse índice
    unsigned tam_chave_primaria;

    // Função utilizada para comparar as chaves do índice secundário.
    // Igual às funções de comparação do bsearch e qsort.
    int (*compar)(const void *key, const void *elem);
} inverted_list;

/* Variáveis globais */
/* Arquivos de dados */
char ARQUIVO_USUARIOS[TAM_ARQUIVO_USUARIO];
char ARQUIVO_JOGOS[TAM_ARQUIVO_JOGO];
char ARQUIVO_COMPRAS[TAM_ARQUIVO_COMPRA];

/* Índices */
usuarios_index *usuarios_idx = NULL;
jogos_index *jogos_idx = NULL;
compras_index *compras_idx = NULL;
titulos_index *titulo_idx = NULL;
data_user_game_index *data_user_game_idx = NULL;
inverted_list categorias_idx = {
    .categorias_secundario_idx = NULL,
    .categorias_primario_idx = NULL,
    .qtd_registros_secundario = 0,
    .qtd_registros_primario = 0,
    .tam_chave_secundaria = TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX,
    .tam_chave_primaria = TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX,
};

/* Funções auxiliares para o qsort.
 * Com uma pequena alteração, é possível utilizá-las no bsearch, assim, evitando código duplicado.
 * */
int qsort_usuarios_idx(const void *a, const void *b);
int qsort_jogos_idx(const void *a, const void *b);
int qsort_compras_idx(const void *a, const void *b);
int qsort_titulo_idx(const void *a, const void *b);
int qsort_data_user_game_idx(const void *a, const void *b);
int qsort_data_idx(const void *a, const void *b);
int qsort_categorias_secundario_idx(const void *a, const void *b);

/* Contadores */
unsigned qtd_registros_usuarios = 0;
unsigned qtd_registros_jogos = 0;
unsigned qtd_registros_compras = 0;

/* Funções de geração determinística de números pseudo-aleatórios */
uint64_t prng_seed;

void prng_srand(uint64_t value) {
    prng_seed = value;
}

uint64_t prng_rand() {
    // https://en.wikipedia.org/wiki/Xorshift#xorshift*
    uint64_t x = prng_seed; // O estado deve ser iniciado com um valor diferente de 0
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    prng_seed = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Funções de manipulação de data */
int64_t epoch;

void set_time(int64_t value) {
    epoch = value;
}

void tick_time() {
    epoch += prng_rand() % 864000; // 10 dias
}

struct tm gmtime_(const int64_t lcltime) {
    // based on https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/time/gmtime_r.c;
    struct tm res;
    long days = lcltime / 86400 + 719468;
    long rem = lcltime % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }

    res.tm_hour = (int) (rem / 3600);
    rem %= 3600;
    res.tm_min = (int) (rem / 60);
    res.tm_sec = (int) (rem % 60);

    int weekday = (3 + days) % 7;
    if (weekday < 0) weekday += 7;
    res.tm_wday = weekday;

    int era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned long eraday = days - era * 146097;
    unsigned erayear = (eraday - eraday / 1460 + eraday / 36524 - eraday / 146096) / 365;
    unsigned yearday = eraday - (365 * erayear + erayear / 4 - erayear / 100);
    unsigned month = (5 * yearday + 2) / 153;
    unsigned day = yearday - (153 * month + 2) / 5 + 1;
    month += month < 10 ? 2 : -10;

    int isleap = ((erayear % 4) == 0 && (erayear % 100) != 0) || (erayear % 400) == 0;
    res.tm_yday = yearday >= 306 ? yearday - 306 : yearday + 59 + isleap;
    res.tm_year = (erayear + era * 400 + (month <= 1)) - 1900;
    res.tm_mon = month;
    res.tm_mday = day;
    res.tm_isdst = 0;

    return res;
}

/**
 * Escreve a <i>data</i> atual no formato <code>AAAAMMDD</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATE];<br />
 * current_date(date);<br />
 * printf("data atual: %s&#92;n", date);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATE</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_date(char buffer[TAM_DATE]) {
    // http://www.cplusplus.com/reference/ctime/strftime/
    // http://www.cplusplus.com/reference/ctime/gmtime/
    // AAAA MM DD
    // %Y   %m %d
    struct tm tm_ = gmtime_(epoch);
    strftime(buffer, TAM_DATE, "%Y%m%d", &tm_);
}

/* Remove comentários (--) e caracteres whitespace do começo e fim de uma string */
void clear_input(char *str) {
    char *ptr = str;
    int len = 0;

    for (; ptr[len]; ++len) {
        if (strncmp(&ptr[len], "--", 2) == 0) {
            ptr[len] = '\0';
            break;
        }
    }

    while(len-1 > 0 && isspace(ptr[len-1]))
        ptr[--len] = '\0';

    while(*ptr && isspace(*ptr))
        ++ptr, --len;

    memmove(str, ptr, len + 1);
}


/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Cria o índice respectivo */
void criar_usuarios_idx();
void criar_jogos_idx();
void criar_compras_idx();
void criar_titulo_idx();
void criar_data_user_game_idx();
void criar_categorias_idx();

/* Exibe um registro com base no RRN */
bool exibir_usuario(int rrn);
bool exibir_jogo(int rrn);
bool exibir_compra(int rrn);

/* Recupera do arquivo o registro com o RRN informado
 * e retorna os dados nas structs Usuario, Jogo e Compra */
Usuario recuperar_registro_usuario(int rrn);
Jogo recuperar_registro_jogo(int rrn);
Compra recuperar_registro_compra(int rrn);

/* Escreve em seu respectivo arquivo na posição informada (RRN) */
void escrever_registro_usuario(Usuario u, int rrn);
void escrever_registro_jogo(Jogo j, int rrn);
void escrever_registro_compra(Compra c, int rrn);

/* Funções principais */
void cadastrar_usuario_menu(char* id_user, char* username, char* email);
void cadastrar_celular_menu(char* id_user, char* celular);
void remover_usuario_menu(char *id_user);
void cadastrar_jogo_menu(char* titulo, char* desenvolvedor, char* editora, char* lancamento, double preco);
void adicionar_saldo_menu(char* id_user, double valor);
void comprar_menu(char* id_user, char* titulo);
void cadastrar_categoria_menu(char* titulo, char* categoria);

/* Busca */
void buscar_usuario_id_user_menu(char *id_user);
void buscar_jogo_id_menu(char *id_game);
void buscar_jogo_titulo_menu(char *titulo);

/* Listagem */
void listar_usuarios_id_user_menu();
void listar_jogos_categorias_menu(char *categoria);
void listar_compras_periodo_menu(char *data_inicio, char *data_fim);

/* Liberar espaço */
void liberar_espaco_menu();

/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu();
void imprimir_arquivo_jogos_menu();
void imprimir_arquivo_compras_menu();

/* Imprimir índices primários */
void imprimir_usuarios_idx_menu();
void imprimir_jogos_idx_menu();
void imprimir_compras_idx_menu();

/* Imprimir índices secundários */
void imprimir_titulo_idx_menu();
void imprimir_data_user_game_idx_menu();
void imprimir_categorias_secundario_idx_menu();
void imprimir_categorias_primario_idx_menu();

/* Liberar memória e encerrar programa */
void liberar_memoria_menu();

/* Funções de manipulação de Lista Invertida */
/**
 * Responsável por inserir duas chaves (chave_secundaria e chave_primaria) em uma Lista Invertida (t).<br />
 * Atualiza os parâmetros dos índices primário e secundário conforme necessário.<br />
 * As chaves a serem inseridas devem estar no formato correto e com tamanho t->tam_chave_primario e t->tam_chave_secundario.<br />
 * O funcionamento deve ser genérico para qualquer Lista Invertida, adaptando-se para os diferentes parâmetros presentes em seus structs.<br />
 *
 * @param chave_secundaria Chave a ser buscada (caso exista) ou inserida (caso não exista) no registro secundário da Lista Invertida.
 * @param chave_primaria Chave a ser inserida no registro primário da Lista Invertida.
 * @param t Ponteiro para a Lista Invertida na qual serão inseridas as chaves.
 */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t);

/**
 * Responsável por buscar uma chave no índice secundário de uma Lista invertida (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice inicial das chaves no registro primário.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = inverted_list_secondary_search(NULL, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, true, categoria, &categorias_idx);<br />
 * </code>
 *
 * @param result Ponteiro para ser escrito o índice inicial (primeira ocorrência) das chaves do registro primário. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave_secundaria Chave a ser buscada na Árvore-B.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t);

/**
 * Responsável por percorrer o índice primário de uma Lista invertida (T). O valor de retorno indica a quantidade de chaves encontradas.
 * O ponteiro para o vetor de strings result pode ser fornecido opcionalmente, e será populado com a lista de todas as chaves encontradas.
 * O ponteiro para o inteiro indice_final também pode ser fornecido opcionalmente, e deve conter o índice do último campo da lista encadeada 
 * da chave primaria fornecida (isso é útil na inserção de um novo registro).<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. As chaves encontradas deverão ser retornadas e tanto o caminho quanto o indice_final não devem ser informados.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, false, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse nas chaves encontradas, apenas no indice_final, e o caminho não deve ser informado.<br />
 * ...<br />
 * int indice_final;
 * int qtd = inverted_list_primary_search(NULL, false, indice, &indice_final, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse nas chaves encontradas e no caminho feito.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, true, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * </code>
 *
 * @param result Ponteiro para serem escritas as chaves encontradas. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param indice Índice do primeiro registro da lista encadeada a ser procurado.
 * @param indice_final Ponteiro para ser escrito o índice do último registro encontrado (cujo campo indice é -1). É ignorado caso NULL.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica a quantidade de chaves encontradas.
 */
int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t);

/**
 * Preenche uma string str com o caractere pad para completar o tamanho size.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 * @param pad Caractere utilizado para fazer o preenchimento à direita.
 * @param size Tamanho desejado para a string.
 */
char* strpadright(char *str, char pad, unsigned size);

/* Funções de busca binária */
/**
 * Função Genérica de busca binária, que aceita parâmetros genéricos (assinatura baseada na função bsearch da biblioteca C).
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou NULL se não encontrou.
 */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho);
/**
 * Função Genérica de busca binária que encontra o elemento de BAIXO mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de BAIXO mais próximo.
 */
void* busca_binaria_piso(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/**
 * Função Genérica de busca binária que encontra o elemento de CIMA mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de CIMA mais próximo.
 */
void* busca_binaria_teto(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/* <<< COLOQUE AQUI OS DEMAIS PROTÓTIPOS DE FUNÇÕES, SE NECESSÁRIO >>> */


/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */

int main() {
    // variáveis utilizadas pelo interpretador de comandos
    char input[500];
    uint64_t seed = 2;
    uint64_t time = 1616077800; // UTC 18/03/2021 14:30:00
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    char id[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char lancamento[TAM_DATE];
    char categoria[TAM_MAX_CATEGORIA];
    double valor;
    char data_inicio[TAM_DATE];
    char data_fim[TAM_DATE];

    scanf("SET ARQUIVO_USUARIOS '%[^\n]\n", ARQUIVO_USUARIOS);
    int temp_len = strlen(ARQUIVO_USUARIOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_usuarios = (temp_len - 2) / TAM_REGISTRO_USUARIO;
    ARQUIVO_USUARIOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_JOGOS '%[^\n]\n", ARQUIVO_JOGOS);
    temp_len = strlen(ARQUIVO_JOGOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_jogos = (temp_len - 2) / TAM_REGISTRO_JOGO;
    ARQUIVO_JOGOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_COMPRAS '%[^\n]\n", ARQUIVO_COMPRAS);
    temp_len = strlen(ARQUIVO_COMPRAS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_compras = (temp_len - 2) / TAM_REGISTRO_COMPRA;
    ARQUIVO_COMPRAS[temp_len - 2] = '\0';

    // inicialização do gerador de números aleatórios e função de datas
    prng_srand(seed);
    set_time(time);

    criar_usuarios_idx();
    criar_jogos_idx();
    criar_compras_idx();
    criar_titulo_idx();
    criar_data_user_game_idx();
    criar_categorias_idx();

    while (1) {
        fgets(input, 500, stdin);
        printf("%s", input);
        clear_input(input);

        if (strcmp("", input) == 0)
            continue; // não avança o tempo nem imprime o comando este seja em branco

        /* Funções principais */
        if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']');", id_user, username, email) == 3)
            cadastrar_usuario_menu(id_user, username, email);
        else if (sscanf(input, "UPDATE usuarios SET celular = '%[^']' WHERE id_user = '%[^']';", celular, id_user) == 2)
            cadastrar_celular_menu(id_user, celular);
        else if (sscanf(input, "DELETE FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            remover_usuario_menu(id_user);
        else if (sscanf(input, "INSERT INTO jogos VALUES ('%[^']', '%[^']', '%[^']', '%[^']', %lf);", titulo, desenvolvedor, editora, lancamento, &valor) == 5)
            cadastrar_jogo_menu(titulo, desenvolvedor, editora, lancamento, valor);
        else if (sscanf(input, "UPDATE usuarios SET saldo = saldo + %lf WHERE id_user = '%[^']';", &valor, id_user) == 2)
            adicionar_saldo_menu(id_user, valor);
        else if (sscanf(input, "INSERT INTO compras VALUES ('%[^']', '%[^']');", id_user, titulo) == 2)
            comprar_menu(id_user, titulo);
        else if (sscanf(input, "UPDATE jogos SET categorias = array_append(categorias, '%[^']') WHERE titulo = '%[^']';", categoria, titulo) == 2)
            cadastrar_categoria_menu(titulo, categoria);

        /* Busca */
        else if (sscanf(input, "SELECT * FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            buscar_usuario_id_user_menu(id_user);
        else if (sscanf(input, "SELECT * FROM jogos WHERE id_game = '%[^']';", id) == 1)
            buscar_jogo_id_menu(id);
        else if (sscanf(input, "SELECT * FROM jogos WHERE titulo = '%[^']';", titulo) == 1)
            buscar_jogo_titulo_menu(titulo);

        /* Listagem */
        else if (strcmp("SELECT * FROM usuarios ORDER BY id_user ASC;", input) == 0)
            listar_usuarios_id_user_menu();
        else if (sscanf(input, "SELECT * FROM jogos WHERE '%[^']' = ANY (categorias) ORDER BY titulo ASC;", categoria) == 1)
            listar_jogos_categorias_menu(categoria);
        else if (sscanf(input, "SELECT * FROM compras WHERE data_compra BETWEEN '%[^']' AND '%[^']' ORDER BY data_compra ASC;", data_inicio, data_fim) == 2)
            listar_compras_periodo_menu(data_inicio, data_fim);

        /* Liberar espaço */
        else if (strcmp("VACUUM usuarios;", input) == 0)
            liberar_espaco_menu();

        /* Imprimir arquivos de dados */
        else if (strcmp("\\echo file ARQUIVO_USUARIOS", input) == 0)
            imprimir_arquivo_usuarios_menu();
        else if (strcmp("\\echo file ARQUIVO_JOGOS", input) == 0)
            imprimir_arquivo_jogos_menu();
        else if (strcmp("\\echo file ARQUIVO_COMPRAS", input) == 0)
            imprimir_arquivo_compras_menu();
        
        /* Imprimir índices primários */
        else if (strcmp("\\echo index usuarios_idx", input) == 0)
            imprimir_usuarios_idx_menu();
        else if (strcmp("\\echo index jogos_idx", input) == 0)
            imprimir_jogos_idx_menu();
        else if (strcmp("\\echo index compras_idx", input) == 0)
            imprimir_compras_idx_menu();

        /* Imprimir índices secundários */
        else if (strcmp("\\echo index titulo_idx", input) == 0)
            imprimir_titulo_idx_menu();
        else if (strcmp("\\echo index data_user_game_idx", input) == 0)
            imprimir_data_user_game_idx_menu();
        else if (strcmp("\\echo index categorias_secundario_idx", input) == 0)
            imprimir_categorias_secundario_idx_menu();
        else if (strcmp("\\echo index categorias_primario_idx", input) == 0)
            imprimir_categorias_primario_idx_menu();

        /* Liberar memória eventualmente alocada e encerrar programa */
        else if (strcmp("\\q", input) == 0)
            { liberar_memoria_menu(); return 0; }
        else if (sscanf(input, "SET SRAND %lu;", &seed) == 1)
            { prng_srand(seed); printf(SUCESSO); continue; }
        else if (sscanf(input, "SET TIME %lu;", &time) == 1)
            { set_time(time); printf(SUCESSO); continue; }
        else
            printf(ERRO_OPCAO_INVALIDA);

        tick_time();
    }
}

/* ========================================================================== */

/* Cria o índice primário usuarios_idx */
void criar_usuarios_idx() {
    if (!usuarios_idx)
        usuarios_idx = malloc(MAX_REGISTROS * sizeof(usuarios_index));

    if (!usuarios_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
        Usuario u = recuperar_registro_usuario(i);

        if (strncmp(u.id_user, "*|", 2) == 0)
            usuarios_idx[i].rrn = -1; // registro excluído
        else
            usuarios_idx[i].rrn = i;

        strcpy(usuarios_idx[i].id_user, u.id_user);
    }

    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
}

/* Cria o índice primário jogos_idx */
void criar_jogos_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if (!jogos_idx)
        jogos_idx = malloc(MAX_REGISTROS * sizeof(jogos_index));

    if (!jogos_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo u = recuperar_registro_jogo(i);

        if (strncmp(u.id_game, "*|", 2) == 0)
            jogos_idx[i].rrn = -1; // registro excluído
        else
            jogos_idx[i].rrn = i;

        strcpy(jogos_idx[i].id_game, u.id_game);
    }

    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);

    // printf(ERRO_NAO_IMPLEMENTADO, "criar_jogos_idx");
}

/* Cria o índice primário compras_idx */
void criar_compras_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    // if (!jogos_idx)
    //     criar_jogos_idx();

    // if (!usuarios_idx)
    //     criar_usuarios_idx();

    if(!compras_idx)
        compras_idx= malloc(MAX_REGISTROS * sizeof(compras_index));  

    if (!compras_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }      

    for (unsigned i = 0; i < qtd_registros_compras; ++i) {
        Compra u = recuperar_registro_compra(i);

        if (strncmp(u.id_game, "*|", 2) == 0 || strncmp(u.id_user_dono, "*|", 2) == 0)
            compras_idx[i].rrn = -1; // registro excluído
        else
            compras_idx[i].rrn = i;

        strcpy(compras_idx[i].id_game, u.id_game);
        strcpy(compras_idx[i].id_user, u.id_user_dono);
    }

    qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx);    

    //printf(ERRO_NAO_IMPLEMENTADO, "criar_compras_idx");
}

/* Cria o índice secundário titulo_idx */
void criar_titulo_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(!titulo_idx)
    titulo_idx= malloc(MAX_REGISTROS * sizeof(titulos_index));  

    if (!titulo_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    } 

    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo u = recuperar_registro_jogo(i);

        if (strncmp(u.id_game, "*|", 2) != 0){
            strcpy(titulo_idx[i].id_game, u.id_game);
            strcpy(titulo_idx[i].titulo, u.titulo);
        }  
    }
    qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);    
    // printf(ERRO_NAO_IMPLEMENTADO, "criar_titulo_idx");
}

/* Cria o índice secundário data_user_game_idx */
void criar_data_user_game_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */ 

    if(!data_user_game_idx)
        data_user_game_idx= malloc(MAX_REGISTROS * sizeof(data_user_game_index));  

    if (!data_user_game_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    } 

    for (unsigned i = 0; i < qtd_registros_compras; ++i) {
        Compra u = recuperar_registro_compra(i);

        if (strncmp(u.id_game, "*|", 2) != 0){
            strcpy(data_user_game_idx[i].id_game, u.id_game);
            strcpy(data_user_game_idx[i].id_user, u.id_user_dono);
            strcpy(data_user_game_idx[i].data, u.data_compra);
        }  
    }
    qsort(data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx);   


    // printf(ERRO_NAO_IMPLEMENTADO, "criar_data_user_game_idx");
}

/* Cria os índices (secundário e primário) de categorias_idx */
void criar_categorias_idx() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    /*inicializar lista*/
    if(!categorias_idx.categorias_primario_idx)
    categorias_idx.categorias_primario_idx = (categorias_primario_index*)malloc(MAX_REGISTROS * sizeof(categorias_primario_index));

    if (!categorias_idx.categorias_primario_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    } 

    if(!categorias_idx.categorias_secundario_idx)
    categorias_idx.categorias_secundario_idx = (categorias_secundario_index*)malloc(MAX_REGISTROS * sizeof(categorias_secundario_index));

    if (!categorias_idx.categorias_secundario_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    } 

    /*for com qtd de jogos -> jogo j = recuperar jogo -> for dentro das cat dele -> if -> busca lista invertida*/

    for(unsigned i = 0; i < qtd_registros_jogos; i++){
        Jogo j = recuperar_registro_jogo(i);

        for(unsigned g = 0; g < QTD_MAX_CATEGORIAS; g++){
            if(*j.categorias[g] != '\0'){
                inverted_list_insert(j.categorias[g], j.id_game, &categorias_idx);
            }
        }
    }
   //printf(ERRO_NAO_IMPLEMENTADO, "criar_categorias_idx");
}


/* Exibe um usuario dado seu RRN */
bool exibir_usuario(int rrn) {
    if (rrn < 0)
        return false;

    Usuario u = recuperar_registro_usuario(rrn);

    printf("%s, %s, %s, %s, %.2lf\n", u.id_user, u.username, u.email, u.celular, u.saldo);
    return true;
}

/* Exibe um jogo dado seu RRN */
bool exibir_jogo(int rrn) {
    if (rrn < 0)
        return false;

    Jogo j = recuperar_registro_jogo(rrn);

    printf("%s, %s, %s, %s, %s, %.2lf\n", j.id_game, j.titulo, j.desenvolvedor, j.editora, j.data_lancamento, j.preco);
    return true;
}

/* Exibe uma compra dado seu RRN */
bool exibir_compra(int rrn) {
    if (rrn < 0)
        return false;

    Compra c = recuperar_registro_compra(rrn);

    printf("%s, %s, %s\n", c.id_user_dono, c.data_compra, c.id_game);

    return true;
}


/* Recupera do arquivo de usuários o registro com o RRN
 * informado e retorna os dados na struct Usuario */
Usuario recuperar_registro_usuario(int rrn) {
    Usuario u;
    char temp[TAM_REGISTRO_USUARIO + 1], *p;
    strncpy(temp, ARQUIVO_USUARIOS + (rrn * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
    temp[TAM_REGISTRO_USUARIO] = '\0';

    p = strtok(temp, ";");
    strcpy(u.id_user, p);
    p = strtok(NULL, ";");
    strcpy(u.username, p);
    p = strtok(NULL, ";");
    strcpy(u.email, p);
    p = strtok(NULL, ";");
    strcpy(u.celular, p);
    p = strtok(NULL, ";");
    u.saldo = atof(p);
    p = strtok(NULL, ";");

    return u;
}

/* Recupera do arquivo de jogos o registro com o RRN
 * informado e retorna os dados na struct Jogo */
Jogo recuperar_registro_jogo(int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    Jogo j;
    char temp[TAM_REGISTRO_JOGO + 1], *p , *q;
    strncpy(temp, ARQUIVO_JOGOS + (rrn * TAM_REGISTRO_JOGO), TAM_REGISTRO_JOGO);
    temp[TAM_REGISTRO_JOGO] = '\0';
    p = strtok(temp, ";");
    strcpy(j.id_game, p);
    p = strtok(NULL, ";");
    strcpy(j.titulo, p);
    p = strtok(NULL, ";");
    strcpy(j.desenvolvedor, p);
    p = strtok(NULL, ";");
    strcpy(j.editora, p);
    p = strtok(NULL, ";");
    strcpy(j.data_lancamento, p);
    p = strtok(NULL, ";");
    j.preco = atof(p);
    p = strtok(NULL, ";");
    q = strtok(p, "|");
    for(int i = 0; i < QTD_MAX_CATEGORIAS; i++){
        if(q == NULL || strncmp(q, "#", 1) == 0 ){
            *j.categorias[i] = '\0';
        } else {
            strcpy(j.categorias[i], q);
        }
        q = strtok(NULL, "|");
    }
    p = strtok(NULL, ";");
    // strcpy("", p);
    // p = strtok(NULL, ";");
    return j;

    //printf(ERRO_NAO_IMPLEMENTADO, "recuperar_registro_jogo");
}

/* Recupera do arquivo de compras o registro com o RRN
 * informado e retorna os dados na struct Compra */
Compra recuperar_registro_compra(int rrn) {     
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    Compra c;
    char temp[TAM_REGISTRO_COMPRA + 1];
    strncpy(temp, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA), TAM_REGISTRO_COMPRA);
    //temp[TAM_REGISTRO_COMPRA] = '\0';

    strncpy(c.id_user_dono, temp, TAM_ID_USER);
    c.id_user_dono[TAM_ID_USER-1]= '\0';
    strncpy(c.data_compra, temp+TAM_ID_USER-1, TAM_DATE);
    c.data_compra[TAM_DATE-1] = '\0';
    strncpy(c.id_game, temp+TAM_ID_USER+TAM_DATE-2, TAM_ID_GAME);
    c.id_game[TAM_ID_GAME-1] = '\0';

    return c;

    //printf(ERRO_NAO_IMPLEMENTADO, "recuperar_registro_compra");
}


/* Escreve no arquivo de usuários na posição informada (RRN)
 * os dados na struct Usuario */
void escrever_registro_usuario(Usuario u, int rrn) {
    char temp[TAM_REGISTRO_USUARIO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, u.id_user);
    strcat(temp, ";");
    strcat(temp, u.username);
    strcat(temp, ";");
    strcat(temp, u.email);
    strcat(temp, ";");
    strcat(temp, u.celular);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", u.saldo);
    strcat(temp, p);
    strcat(temp, ";");
    
    for (int i = strlen(temp); i < TAM_REGISTRO_USUARIO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_USUARIOS + rrn*TAM_REGISTRO_USUARIO, temp, TAM_REGISTRO_USUARIO);
    ARQUIVO_USUARIOS[qtd_registros_usuarios*TAM_REGISTRO_USUARIO] = '\0';
}

/* Escreve no arquivo de jogos na posição informada (RRN)
 * os dados na struct Jogo */
void escrever_registro_jogo(Jogo j, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_JOGO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, j.id_game);
    strcat(temp, ";");
    strcat(temp, j.titulo);
    strcat(temp, ";");
    strcat(temp, j.desenvolvedor);
    strcat(temp, ";");
    strcat(temp, j.editora);
    strcat(temp, ";");
    strcat(temp, j.data_lancamento);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", j.preco);
    strcat(temp, p);
    strcat(temp, ";");
    for(int i = 0; i < QTD_MAX_CATEGORIAS; i++){
        if(*j.categorias[i] == '\0'){
            strcat(temp, "");
        } else {
            if(i == 1 || i == 2)
                strcat(temp, "|");

                strcat(temp, j.categorias[i]);
        }
    }
    // strcat(temp, "");
    strcat(temp, ";");
    
    for (int i = strlen(temp); i < TAM_REGISTRO_JOGO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_JOGOS + rrn*TAM_REGISTRO_JOGO, temp, TAM_REGISTRO_JOGO);
    ARQUIVO_JOGOS[qtd_registros_jogos*TAM_REGISTRO_JOGO] = '\0';
    //printf(ERRO_NAO_IMPLEMENTADO, "escrever_registro_jogo");
}

/* Escreve no arquivo de compras na posição informada (RRN)
 * os dados na struct Compra */
void escrever_registro_compra(Compra c, int rrn) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_COMPRA + 1];
    temp[0] = '\0'; 

    strcpy(temp, c.id_user_dono);
    strcat(temp, c.data_compra);
    strcat(temp, c.id_game);
    

    strncpy(ARQUIVO_COMPRAS + rrn*TAM_REGISTRO_COMPRA, temp, TAM_REGISTRO_COMPRA);
    ARQUIVO_COMPRAS[qtd_registros_compras*TAM_REGISTRO_COMPRA] = '\0';

    //printf(ERRO_NAO_IMPLEMENTADO, "escrever_registro_compra");
}


/* Funções principais */
void cadastrar_usuario_menu(char *id_user, char *username, char *email) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    usuarios_index temp;
    strcpy(temp.id_user, id_user);
    usuarios_index *aux = busca_binaria((void*)&temp, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    if(aux){
        printf(ERRO_PK_REPETIDA, id_user);
        return;
    }
    Usuario u;
    strcpy(u.id_user, id_user);
    strcpy(u.username, username);
    strcpy(u.email, email);
    strcpy(u.celular, "***********");
    u.saldo = 0;

    usuarios_idx[qtd_registros_usuarios].rrn= qtd_registros_usuarios;
    strcpy(usuarios_idx[qtd_registros_usuarios].id_user, id_user);
    qtd_registros_usuarios++;
    escrever_registro_usuario(u, usuarios_idx[qtd_registros_usuarios-1].rrn);
    
    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
    
    // criar_usuarios_idx();

    printf(SUCESSO);
    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_usuario_menu");
}

void cadastrar_celular_menu(char* id_user, char* celular) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    usuarios_index temp;
    strcpy(temp.id_user, id_user);
    usuarios_index *aux = busca_binaria((void*)&temp, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        // free(temp);
        return;
    }
    Usuario u = recuperar_registro_usuario(aux->rrn);
    strcpy(u.celular, celular);

    escrever_registro_usuario(u, aux->rrn);

    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
    // criar_usuarios_idx();

    printf(SUCESSO);
    // free(temp);
    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_celular_menu");
}

void remover_usuario_menu(char *id_user) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    // usuarios_index *temp = (usuarios_index*)malloc(sizeof(usuarios_index));
    // strcpy(temp->id_user, id_user);
    usuarios_index index;
    strcpy(index.id_user, id_user);
    
    usuarios_index *aux = busca_binaria((void*)&index, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        // free(temp);
        return;
    }
    Usuario u = recuperar_registro_usuario(aux->rrn);
    // strncpy(u.id_user, "*|", 2);
    u.id_user[0] = '*';
    u.id_user[1] = '|';

    escrever_registro_usuario(u, aux->rrn);
    aux->rrn = -1;

    printf(SUCESSO);
    // free(temp);
    //printf(ERRO_NAO_IMPLEMENTADO, "remover_usuario_menu");
}

void cadastrar_jogo_menu(char *titulo, char *desenvolvedor, char *editora, char* lancamento, double preco) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    // jogos_index *temp = (jogos_index*)malloc(sizeof(jogos_index));
    // strcpy(temp->id_game, titulo);
    // jogos_index *aux = (jogos_index*)busca_binaria(temp, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);
    titulos_index temp;
    strcpy(temp.titulo, titulo);
    titulos_index *aux = busca_binaria((void*)&temp, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    if(aux){
        printf(ERRO_PK_REPETIDA, titulo);
        // free(temp);
        return;
    }
    Jogo j;
    sprintf(j.id_game, "%.8u", qtd_registros_jogos);
    strcpy(j.titulo, titulo);
    strcpy(j.desenvolvedor, desenvolvedor);
    strcpy(j.editora, editora);
    strcpy(j.data_lancamento, lancamento);
    j.preco = preco;
    *j.categorias[0] = '\0';
    *j.categorias[1] = '\0';
    *j.categorias[2] = '\0';


    jogos_idx[qtd_registros_jogos].rrn = qtd_registros_jogos;
    sprintf(jogos_idx[qtd_registros_jogos].id_game, "%.8u", qtd_registros_jogos);

    sprintf(titulo_idx[qtd_registros_jogos].id_game, "%.8u", qtd_registros_jogos);
    strcpy(titulo_idx[qtd_registros_jogos].titulo, titulo);

    qtd_registros_jogos++;
    escrever_registro_jogo(j, jogos_idx[qtd_registros_jogos-1].rrn);

    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);
    qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);
    
    // criar_jogos_idx();
    // criar_titulo_idx();

    printf(SUCESSO);
    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_jogo_menu");
}

void adicionar_saldo_menu(char *id_user, double valor) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(valor <= 0){
        printf(ERRO_VALOR_INVALIDO);
        return;
    }

    usuarios_index temp;
    strcpy(temp.id_user, id_user);
    usuarios_index *aux = busca_binaria((void*)&temp, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        // free(temp);
        return;
    }

    // for(int i = 0; i < qtd_registros_usuarios; i ++){
    //     printf("%s", usuarios_idx[i].id_user);
    //      if(usuarios_idx[i].id_user == id_user){
    //          aux = usuarios_idx[i].rrn;_
    //          break;
    //      }
    // }

    Usuario u = recuperar_registro_usuario(aux->rrn);
    u.saldo = u.saldo + valor;

    escrever_registro_usuario(u, aux->rrn);

    printf(SUCESSO);
    // free(temp);
    //printf(ERRO_NAO_IMPLEMENTADO, "adicionar_saldo_menu");
}

void comprar_menu(char *id_user, char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    usuarios_index tempU;
    strcpy(tempU.id_user, id_user);
    usuarios_index *aux = busca_binaria((void*)&tempU, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    titulos_index tempT;
    strcpy(tempT.titulo, titulo);
    titulos_index *aux_titulo = busca_binaria((void*)&tempT, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    if(!aux_titulo){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    jogos_index tempJ;
    strcpy(tempJ.id_game, aux_titulo->id_game);
    jogos_index *aux_jogos = busca_binaria((void*)&tempJ, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);
    if(!aux_jogos){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    compras_index tempC;
    strcpy(tempC.id_user, id_user);
    strcpy(tempC.id_game, aux_jogos->id_game);
    compras_index *aux_compras = busca_binaria((void*)&tempC, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, false);
    if(aux_compras){
        char *aux_id = malloc(TAM_CHAVE_COMPRAS_IDX * sizeof(char));
        strcpy(aux_id, id_user);
        strcat(aux_id, aux_jogos->id_game);
        printf(ERRO_PK_REPETIDA, aux_id);
        free(aux_id);
        return;
    }

    Usuario u = recuperar_registro_usuario(aux->rrn);
    Jogo j = recuperar_registro_jogo(aux_jogos->rrn);
    if( j.preco > u.saldo ){
        printf(ERRO_SALDO_NAO_SUFICIENTE);
        return;
    }

    Compra c;
    char timestamp[TAM_DATE];
    current_date(timestamp);
    
    strcpy(c.id_user_dono, id_user);
    strcpy(c.id_game, aux_jogos->id_game);
    strcpy(c.data_compra, timestamp);


    compras_idx[qtd_registros_compras].rrn = qtd_registros_compras;
    strcpy(compras_idx[qtd_registros_compras].id_game, aux_jogos->id_game);
    strcpy(compras_idx[qtd_registros_compras].id_user, id_user);

    strcpy(data_user_game_idx[qtd_registros_compras].data, timestamp);
    strcpy(data_user_game_idx[qtd_registros_compras].id_game, aux_jogos->id_game);
    strcpy(data_user_game_idx[qtd_registros_compras].id_user, id_user);

    qtd_registros_compras++;

    escrever_registro_compra(c, compras_idx[qtd_registros_compras-1].rrn);

    qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx);    

    u.saldo = u.saldo - j.preco;
    escrever_registro_usuario(u, aux->rrn);

    qsort(data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx);   
    

    printf(SUCESSO);
}

void cadastrar_categoria_menu(char* titulo, char* categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    titulos_index tempT;
    strcpy(tempT.titulo, titulo);
    titulos_index *aux_titulo = busca_binaria((void*)&tempT, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    if(!aux_titulo){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    jogos_index tempJ;
    strcpy(tempJ.id_game, aux_titulo->id_game);
    jogos_index *aux_jogo = busca_binaria((void*)&tempJ, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);
    Jogo j = recuperar_registro_jogo(aux_jogo->rrn);
    for(int i = 0; i < QTD_MAX_CATEGORIAS; i++){
        if(strcmp(categoria, j.categorias[i]) == 0){
            printf(ERRO_CATEGORIA_REPETIDA, aux_titulo->titulo,categoria);
            return;
        }
        if(*j.categorias[i] == '\0'){
            strcpy(j.categorias[i], categoria);
            break;
        }
    }

    escrever_registro_jogo(j, aux_jogo->rrn);
    inverted_list_insert(categoria, aux_jogo->id_game, &categorias_idx);
    printf(SUCESSO);
    //printf(ERRO_NAO_IMPLEMENTADO, "cadastrar_categoria_menu");
}


/* Busca */
void buscar_usuario_id_user_menu(char *id_user) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    usuarios_index temp;
    strcpy(temp.id_user, id_user);
    printf(REGS_PERCORRIDOS);
    usuarios_index *aux = busca_binaria((void*)&temp, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, true);
    printf("\n");
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        // free(temp);
        return;
    }

    exibir_usuario(aux->rrn);
    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_usuario_id_user_menu");
}

void buscar_jogo_id_menu(char *id_game) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    jogos_index temp;
    strcpy(temp.id_game, id_game);
    printf(REGS_PERCORRIDOS);
    jogos_index *aux = busca_binaria((void*)&temp, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, true);
    printf("\n");
    if(!aux || aux->rrn == -1){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        // free(temp);
        return;
    }
    exibir_jogo(aux->rrn);
    // printf("%s, %s, %s, %s, %s, %.2f\n", j.id_game, j.titulo, j.desenvolvedor, j.editora, j.data_lancamento, j.preco);
    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_jogo_id_menu");
}

void buscar_jogo_titulo_menu(char *titulo) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    titulos_index temp;
    strcpy(temp.titulo, titulo);
    printf(REGS_PERCORRIDOS);
    titulos_index *aux = busca_binaria((void*)&temp, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, true);
    printf("\n");
    if(!aux){
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        return;
    }

    buscar_jogo_id_menu(aux->id_game);  
    //printf(ERRO_NAO_IMPLEMENTADO, "buscar_jogo_titulo_menu");
}


/* Listagem */
void listar_usuarios_id_user_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(qtd_registros_usuarios == 0){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }
    for(int i = 0; i < qtd_registros_usuarios; i++){
        //usuarios_index *aux = (usuarios_index*)malloc(sizeof(usuarios_index));
        //strcpy(aux->id_user, usuarios_idx[i].id_user);

        usuarios_index *user = (usuarios_index *)busca_binaria(usuarios_idx[i].id_user, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
        exibir_usuario(user->rrn);
    }
    //printf(ERRO_NAO_IMPLEMENTADO, "listar_usuarios_id_user_menu");
}

void listar_jogos_categorias_menu(char *categoria) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    // printf(REGS_PERCORRIDOS);
    int result;
    bool flag = inverted_list_secondary_search(&result, false, categoria,&categorias_idx);
    
    if(!flag){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    char result_jogos[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];
    int indice_fim;
    printf(REGS_PERCORRIDOS);
    int aux = inverted_list_primary_search(result_jogos, true, result, &indice_fim, &categorias_idx);
    printf(" %d", indice_fim);
    printf("\n");

    if(aux == 0){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    qsort(result_jogos, aux, sizeof(char**), qsort_jogos_idx);  
    
    for(int i = 0; i < aux; i++){
        jogos_index temp;
        // strncpy(temp.id_game, result_jogos[i], TAM_ID_GAME);
        strncpy(temp.id_game, result_jogos[i], TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX);

        jogos_index *aux = busca_binaria((void*)&temp, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, false);
        exibir_jogo(aux->rrn);
    }

    //printf(ERRO_NAO_IMPLEMENTADO, "listar_jogo_categorias_menu");
}

void listar_compras_periodo_menu(char *data_inicio, char *data_fim) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    
    data_user_game_index temp_ini;
    strcpy(temp_ini.data, data_inicio);
    data_user_game_index *aux_ini = busca_binaria_teto((void*)&temp_ini, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);
    if(!aux_ini){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }

    data_user_game_index temp_fim;
    strcpy(temp_fim.data, data_fim);
    data_user_game_index *aux_fim = busca_binaria_piso((void*)&temp_fim, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);
    if(!aux_fim){
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        return;
    }
    // printf("%s %s\n", data_fim, aux_fim->data);
    for(int i = 0; i < qtd_registros_compras; i++){
        if(strcmp(data_user_game_idx[i].data, aux_ini->data) >= 0 && strcmp(data_user_game_idx[i].data, aux_fim->data) <= 0){
            compras_index temp_compra;
            strcpy(temp_compra.id_user, data_user_game_idx[i].id_user);
            strcpy(temp_compra.id_game, data_user_game_idx[i].id_game);
            printf(REGS_PERCORRIDOS);
            compras_index *aux_compras = busca_binaria((void*)&temp_compra, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, true);
            printf("\n");
            exibir_compra(aux_compras->rrn);
        }
    }

    // for(int i = aux_ini->data; i <= aux_fim->data; i++){
    //     data_user_game_index temp;
    //     strcpy(temp.data, i);
    //     data_user_game_index *aux_user = busca_binaria((void*)&temp, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx, true);


    //     compras_index temp_compra;
    //     strcpy(temp_compra.id_user, aux_user->id_user);
    //     compras_index *aux_compra = busca_binaria((void*)temp_compra,compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, true);
    //     if(aux_compra){
    //         exibir_compra(aux_compra->rrn);
    //     }
    // }
    //printf(ERRO_NAO_IMPLEMENTADO, "listar_compras_periodo_menu");
}


/* Liberar espaço */
void liberar_espaco_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    //ARQUIVO_USUARIOS[qtd_registros_usuarios*TAM_REGISTRO_USUARIO]
    int aux = 0;
    for(int i = 0; i < (qtd_registros_usuarios * TAM_REGISTRO_USUARIO); i = (i + TAM_REGISTRO_USUARIO)){
        if(strncmp(&ARQUIVO_USUARIOS[i], "*|", 2) == 0){
            int w = i;
            for(int j = i + TAM_REGISTRO_USUARIO; j < (qtd_registros_usuarios * TAM_REGISTRO_USUARIO); j++){
                memmove(&ARQUIVO_USUARIOS[w], &ARQUIVO_USUARIOS[j], TAM_REGISTRO_USUARIO);
                w++;
            }
            aux++;
            i = i - TAM_REGISTRO_USUARIO;
        }
    }

    qtd_registros_usuarios = qtd_registros_usuarios - aux;
    criar_usuarios_idx();
    
    printf(SUCESSO);
    //printf(ERRO_NAO_IMPLEMENTADO, "liberar_espaco_menu");
}


/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_USUARIOS);
}

void imprimir_arquivo_jogos_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_JOGOS);
}

void imprimir_arquivo_compras_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_COMPRAS);
}


/* Imprimir índices primários */
void imprimir_usuarios_idx_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i)
        printf("%s, %d\n", usuarios_idx[i].id_user, usuarios_idx[i].rrn);
}

void imprimir_jogos_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %d\n", jogos_idx[i].id_game, jogos_idx[i].rrn);
}

void imprimir_compras_idx_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %d\n", compras_idx[i].id_user, compras_idx[i].id_game, compras_idx[i].rrn);
}


/* Imprimir índices secundários */
void imprimir_titulo_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %s\n", titulo_idx[i].titulo, titulo_idx[i].id_game);
}

void imprimir_data_user_game_idx_menu() {
    if (qtd_registros_compras == 0) {
        printf(ERRO_ARQUIVO_VAZIO);
        return;
    }

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %s\n", data_user_game_idx[i].data, data_user_game_idx[i].id_user, data_user_game_idx[i].id_game);
}

void imprimir_categorias_secundario_idx_menu() {
    if (categorias_idx.qtd_registros_secundario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_secundario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_secundario_idx)[i].chave_secundaria, (categorias_idx.categorias_secundario_idx)[i].primeiro_indice);
}

void imprimir_categorias_primario_idx_menu() {
    if (categorias_idx.qtd_registros_primario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_primario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_primario_idx)[i].chave_primaria, (categorias_idx.categorias_primario_idx)[i].proximo_indice);
}


/* Liberar memória e encerrar programa */
void liberar_memoria_menu() {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    free(usuarios_idx);
    free(jogos_idx);
    free(compras_idx);
    free(titulo_idx);
    free(data_user_game_idx);
    free(categorias_idx.categorias_secundario_idx);
    free(categorias_idx.categorias_primario_idx);
    //printf(ERRO_NAO_IMPLEMENTADO, "liberar_memoria_menu");
}


/* Função de comparação entre chaves do índice usuarios_idx */
int qsort_usuarios_idx(const void *a, const void *b) {
    return strcmp( ( (usuarios_index *)a )->id_user, ( (usuarios_index *)b )->id_user);
}

/* Função de comparação entre chaves do índice jogos_idx */
int qsort_jogos_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strcmp( ( (jogos_index *)a )->id_game, ( (jogos_index *)b )->id_game);
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_jogos_idx");
}

/* Função de comparação entre chaves do índice compras_idx */
int qsort_compras_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int aux = strcmp( ( (compras_index *)a )->id_user, ( (compras_index *)b )->id_user);
    if(!aux){
        return(strcmp( ( (compras_index *)a )->id_game, ( (compras_index *)b )->id_game));
    }
    return (aux);
   // printf(ERRO_NAO_IMPLEMENTADO, "qsort_compras_idx");
}

/* Função de comparação entre chaves do índice titulo_idx */
int qsort_titulo_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
     return (strcmp( ( (titulos_index *)a )->titulo, ( (titulos_index *)b )->titulo));
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_titulo_idx");
}

/* Funções de comparação entre chaves do índice data_user_game_idx */
int qsort_data_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strcmp( ( (data_user_game_index *)a )->data, ( (data_user_game_index *)b )->data);
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_data_idx");
}

int qsort_data_user_game_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int aux_user = strcmp( ( (data_user_game_index *)a )->id_user, ( (data_user_game_index *)b )->id_user);
    int aux_data = strcmp( ( (data_user_game_index *)a )->data, ( (data_user_game_index *)b )->data);
    // if(!aux_user){
    //     if(!aux_data){
    //         return strcmp( ( (data_user_game_index *)a )->id_game, ( (data_user_game_index *)b )->id_game);
    //     }
    //     return aux_data;
    // }
    // return aux_user;

    if(!aux_data){
        if(!aux_user){
            return strcmp( ( (data_user_game_index *)a )->id_game, ( (data_user_game_index *)b )->id_game);
        }
        return(aux_user);
    }
    return(aux_data);
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_data_user_game_idx");
}

/* Função de comparação entre chaves do índice secundário de categorias_idx */
int qsort_categorias_secundario_idx(const void *a, const void *b) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    return strcmp( ( (categorias_secundario_index *)a )->chave_secundaria , ( (categorias_secundario_index *)b )->chave_secundaria );
    //printf(ERRO_NAO_IMPLEMENTADO, "qsort_categorias_secundario_idx");
}


/* Funções de manipulação de Lista Invertida */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int result;
    bool flag = inverted_list_secondary_search(&result,false,chave_secundaria, t);
    
    if(!flag){
        strcpy(t->categorias_secundario_idx[t->qtd_registros_secundario].chave_secundaria, chave_secundaria);
        t->categorias_secundario_idx[t->qtd_registros_secundario].primeiro_indice = t->qtd_registros_primario;
        t->qtd_registros_secundario++;
        strcpy(t->categorias_primario_idx[t->qtd_registros_primario].chave_primaria, chave_primaria);
        t->categorias_primario_idx[t->qtd_registros_primario].proximo_indice = -1;
        t->qtd_registros_primario++;
        qsort(t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx); 
    } else {
        int aux = result;
        for(int w = result; t->categorias_primario_idx[w].proximo_indice != -1; w = t->categorias_primario_idx[w].proximo_indice){
            aux = categorias_idx.categorias_primario_idx[w].proximo_indice;
        }
        
        t->categorias_primario_idx[aux].proximo_indice = t->qtd_registros_primario;
        strcpy(t->categorias_primario_idx[t->qtd_registros_primario].chave_primaria, chave_primaria);
        t->categorias_primario_idx[categorias_idx.qtd_registros_primario].proximo_indice = -1;
        t->qtd_registros_primario++;
    }
    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_insert");
}

bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    categorias_secundario_index temp;
    strcpy(temp.chave_secundaria, chave_secundaria);
    categorias_secundario_index *aux = (categorias_secundario_index *)busca_binaria(&temp, t->categorias_secundario_idx,t->qtd_registros_secundario ,sizeof(categorias_secundario_index), qsort_categorias_secundario_idx, exibir_caminho);

    if (aux){
        if(result){
            *result = aux->primeiro_indice;
        }
         return true;
    }
     return false;
    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_secondary_search");
}

int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    int count = 0;
    *indice_final = indice;
    for(int i = indice; t->categorias_primario_idx[i].proximo_indice != -1; i = t->categorias_primario_idx[i].proximo_indice){
        if(result)
            strcpy(result[count], t->categorias_primario_idx[i].chave_primaria);
        if(exibir_caminho)
            printf(" %d", i);
        if(indice_final)
            *indice_final = t->categorias_primario_idx[i].proximo_indice; 
        count++;       
    }
    strcpy(result[count], t->categorias_primario_idx[*indice_final].chave_primaria);
    return count+1;
    //printf(ERRO_NAO_IMPLEMENTADO, "inverted_list_primary_search");
}

char* strpadright(char *str, char pad, unsigned size) {
    for (unsigned i = strlen(str); i < size; ++i)
        str[i] = pad;
    str[size] = '\0';
    return str;
}

/* Funções da busca binária */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    if(exibir_caminho){
        size_t imin, imax, imid;
        const void *p;
        int comp;
        imin = 0;
        imax = nmemb;
        while (imin < imax)
            {
                imid = (imin + imax) / 2;
                printf(" %zu", imid);
                p = (void *) (((const char *) base0) + (imid * size));
                comp = (*compar) (key, p);
                if (comp < 0){
                    imax = imid;
                }  
                else if (comp > 0){
                    imin = imid + 1;
                }
                else{
                    return (void *) p;
                }  
            }
        return NULL;
    }
    size_t imin, imax, imid;
    const void *p;
    int comp;
    imin = 0;
    imax = nmemb;
    while (imin < imax)
        {
            imid = (imin + imax) / 2;
            p = (void *) (((const char *) base0) + (imid * size));
            comp = (*compar) (key, p);
            if (comp < 0)
                imax = imid;
            else if (comp > 0)
                imin = imid + 1;
            else
                return (void *) p;
        }
    return NULL;
   
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria");
}

void* busca_binaria_piso(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*)) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */
    size_t imin, imax, imid;
    const void *p;
    int comp;
    imin = 0;
    imax = num;
    const void *retorno = NULL;
    while (imin < imax)
        {
            imid = (imin + imax) / 2;
            p = (void *) (((const char *) base) + (imid * size));
            comp = (*compar) (key, p);
            if (comp < 0){
                imax = imid;
            }
            else if (comp > 0){
                retorno = p;
                imin = imid + 1;
            }
            else
                return (void *) p;
                
        }
    return (void *)retorno;
    //problema: imid = 0 -> imid-1??
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria_piso");
}

void* busca_binaria_teto(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*)) {
    /* <<< COMPLETE AQUI A IMPLEMENTAÇÃO >>> */

    size_t imin, imax, imid;
    const void *p;
    int comp;
    imin = 0;
    imax = num;
    const void *retorno = NULL;
    while (imin < imax)
        {
            imid = (imin + imax) / 2;
            p = (void *) (((const char *) base) + (imid * size));
            comp = (*compar) (key, p);
            if (comp < 0){
                retorno = p;
                imax = imid;
            }
            else if (comp > 0)
                imin = imid + 1;
            else
                return (void *) p;
                
        }
    return (void*)retorno;
    //printf(ERRO_NAO_IMPLEMENTADO, "busca_binaria_teto");
}