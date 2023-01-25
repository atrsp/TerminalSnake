
#include <stdio.h>
#include <stdlib.h>

#define TAM_MAPA 100
#define TAM_ENTRADA 1000
#define TAM_COBRA 100
#define CARACTERE_COMIDA '*'
#define CARACTERE_CORPO 'o'
#define CARACTERE_DINHEIRO '$'
#define CARACTERE_CABECA_DIREITA '>'
#define CARACTERE_MORTE 'X'
#define CARACTERE_PAREDE '#'
#define CARACTERE_CABECA_ESQUERDA '<'
#define CARACTERE_CABECA_CIMA '^'
#define CARACTERE_CABECA_BAIXO 'v'
#define CARACTERE_MOV_continua 'c'
#define CARACTERE_MOV_horario 'h'
#define CARACTERE_MOV_antihorario 'a'
#define CARACTERE_VAZIO ' '
#define CARACTERE_TUNEL '@'
#define QTD_TUNEIS 2

typedef struct {
    char matriz [TAM_MAPA][TAM_MAPA];
    int heatmap [TAM_MAPA][TAM_MAPA];
    int linhas;
    int colunas;
    int qtd_comidas;
    int tuneis[2][2];
} tMapa;

typedef struct {
    char direcao_giro;
    int numero_jogada;
    int linha_mov;
    int coluna_mov;
} tJogada;

typedef struct {
    int tam_corpo;
    char cabeca;
    int pontos;
    int morreu;
    int coordenadas_corpo[TAM_COBRA][2];
    int coord_ultimo_corpo[2];
} tCobrinha;

typedef struct {
    int qtd_jogadas;
    int qtd_jogadas_nao_pontuadas;
    int qtd_movimentos_para_baixo;
    int qtd_movimentos_para_cima;
    int qtd_movimentos_para_direita;
    int qtd_movimentos_para_esquerda;
} tEstatisticas;

typedef struct {
    tJogada jogada;
    tCobrinha cobrinha;
    tMapa mapa;
    tEstatisticas dados;
    int colisao;
    int venceu;
    int gera_dinheiro;
    int cresce_cobrinha;
} tVariaveis;

//Funcoes que inicializam o jogo:
tMapa InicializaMapa(tVariaveis jogo, char*argv[], int argc);
tMapa LeMapaInicial(FILE*FMapaInicial);
tVariaveis ImprimeMapaInicial(tVariaveis jogo, char*argv[]);

//Funcoes que jogam o jogo:
void JogandoOJogo(tVariaveis jogo, char*argv[]);
tCobrinha AvaliaTamanhoCobrinha(tMapa mapa_parcial, tCobrinha cobrinha);
int AvaliaQuantidadedeComida(tVariaveis jogo);
tCobrinha BuscaPosicaoInicialCobrinha(tCobrinha cobrinha, tMapa mapa_inicial);
tJogada LeJogada(tJogada jogada);
tVariaveis AvaliaJogada(tVariaveis jogo);
tVariaveis MovimentaCobrinha(tVariaveis jogo);
tVariaveis CresceCobrinha(tVariaveis jogo);
tVariaveis AnalisaMovimento(tVariaveis jogo);
tVariaveis EhDinheiro(tVariaveis jogo);
tVariaveis EhComida(tVariaveis jogo);
tVariaveis MorreCobrinha(tVariaveis jogo);
tVariaveis AnalisaProxLinha(tVariaveis jogo);
tVariaveis AnalisaProxColuna(tVariaveis jogo);
tVariaveis AnalisaLinhaAnterior(tVariaveis jogo);
tVariaveis AnalisaColunaAnterior(tVariaveis jogo);
void ImprimeMapaParcial(tVariaveis jogo);
void ImprimeResultadoFinal(tVariaveis jogo);
tVariaveis TrataTuneis(tVariaveis jogo);
tVariaveis BuscaTuneis(tVariaveis jogo);

//Funcoes que geram heatmap:
void ImprimeHeatMap(char*argv[], tVariaveis jogo);
tVariaveis AtualizaHeatMap(tVariaveis jogo);
tVariaveis InicializaHeatMap(tVariaveis jogo);

//Funcoes que geram estatisticas para analise:
void ImprimeEstatisticas(tVariaveis jogo, char*argv[]);

//Funcoes que geram ranking de posicoes mais frequentes:
void GeraRanking(tVariaveis jogo, char*argv[]);
void OrdenaPosicoes(tVariaveis jogo, int qtd_posicoes_acessadas, int ranking [qtd_posicoes_acessadas][3]);

//Funcoes que geram resumo do resultado:
void AtualizaResumo(tVariaveis jogo, char*argv[], int fim_de_jogo);

//--------------------------------------------------MAIN------------------------------------------------------

int main(int argc, char*argv[]) {

    tVariaveis jogo;
    jogo.mapa = InicializaMapa(jogo, argv, argc);
    JogandoOJogo(jogo, argv);

    return 0;
}

//----------------------------------------------INICIALIZACAO--------------------------------------------------

tMapa InicializaMapa(tVariaveis jogo, char*argv[], int argc) {

    //Funcao inicial, que prepara o ambiente de jogo. Ela identifica se eh possivel abrir o arquivo mapa.txt e, se for o caso, chama outras 2 funcoes: a "LeMapa" para ler o conteudo do arquivo e a "ImprimeMapa" para imprimir o mapa e as posicoes iniciais da cabeca da cobra no arquivo inicializa.txt .

    FILE*FMapaInicial;
    char caminhodoarquivo [TAM_ENTRADA], arquivofinal[TAM_ENTRADA];


    if (argc <= 1) {
        printf("ERRO: O diretorio de arquivos de configuracao nao foi informado.\n");
        exit(0);
    }

    sprintf(caminhodoarquivo, "%s/mapa.txt", argv[1]);
    FMapaInicial = fopen(caminhodoarquivo, "r");

    if (!FMapaInicial) {
        printf("Nao foi possivel ler o arquivo 'mapa.txt' pelo caminho %s\n", argv[1]);
        exit(0);
    } else {
        jogo.mapa = LeMapaInicial(FMapaInicial);
        jogo = ImprimeMapaInicial(jogo, argv);
    }

    fclose(FMapaInicial);
    return jogo.mapa;
}

tMapa LeMapaInicial(FILE*FMapaInicial) {
    tMapa mapalido;
    int i = 0, j = 0, colunas, linhas, retorno = 2;

    retorno = fscanf(FMapaInicial, "%d %d%*c", &linhas, &colunas);

    mapalido.linhas = linhas;
    mapalido.colunas = colunas;

    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            fscanf(FMapaInicial, "%c", &mapalido.matriz[i][j]);
        }

        fscanf(FMapaInicial, "%*c");
    }


    return mapalido;
}

tVariaveis ImprimeMapaInicial(tVariaveis jogo, char*argv[]) {

    //Imprime o mapa inicial, lido de outro arquivo, e o transfere para outro arquivo, denominado "inicializacao.txt".
    //Nessa funcao, tambem buscamos as coordenadas do ponto inicial da cobra, a ser impresso no mesmo arquivo, junto ao mapa.

    int i = 0, j = 0, encontrou_cabeca_cobra = 0;
    FILE*fInicializa;
    char arquivofinal [TAM_ENTRADA];

    sprintf(arquivofinal, "%s/saida/inicializacao.txt", argv[1]);
    fInicializa = fopen(arquivofinal, "w");

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {

            fprintf(fInicializa, "%c", jogo.mapa.matriz[i][j]);

            //Aproveitando o loop para buscar a cabeca da cobra:

            if (!encontrou_cabeca_cobra && jogo.mapa.matriz[i][j] == CARACTERE_CABECA_DIREITA) {
                jogo.cobrinha.coordenadas_corpo[0][0] = i + 1;
                jogo.cobrinha.coordenadas_corpo[0][1] = j + 1;
                encontrou_cabeca_cobra = 1;
            }
        }
        fprintf(fInicializa, "\n");
    }

    fprintf(fInicializa, "A cobra comecara o jogo na linha %d e coluna %d\n", jogo.cobrinha.coordenadas_corpo[0][0], jogo.cobrinha.coordenadas_corpo[0][1]);

    jogo.cobrinha.coordenadas_corpo[0][0]--;
    jogo.cobrinha.coordenadas_corpo[0][1]--;

    fclose(fInicializa);
    return jogo;
}

//--------------------------------------------------JOGO-------------------------------------------------------

void JogandoOJogo(tVariaveis jogo, char*argv[]) {

    //Funcao que joga o jogo; Responsavel por chamar funcoes que exercem os seguintes papeis: ler as jogadas da entrada padrao, movimentar a cobra pelo mapa, consumir "comidas" e "dinheiros", aumentar a cobra de tamanho, etc. (OBS: a cada movimento, uma parcial de como esta o mapa é impressa);


    int primeira_rodada = 1, fim_de_jogo = 0;
    jogo.cobrinha.morreu = 0;
    while (1) {

        if (primeira_rodada == 1) {

            jogo.cobrinha.tam_corpo = 1;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_DIREITA;
            jogo.cobrinha.pontos = 0;
            jogo.cobrinha.morreu = 0;
            jogo.mapa.qtd_comidas = AvaliaQuantidadedeComida(jogo);

            jogo.colisao = 0;
            jogo.venceu = 0;
            jogo.gera_dinheiro = 0;
            jogo.cresce_cobrinha = 0;

            jogo = InicializaHeatMap(jogo);

            jogo.dados.qtd_jogadas = 0;
            jogo.dados.qtd_jogadas_nao_pontuadas = 0;
            jogo.dados.qtd_movimentos_para_baixo = 0;
            jogo.dados.qtd_movimentos_para_cima = 0;
            jogo.dados.qtd_movimentos_para_direita = 0;
            jogo.dados.qtd_movimentos_para_esquerda = 0;

            jogo.jogada.numero_jogada = 0;

            primeira_rodada = 0;

            jogo.cobrinha = BuscaPosicaoInicialCobrinha(jogo.cobrinha, jogo.mapa);
                jogo.cobrinha.coord_ultimo_corpo[0] = jogo.cobrinha.coordenadas_corpo[0][0];
                jogo.cobrinha.coord_ultimo_corpo[1] = jogo.cobrinha.coordenadas_corpo[0][1];


            jogo.mapa.heatmap[jogo.cobrinha.coordenadas_corpo[0][0]][jogo.cobrinha.coordenadas_corpo[0][1]]++;

            jogo.mapa.qtd_comidas = AvaliaQuantidadedeComida(jogo);
            jogo = BuscaTuneis(jogo);
            AtualizaHeatMap(jogo);


        } else {

            jogo.jogada = LeJogada(jogo.jogada);
            jogo.dados.qtd_jogadas++;
            jogo = AvaliaJogada(jogo);
            ImprimeMapaParcial(jogo);

            if (jogo.cobrinha.morreu == 1) {
                ImprimeResultadoFinal(jogo);
                fim_de_jogo = 1;
                jogo.colisao = 1;
                AtualizaResumo(jogo, argv, fim_de_jogo);
                break;

            } else {
                if (jogo.mapa.qtd_comidas == 0) {

                    ImprimeResultadoFinal(jogo);
                    fim_de_jogo = 1;
                    jogo.venceu = 1;
                    AtualizaResumo(jogo, argv, fim_de_jogo);
                    break;
                }
                AtualizaHeatMap(jogo);
                AtualizaResumo(jogo, argv, fim_de_jogo);
                jogo.cresce_cobrinha = 0;
                jogo.gera_dinheiro = 0;
            }
        }
    }

    GeraRanking(jogo, argv);
    ImprimeHeatMap(argv, jogo);
    ImprimeEstatisticas(jogo, argv);

}

//Funcoes auxiliares (funcionamento do jogo):

tVariaveis InicializaMatriz(tVariaveis jogo) {
    int i = 0, j = 0;

    //Inicializando a matriz que armazena as coordenadas do corpo da cobrinha.

    for (i = 0; i < TAM_COBRA; i++) {
        for (j = 0; j < 2; j++) {
            jogo.cobrinha.coordenadas_corpo[i][j] = -34;
        }
    }

    return jogo;
}

int AvaliaQuantidadedeComida(tVariaveis jogo) {

    //Busca a quantidade de comida no mapa.

    int qtdcomida = 0, i = 0, j = 0;

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            if (jogo.mapa.matriz[i][j] == CARACTERE_COMIDA) {
                qtdcomida++;
            }
        }
    }
    return qtdcomida;
}

tCobrinha BuscaPosicaoInicialCobrinha(tCobrinha cobrinha, tMapa mapa_inicial) {

    //Transcorre o mapa buscando pela cabeca da cobra (direita), simbolizando o ponto de inicio do jogo. 

    int i = 0, j = 0;
    cobrinha.cabeca = CARACTERE_CABECA_DIREITA;
    for (i = 0; i < mapa_inicial.linhas; i++) {
        for (j = 0; j < mapa_inicial.colunas; j++) {
            if (mapa_inicial.matriz[i][j] == CARACTERE_CABECA_DIREITA) {
                cobrinha.coordenadas_corpo[0][0] = i;
                cobrinha.coordenadas_corpo[0][1] = j;
                
            }
        }
    }
    return cobrinha;
}

tJogada LeJogada(tJogada jogada) {

    //Le a jogada inserida pelo usuario na entrada padrao.

    scanf("%*[^a-z]");
    scanf("%c", &jogada.direcao_giro);
    scanf("%*c");

    jogada.numero_jogada++;
    return jogada;
}

void ImprimeMapaParcial(tVariaveis jogo) {

    //Imprime a situacao do mapa apos cada jogada.

    int i = 0, j = 0;

    printf("\nEstado do jogo apos o movimento '%c':\n", jogo.jogada.direcao_giro);
    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {

            printf("%c", jogo.mapa.matriz[i][j]);
        }
        printf("\n");
    }

    printf("Pontuacao: %d\n", jogo.cobrinha.pontos);
}

tVariaveis AvaliaJogada(tVariaveis jogo) {

    //Avalia a direcao da jogada (entrada do usuario) e, a partir da direcao da cabeca da cobra, toma decisao sobre qual a proxima casa para a qual ela se movera.

    if (jogo.jogada.direcao_giro == CARACTERE_MOV_continua) {
        if (jogo.cobrinha.cabeca == CARACTERE_CABECA_DIREITA) {

            jogo.dados.qtd_movimentos_para_direita++;
            jogo = AnalisaProxColuna(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_BAIXO) {

            jogo.dados.qtd_movimentos_para_baixo++;
            jogo = AnalisaProxLinha(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_CIMA) {
            jogo.dados.qtd_movimentos_para_cima++;
            jogo = AnalisaLinhaAnterior(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_ESQUERDA) {
            jogo.dados.qtd_movimentos_para_esquerda++;
            jogo = AnalisaColunaAnterior(jogo);

        }

    } else if (jogo.jogada.direcao_giro == CARACTERE_MOV_horario) {
        if (jogo.cobrinha.cabeca == CARACTERE_CABECA_DIREITA) {

            jogo.dados.qtd_movimentos_para_baixo++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_BAIXO;
            jogo = AnalisaProxLinha(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_BAIXO) {

            jogo.dados.qtd_movimentos_para_esquerda++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_ESQUERDA;
            jogo = AnalisaColunaAnterior(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_CIMA) {

            jogo.dados.qtd_movimentos_para_direita++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_DIREITA;
            jogo = AnalisaProxColuna(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_ESQUERDA) {

            jogo.dados.qtd_movimentos_para_cima++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_CIMA;
            jogo = AnalisaLinhaAnterior(jogo);

        }
    } else if (jogo.jogada.direcao_giro == CARACTERE_MOV_antihorario) {
        if (jogo.cobrinha.cabeca == CARACTERE_CABECA_DIREITA) {

            jogo.dados.qtd_movimentos_para_cima++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_CIMA;
            jogo = AnalisaLinhaAnterior(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_BAIXO) {

            jogo.dados.qtd_movimentos_para_direita++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_DIREITA;
            jogo = AnalisaProxColuna(jogo);

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_CIMA) {

            jogo.dados.qtd_movimentos_para_esquerda++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_ESQUERDA;
            jogo = AnalisaColunaAnterior(jogo);


        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_ESQUERDA) {

            jogo.dados.qtd_movimentos_para_baixo++;
            jogo.cobrinha.cabeca = CARACTERE_CABECA_BAIXO;
            jogo = AnalisaProxLinha(jogo);

        }
    }
    jogo = AnalisaMovimento(jogo);
    return jogo;
}

tVariaveis AnalisaProxLinha(tVariaveis jogo) {

    //Analisa a mesma coluna na linha seguinte (no mapa principal), para avaliarmos se tem algo nela em outra funcao (AnalisaMovimento).

    if (jogo.cobrinha.coordenadas_corpo[0][0] < jogo.mapa.linhas - 1 && jogo.cobrinha.coordenadas_corpo[0][0] >= 0) {

        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0] + 1;
        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1];
    } else if (jogo.cobrinha.coordenadas_corpo[0][0] == jogo.mapa.linhas - 1) {

        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1];
        jogo.jogada.linha_mov = 0;
    }
    return jogo;
}

tVariaveis AnalisaProxColuna(tVariaveis jogo) {

    //Analisa a mesma linha na coluna seguinte (no mapa principal), para avaliarmos se tem algo nela em outra funcao (AnalisaMovimento).

    if (jogo.cobrinha.coordenadas_corpo[0][1] < jogo.mapa.colunas - 1 && jogo.cobrinha.coordenadas_corpo[0][1] >= 0) {

        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1] + 1;
        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0];
    } else if (jogo.cobrinha.coordenadas_corpo[0][1] == jogo.mapa.colunas - 1) {

        jogo.jogada.coluna_mov = 0;
        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0];
    }
    return jogo;
}

tVariaveis AnalisaLinhaAnterior(tVariaveis jogo) {

    //Analisa a mesma coluna na linha anterior (no mapa principal), para avaliarmos se tem algo nela em outra funcao (AnalisaMovimento).

    if (jogo.cobrinha.coordenadas_corpo[0][0] <= jogo.mapa.linhas - 1 && jogo.cobrinha.coordenadas_corpo[0][0] > 0) {

        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0] - 1;
        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1];
    } else if (jogo.cobrinha.coordenadas_corpo[0][0] == 0) {

        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1];
        jogo.jogada.linha_mov = jogo.mapa.linhas - 1;
    }
    return jogo;
}

tVariaveis AnalisaColunaAnterior(tVariaveis jogo) {

    //Analisa a mesma linha na coluna anterior (no mapa principal), para avaliarmos se tem algo nela em outra funcao (AnalisaMovimento).

    if (jogo.cobrinha.coordenadas_corpo[0][1] <= jogo.mapa.colunas - 1 && jogo.cobrinha.coordenadas_corpo[0][1] > 0) {

        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0];
        jogo.jogada.coluna_mov = jogo.cobrinha.coordenadas_corpo[0][1] - 1;
    } else if (jogo.cobrinha.coordenadas_corpo[0][1] == 0) {

        jogo.jogada.coluna_mov = jogo.mapa.colunas - 1;
        jogo.jogada.linha_mov = jogo.cobrinha.coordenadas_corpo[0][0];
    }

    return jogo;
}

tVariaveis EhComida(tVariaveis jogo) {

    //Se aproxima casa for uma comida, o corpo da cobrinha cresce, ela se movimenta e ganha 1 ponto por comida.

    jogo = CresceCobrinha(jogo);
    jogo.cresce_cobrinha = 1;
    jogo = MovimentaCobrinha(jogo);
    jogo.mapa.qtd_comidas--;
    jogo.cobrinha.pontos++;

    return jogo;
}

tVariaveis EhDinheiro(tVariaveis jogo) {

    //Se a proxima casa for um dinheiro, movimenta a cobrinha, arrecadando 10 pontos por dinheiro.

    jogo = MovimentaCobrinha(jogo);
    jogo.cobrinha.pontos += 10;
    jogo.gera_dinheiro = 1;
    return jogo;
}

tVariaveis EhParede(tVariaveis jogo) {

    //Se a proxima casa for uma parede, mexe a cobrinha, gerando colisao entre a cabeca e a parede e matando-a.

    jogo = MovimentaCobrinha(jogo);
    jogo = MorreCobrinha(jogo);
    jogo.cobrinha.morreu = 1;
    return jogo;
}

tVariaveis EhCorpo(tVariaveis jogo) {

    //Se a proxima casa for um corpo da cobrinha, sem ser o ultimo corpo da cobra, mexe a cobrinha, gerando colisao entre a cabeca e o corpo e matando-a.
    if (jogo.jogada.coluna_mov == jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo-1][1] && jogo.jogada.linha_mov == jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo-1][0]){
        jogo = MovimentaCobrinha(jogo);
        return jogo;
    }
    jogo = MovimentaCobrinha(jogo);
    jogo = MorreCobrinha(jogo);
    jogo.cobrinha.morreu = 1;
    return jogo;
}

tVariaveis EhVazio(tVariaveis jogo) {

    //Se a proxima casa for vazia, basta a cobrinha andar sem alterar seu corpo, sua posicao ou o mapa.

    jogo = MovimentaCobrinha(jogo);
    return jogo;
}

tVariaveis AnalisaMovimento(tVariaveis jogo) {

    //Analisa se a proxima posicao da cabeca tem algum obstaculo ou premio.

    if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_COMIDA) {
        jogo = EhComida(jogo);

    } else if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_DINHEIRO) {
        jogo = EhDinheiro(jogo);

    } else if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_CORPO) {
        jogo = EhCorpo(jogo);
        jogo.dados.qtd_jogadas_nao_pontuadas++;

    } else if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_PAREDE) {
        jogo = EhParede(jogo);
        jogo.dados.qtd_jogadas_nao_pontuadas++;

    } else if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_VAZIO) {
        jogo = EhVazio(jogo);
        jogo.dados.qtd_jogadas_nao_pontuadas++;

    } else if (jogo.mapa.matriz [jogo.jogada.linha_mov][jogo.jogada.coluna_mov] == CARACTERE_TUNEL) {
        jogo = TrataTuneis(jogo);
        jogo.dados.qtd_jogadas_nao_pontuadas++;
    }

    return jogo;
}

tVariaveis MovimentaCobrinha(tVariaveis jogo) {

    int i = 1, aux[2], armazena[2];

    //Mexe a cabeca, seguindo a posicao determinada pelas funcoes antecedentes (AvaliaJogada e AnalisaMovimento):

    jogo.mapa.matriz[jogo.jogada.linha_mov][jogo.jogada.coluna_mov] = jogo.cobrinha.cabeca;
    jogo.mapa.heatmap[jogo.jogada.linha_mov][jogo.jogada.coluna_mov]++;

    aux[0] = jogo.cobrinha.coordenadas_corpo[0][0];
    aux[1] = jogo.cobrinha.coordenadas_corpo[0][1];

    jogo.cobrinha.coordenadas_corpo[0][0] = jogo.jogada.linha_mov;
    jogo.cobrinha.coordenadas_corpo[0][1] = jogo.jogada.coluna_mov;
    
    jogo.mapa.matriz[aux[0]][aux[1]] = CARACTERE_VAZIO;
    
    if (jogo.cobrinha.tam_corpo==1){

    jogo.cobrinha.coordenadas_corpo[1][0] = aux[0];
    jogo.cobrinha.coordenadas_corpo[1][1] = aux[1];
    
    jogo.cobrinha.coord_ultimo_corpo[0] = aux[0];
    jogo.cobrinha.coord_ultimo_corpo[1] = aux [1];
    
    
    return jogo;
    }
    
    //Mexe o corpo, seguindo as coordenadas da cabeca:

    for (i = 1; i <= jogo.cobrinha.tam_corpo; i++) {

        armazena[0] = jogo.cobrinha.coordenadas_corpo[i][0];
        armazena[1] = jogo.cobrinha.coordenadas_corpo[i][1];

        jogo.cobrinha.coordenadas_corpo[i][0] = aux[0];
        jogo.cobrinha.coordenadas_corpo[i][1] = aux[1];

        aux[0] = armazena[0];
        aux[1] = armazena[1];

        jogo.cobrinha.coord_ultimo_corpo[0] = aux[0];
        jogo.cobrinha.coord_ultimo_corpo[1] = aux [1];
    }

    if (!jogo.cresce_cobrinha){
    jogo.cobrinha.coord_ultimo_corpo[0] = jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo][0];
    jogo.cobrinha.coord_ultimo_corpo[1] = jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo][1];}
    
     if(jogo.mapa.matriz[jogo.cobrinha.coord_ultimo_corpo[0]][jogo.cobrinha.coord_ultimo_corpo[1]] == CARACTERE_CABECA_BAIXO || jogo.mapa.matriz[jogo.cobrinha.coord_ultimo_corpo[0]][jogo.cobrinha.coord_ultimo_corpo[1]]== CARACTERE_CABECA_CIMA  || jogo.mapa.matriz[jogo.cobrinha.coord_ultimo_corpo[0]][jogo.cobrinha.coord_ultimo_corpo[1]]== CARACTERE_CABECA_DIREITA || jogo.mapa.matriz[jogo.cobrinha.coord_ultimo_corpo[0]][jogo.cobrinha.coord_ultimo_corpo[1]]== CARACTERE_CABECA_ESQUERDA){
        return jogo;
    }
    else {
    jogo.mapa.matriz[jogo.cobrinha.coord_ultimo_corpo[0]][jogo.cobrinha.coord_ultimo_corpo[1]] = CARACTERE_VAZIO;}

    for (i = 1; i < jogo.cobrinha.tam_corpo; i++) {
        jogo.mapa.matriz[jogo.cobrinha.coordenadas_corpo[i][0]][jogo.cobrinha.coordenadas_corpo[i][1]] = CARACTERE_CORPO;
    }
    

   
    return jogo;
}

tVariaveis MorreCobrinha(tVariaveis jogo) {
    int i = 0, j = 0;
    jogo.cobrinha.morreu = 1;

    //Substitui os caracteres de corpo e cabeca da cobrinha por 'X', sinalizando a morte da cobrinha.

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            if (jogo.mapa.matriz[i][j] == CARACTERE_CABECA_BAIXO || jogo.mapa.matriz[i][j] == CARACTERE_CABECA_CIMA || jogo.mapa.matriz[i][j] == CARACTERE_CABECA_ESQUERDA || jogo.mapa.matriz[i][j] == CARACTERE_CABECA_DIREITA || jogo.mapa.matriz[i][j] == CARACTERE_CORPO) {
                jogo.mapa.matriz[i][j] = CARACTERE_MORTE;
            }
        }
    }

    return jogo;
}

tVariaveis CresceCobrinha(tVariaveis jogo) {

    //Faz a cobrinha crescer e atualizar a coordenada do ultimo corpo, a ser analisada na proxima jogada.
    if (jogo.cobrinha.tam_corpo <TAM_COBRA){
    jogo.cobrinha.tam_corpo++;}
    jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo][0] = jogo.cobrinha.coord_ultimo_corpo[0];
    jogo.cobrinha.coordenadas_corpo[jogo.cobrinha.tam_corpo][1] = jogo.cobrinha.coord_ultimo_corpo[1];

    return jogo;
}

void ImprimeResultadoFinal(tVariaveis jogo) {

    //Imprime o resultado do jogo: se o usuario venceu ou perdeu.

    if (!jogo.cobrinha.morreu) {
        printf("Voce venceu!\nPontuacao final: %d\n", jogo.cobrinha.pontos);
    } else if (jogo.cobrinha.morreu == 1) {
        printf("Game over!\nPontuacao final: %d\n", jogo.cobrinha.pontos);
    }
}

//FUNCIONALIDADE EXTRA: ----------------------------TUNEIS--------------------------------------------------------

tVariaveis TrataTuneis(tVariaveis jogo) {

    if (jogo.jogada.linha_mov == jogo.mapa.tuneis[0][0] && jogo.jogada.coluna_mov == jogo.mapa.tuneis[0][1]) {

        //Avaliando o caractere da cabeca, para saber em que sentido sair pelo outro tunel:

        if (jogo.cobrinha.cabeca == CARACTERE_CABECA_BAIXO) {
            jogo.mapa.tuneis[1][0]++;
            //"Atravessando" o tunel, para a posicao (posicao 0 da coluna da matriz, refere-se a linha do mapa, e 1 a coluna do mapa) analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[1][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[1][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[1][0]--;

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_CIMA) {
            jogo.mapa.tuneis[1][0]--;
            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[1][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[1][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[1][0]++;

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_ESQUERDA) {
            jogo.mapa.tuneis[1][1]--;

            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[1][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[1][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[1][1]++;
        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_DIREITA) {
            jogo.mapa.tuneis[1][1]++;
            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[1][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[1][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[1][1]--;

        }
        //"Atravessando" o tunel, para a posicao analisada acima:




    } else if (jogo.jogada.linha_mov == jogo.mapa.tuneis[1][0] && jogo.jogada.coluna_mov == jogo.mapa.tuneis[1][1]) {

        //Avaliando o caractere da cabeca, para saber em que sentido sair pelo outro tunel:

        if (jogo.cobrinha.cabeca == CARACTERE_CABECA_BAIXO) {
            jogo.mapa.tuneis[0][0]++;

            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[0][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[0][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[0][0]--;


        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_CIMA) {
            jogo.mapa.tuneis[0][0]--;

            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[0][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[0][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[0][0]++;

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_ESQUERDA) {
            jogo.mapa.tuneis[0][1]--;

            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[0][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[0][1];

            jogo = MovimentaCobrinha(jogo);

            jogo.mapa.tuneis[0][1]++;

        } else if (jogo.cobrinha.cabeca == CARACTERE_CABECA_DIREITA) {
            jogo.mapa.tuneis[0][1]++;

            //"Atravessando" o tunel, para a posicao analisada acima:
            jogo.jogada.linha_mov = jogo.mapa.tuneis[0][0];
            jogo.jogada.coluna_mov = jogo.mapa.tuneis[0][1];

            jogo = MovimentaCobrinha(jogo);
            jogo.mapa.tuneis[0][1]--;
        }
    }

    return jogo;
}

tVariaveis BuscaTuneis(tVariaveis jogo) {

    int i = 0, j = 0, linha = 0, coluna = 1, tunel = 0;

    //Busca as posicoes dos tuneis.
    for (i = 0; i < jogo.mapa.linhas && tunel < QTD_TUNEIS; i++) {
        for (j = 0; j < jogo.mapa.colunas && tunel < QTD_TUNEIS; j++) {
            if (jogo.mapa.matriz[i][j] == CARACTERE_TUNEL && tunel < 2) {
                jogo.mapa.tuneis[tunel][linha] = i;
                jogo.mapa.tuneis[tunel][coluna] = j;
                tunel++;
            }
        }
    }

    return jogo;
}

//--------------------------------------------------HEATMAP-------------------------------------------------------

tVariaveis AtualizaHeatMap(tVariaveis jogo) {
    int i = 0, j = 0;

    //Atualiza o heatmap a cada jogada: aumenta o modulo da posicao por onde a cabeca passa.
    jogo.mapa.heatmap[jogo.jogada.linha_mov][jogo.jogada.coluna_mov]++;
    return jogo;
}

void ImprimeHeatMap(char*argv[], tVariaveis jogo) {

    int i = 0, j = 0;
    FILE*fileheatmap;
    char caminho_heatmap[TAM_ENTRADA];

    //Funcao responsavel por imprimir o heatmap no arquivo "heat_map.txt".

    sprintf(caminho_heatmap, "%s/saida/heatmap.txt", argv[1]);
    fileheatmap = fopen(caminho_heatmap, "w");

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            
            fprintf(fileheatmap, "%d", jogo.mapa.heatmap[i][j]);
            if (j<jogo.mapa.colunas-1){
                fprintf(fileheatmap, " ");
            }
        }
        fprintf(fileheatmap, "\n");
    }

    fclose(fileheatmap);
}

tVariaveis InicializaHeatMap(tVariaveis jogo) {
    int i = 0, j = 0;

    //Inicializa todas as posicoes do heatmap como 0, para comecarmos a contagem a partir de 0 acessos.
    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            jogo.mapa.heatmap[i][j] = 0;
        }
    }
    return jogo;
}

//-----------------------------------------------ESTATISTICAS-----------------------------------------------------

void ImprimeEstatisticas(tVariaveis jogo, char*argv[]) {

    int i = 0, j = 0;
    FILE*file_estatisticas;
    char caminho_heatmap[TAM_ENTRADA];

    /*Imprime os seguintes dados, coletados durante o jogo, no arquivo "estatisticas.txt".
        1. Numero de Movimentos;
        2. Numero de Movimentos Sem Pontuar;
        3. Numero de movimentos para cima;
        4. Numero de movimentos para esquerda;
        5. Numero de movimentos para direita;
     */
    sprintf(caminho_heatmap, "%s/saida/estatisticas.txt", argv[1]);
    file_estatisticas = fopen(caminho_heatmap, "w");

    fprintf(file_estatisticas, "Numero de movimentos: %d\n", jogo.dados.qtd_jogadas);
    fprintf(file_estatisticas, "Numero de movimentos sem pontuar: %d\n", jogo.dados.qtd_jogadas_nao_pontuadas);
    fprintf(file_estatisticas, "Numero de movimentos para baixo: %d\n", jogo.dados.qtd_movimentos_para_baixo);
    fprintf(file_estatisticas, "Numero de movimentos para cima: %d\n", jogo.dados.qtd_movimentos_para_cima);
    fprintf(file_estatisticas, "Numero de movimentos para esquerda: %d\n", jogo.dados.qtd_movimentos_para_esquerda);
    fprintf(file_estatisticas, "Numero de movimentos para direita: %d\n", jogo.dados.qtd_movimentos_para_direita);

    fclose(file_estatisticas);
}

//--------------------------------------------------RESUMO--------------------------------------------------------

void AtualizaResumo(tVariaveis jogo, char*argv[], int fim_de_jogo) {

    int i = 0, j = 0;
    FILE*file_resumo;
    char caminho_resumo[TAM_ENTRADA];

    //Faz a impressao do resumo a cada jogada, descrevendo suas consequencias, se existirem.

    sprintf(caminho_resumo, "%s/saida/resumo.txt", argv[1]);
    file_resumo = fopen(caminho_resumo, "a");

    if (fim_de_jogo) {
        if (jogo.venceu) {
            fprintf(file_resumo, "Movimento %d (%c) fez a cobra crescer para o tamanho %d, terminando o jogo\n", jogo.jogada.numero_jogada, jogo.jogada.direcao_giro, jogo.cobrinha.tam_corpo++);
        } else if (jogo.colisao) {
            fprintf(file_resumo, "Movimento %d (%c) resultou no fim de jogo por conta de colisao\n", jogo.jogada.numero_jogada, jogo.jogada.direcao_giro);
        }
    } else {

        if (jogo.gera_dinheiro) {
            fprintf(file_resumo, "Movimento %d (%c) gerou dinheiro\n", jogo.jogada.numero_jogada, jogo.jogada.direcao_giro);
        } else if (jogo.cresce_cobrinha) {
            fprintf(file_resumo, "Movimento %d (%c) fez a cobra crescer para o tamanho %d\n", jogo.jogada.numero_jogada, jogo.jogada.direcao_giro, jogo.cobrinha.tam_corpo++);

        }
    }

    fclose(file_resumo);
}

//-------------------------------------------------RANKING--------------------------------------------------------

void GeraRanking(tVariaveis jogo, char*argv[]) {

    FILE*file_ranking;
    char caminho_ranking[TAM_ENTRADA];
    int j = 0, i = 0, qtd_posicoes_acessadas = 0;

    //Procura a quantidade de posicoes pelas quais a cobrinha passou.

    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            if (jogo.mapa.heatmap[i][j] > 0) {
                qtd_posicoes_acessadas++;
            }
        }
    }

    int ranking [qtd_posicoes_acessadas][3], cont = 0;

    /*padrão:
        - ranking[qtd_posicoes_acessadas][0] corresponde a coordenada da linha;
        - ranking[qtd_posicoes_acessadas][1] corresponde a coordenada da coluna;
        - ranking[qtd_posicoes_acessadas][2] corresponde a quantidade de acessos;
     */

    //atribuindo os valores do heatmap (linha, coluna e quantidade de acessos) a uma matriz que junta esses dados
    for (i = 0; i < jogo.mapa.linhas; i++) {
        for (j = 0; j < jogo.mapa.colunas; j++) {
            if (jogo.mapa.heatmap[i][j] > 0) {

                ranking[cont][0] = i;
                ranking[cont][1] = j;
                ranking[cont][2] = jogo.mapa.heatmap[i][j];
                cont++;
            }
        }
    }

    OrdenaPosicoes(jogo, qtd_posicoes_acessadas, ranking);

    sprintf(caminho_ranking, "%s/saida/ranking.txt", argv[1]);
    file_ranking = fopen(caminho_ranking, "w");

    for (i = 0; i < qtd_posicoes_acessadas; i++) {
        fprintf(file_ranking, "(%d, %d) - %d\n", ranking[i][0], ranking [i][1], ranking [i][2]);
    }

    fclose(file_ranking);

}

void OrdenaPosicoes(tVariaveis jogo, int qtd_posicoes_acessadas, int ranking [qtd_posicoes_acessadas][3]) {

    /*Ordenando a matriz de acordo com so seguintes critérios de desempate, em ordem de prioridade:
        1. Maior Quantidade de Acessos;
        2. Menor Linha;
        3. Menor Coluna.
     */

    int i = 0, j = 0, maior_num_acessos [3], organizadora[3];

    for (i = 0; i < qtd_posicoes_acessadas; i++) {
        for (j = i + 1; j < qtd_posicoes_acessadas; j++) {

            if (ranking [j][2] > ranking [i][2]) {

                maior_num_acessos[2] = ranking[j][2];
                maior_num_acessos[1] = ranking[j][1];
                maior_num_acessos[0] = ranking[j][0];

                ranking [j][0] = ranking [i][0];
                ranking [j][1] = ranking [i][1];
                ranking [j][2] = ranking [i][2];

                ranking [i][0] = maior_num_acessos[0];
                ranking [i][1] = maior_num_acessos[1];
                ranking [i][2] = maior_num_acessos[2];
                
            } else if (ranking [j][2] == ranking [i][2]) {
                if (ranking [j][0] < ranking [i][0]) {
                    maior_num_acessos[2] = ranking[j][2];
                    maior_num_acessos[1] = ranking[j][1];
                    maior_num_acessos[0] = ranking[j][0];

                    ranking [j][0] = ranking [i][0];
                    ranking [j][1] = ranking [i][1];
                    ranking [j][2] = ranking [i][2];

                    ranking [i][0] = maior_num_acessos[0];
                    ranking [i][1] = maior_num_acessos[1];
                    ranking [i][2] = maior_num_acessos[2];

                } else if (ranking [j][0] == ranking [i][0]) {
                    if (ranking [j][1] < ranking [i][1]) {
                        maior_num_acessos[2] = ranking[j][2];
                        maior_num_acessos[1] = ranking[j][1];
                        maior_num_acessos[0] = ranking[j][0];

                        ranking [j][0] = ranking [i][0];
                        ranking [j][1] = ranking [i][1];
                        ranking [j][2] = ranking [i][2];

                        ranking [i][0] = maior_num_acessos[0];
                        ranking [i][1] = maior_num_acessos[1];
                        ranking [i][2] = maior_num_acessos[2];
                    }
                }
            }
        }
    }
}