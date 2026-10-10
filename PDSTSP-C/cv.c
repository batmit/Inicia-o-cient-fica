#include "cv.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhar(char *erro, size_t tamanho, const char *mensagem) {
    if (erro && tamanho) snprintf(erro, tamanho, "%s", mensagem);
    return 0;
}

void inicializar_instancia(InstanciaPDSTSPC *in) {
    if (in) memset(in, 0, sizeof *in);
}

static void liberar_preprocessamento(InstanciaPDSTSPC *in) {
    size_t i;
    free(in->distancias_caminhao_km);
    free(in->tempos_caminhao_min);
    free(in->distancias_drone_km);
    free(in->opcoes_drone);
    in->distancias_caminhao_km = NULL;
    in->tempos_caminhao_min = NULL;
    in->distancias_drone_km = NULL;
    in->opcoes_drone = NULL;
    in->max_grupo = 0;
    for (i = 0; i < in->numero_nos; ++i)
        in->nos[i].minimo_drones = in->nos[i].maximo_drones = 0;
}

void liberar_instancia(InstanciaPDSTSPC *in) {
    if (!in) return;
    liberar_preprocessamento(in);
    free(in->nos);
    inicializar_instancia(in);
}

static char *aparar(char *s) {
    char *fim;
    while (isspace((unsigned char)*s)) ++s;
    fim = s + strlen(s);
    while (fim > s && isspace((unsigned char)fim[-1])) --fim;
    *fim = '\0';
    return s;
}

static int ler_linha(FILE *f, char *s, size_t capacidade, size_t *numero) {
    while (fgets(s, (int)capacidade, f)) {
        char *p;
        ++*numero;
        if (!strchr(s, '\n') && !feof(f)) return -1;
        p = aparar(s);
        if (*p && *p != '#' && *p != '%') {
            memmove(s, p, strlen(p) + 1);
            return 1;
        }
    }
    return ferror(f) ? -1 : 0;
}

static int numero_real(const char *s, double *valor) {
    char *fim;
    errno = 0;
    *valor = strtod(s, &fim);
    if (fim == s) return 0;
    while (isspace((unsigned char)*fim)) ++fim;
    return !errno && !*fim && isfinite(*valor);
}

static int inteiro_positivo(double d) {
    /* Limite defensivo, acima de todos os valores do benchmark. */
    return d >= 1 && d <= 1024 && floor(d) == d;
}

static int ler_parametro(char *linha, const char *const *nomes, size_t n,
                         double *valores, unsigned *vistos) {
    char *virgula = strchr(linha, ',');
    size_t i;
    if (!virgula) return 0;
    *virgula++ = '\0';
    for (i = 0; i < n; ++i) {
        if (!strcmp(aparar(linha), nomes[i])) {
            if ((*vistos & (1u << i)) || !numero_real(virgula, &valores[i])) return 0;
            *vistos |= 1u << i;
            return 1;
        }
    }
    return 0;
}

static char *ultima_barra(char *s) {
    char *a = strrchr(s, '/'), *b = strrchr(s, '\\');
    return !a ? b : (!b || a > b ? a : b);
}

static FILE *abrir_parametros(const char *arquivo, const char *explicito) {
    char pasta[4096], caminho[4096], *barra;
    size_t nivel;
    if (explicito) return fopen(explicito, "r");
    if (strlen(arquivo) >= sizeof pasta) return NULL;
    strcpy(pasta, arquivo);
    barra = ultima_barra(pasta);
    if (barra) *barra = '\0'; else strcpy(pasta, ".");
    for (nivel = 0; nivel < 3; ++nivel) {
        FILE *f;
        int n = snprintf(caminho, sizeof caminho, "%s/other-params.txt", pasta);
        if (n < 0 || (size_t)n >= sizeof caminho) return NULL;
        f = fopen(caminho, "r");
        if (f) return f;
        barra = ultima_barra(pasta);
        if (!barra) break;
        *barra = '\0';
    }
    return NULL;
}

int ler_instancia_pdstspc(const char *arquivo, const char *params,
                         InstanciaPDSTSPC *in, char *erro, size_t tamanho) {
    static const char *const cabecalho[] = {"NUM_DRONES", "TRUCK_SPEED",
        "BATTERY_WEIGHT", "BATTERY_CAPACITY", "MAX_DISCHARGE_CURRENT", "VOLTAGE"};
    static const char *const complementos[] = {"drone_prepare_time", "max_combination",
        "max_weight", "frame_weight", "num_propellers", "rotor_blade_radius",
        "peukert_constant", "h_flight", "v_takeoff", "v_landing", "air_density_rho"};
    InstanciaPDSTSPC nova;
    FILE *f = NULL;
    char linha[1024], mensagem[256];
    double v[11] = {0};
    unsigned vistos = 0;
    size_t numero = 0, i;
    int status, tabela = 0;
    const char *motivo = "formato PDSTSP-C invalido";
    inicializar_instancia(&nova);
    if (!in || !arquivo) return falhar(erro, tamanho, "argumento nulo");
    inicializar_instancia(in);
    f = fopen(arquivo, "r");
    if (!f) return falhar(erro, tamanho, "nao foi possivel abrir a instancia PDSTSP-C");
    while ((status = ler_linha(f, linha, sizeof linha, &numero)) == 1) {
        if (!tabela) {
            char a[32], b[32], c[32], d[32], extra;
            if (sscanf(linha, "%31s %31s %31s %31s %c", a,b,c,d,&extra) == 4 &&
                !strcmp(a,"node") && !strcmp(b,"x") && !strcmp(c,"y") && !strcmp(d,"weight")) {
                if (vistos != 63u) { motivo = "cabecalho incompleto"; goto falha; }
                tabela = 1;
                continue;
            }
            if (!ler_parametro(linha, cabecalho, 6, v, &vistos)) {
                motivo = "parametro desconhecido, duplicado ou invalido"; goto falha;
            }
        } else {
            Cliente cliente = {0}, *novos;
            char id[64], x[128], y[128], peso[128], extra, *fim;
            unsigned long long ident;
            size_t n = nova.numero_nos;
            if (sscanf(linha, "%63s %127s %127s %127s %c", id,x,y,peso,&extra) != 4 ||
                id[0] == '-' || !numero_real(x,&cliente.x_km) ||
                !numero_real(y,&cliente.y_km) || !numero_real(peso,&cliente.peso_kg)) goto falha;
            errno = 0;
            ident = strtoull(id, &fim, 10);
            if (errno || fim == id || *fim || ident != n || cliente.peso_kg < 0 ||
                (n == 0 && cliente.peso_kg != 0)) {
                motivo = "IDs devem ser consecutivos desde 0 e pesos nao negativos (deposito: 0)";
                goto falha;
            }
            if (n >= SIZE_MAX / sizeof *novos) { motivo = "instancia muito grande"; goto falha; }
            novos = realloc(nova.nos, (n + 1) * sizeof *novos);
            if (!novos) { motivo = "memoria insuficiente"; goto falha; }
            nova.nos = novos;
            cliente.id = n;
            nova.nos[n] = cliente;
            nova.numero_nos++;
        }
    }
    if (status < 0 || !tabela || nova.numero_nos < 2) {
        motivo = "arquivo incompleto, erro de leitura ou linha longa"; goto falha;
    }
    if (!inteiro_positivo(v[0]) || v[1] <= 0 || v[2] < 0 || v[3] <= 0 || v[4] <= 0 || v[5] <= 0) {
        motivo = "parametros dos veiculos fora dos limites"; goto falha;
    }
    nova.numero_drones = (size_t)v[0];
    nova.velocidade_caminhao_kmh = v[1];
    nova.bateria_kg = v[2]; nova.capacidade_ah = v[3];
    nova.corrente_max_a = v[4]; nova.tensao_v = v[5];
    fclose(f);
    f = abrir_parametros(arquivo, params);
    numero = 0;
    if (!f) { motivo = "other-params.txt nao encontrado; informe o caminho explicitamente"; goto falha; }
    vistos = 0;
    while ((status = ler_linha(f, linha, sizeof linha, &numero)) == 1) {
        if (!ler_parametro(linha, complementos, 11, v, &vistos)) {
            motivo = "other-params.txt: parametro desconhecido, duplicado ou invalido"; goto falha;
        }
    }
    if (status < 0 || vistos != 2047u || v[0] < 0 || !inteiro_positivo(v[1]) ||
        !inteiro_positivo(v[4]) || v[6] < 1) {
        motivo = "other-params.txt incompleto ou invalido"; goto falha;
    }
    for (i = 2; i < 11; ++i) if (v[i] <= 0) { motivo = "parametro fisico deve ser positivo"; goto falha; }
    nova.energia = (ParametrosEnergia){v[0], (size_t)v[1], v[2], v[3], (size_t)v[4],
                                      v[5], v[6], v[7], v[8], v[9], v[10]};
    fclose(f);
    *in = nova;
    if (erro && tamanho) erro[0] = '\0';
    return 1;
falha:
    if (f) fclose(f);
    liberar_instancia(&nova);
    snprintf(mensagem, sizeof mensagem, "linha %zu: %s", numero, motivo);
    return falhar(erro, tamanho, mensagem);
}

int ler_instancia(const char *arquivo, InstanciaPDSTSPC *in, char *erro, size_t tamanho) {
    return ler_instancia_pdstspc(arquivo, NULL, in, erro, tamanho);
}

double calcular_Tij(double distancia_km, double velocidade_kmh) {
    if (!isfinite(distancia_km) || distancia_km < 0 ||
        !isfinite(velocidade_kmh) || velocidade_kmh <= 0) return NAN;
    return 60.0 * distancia_km / velocidade_kmh;
}

double tempo_caminhao_min(const InstanciaPDSTSPC *in, size_t i, size_t j) {
    if (!in || !in->tempos_caminhao_min || i >= in->numero_nos || j >= in->numero_nos) return NAN;
    return in->tempos_caminhao_min[i * in->numero_nos + j];
}

const OpcaoDrone *opcao_drone(const InstanciaPDSTSPC *in, size_t i, size_t k) {
    if (!in || !in->opcoes_drone || i >= in->numero_nos || k == 0 || k > in->max_grupo) return NULL;
    return &in->opcoes_drone[i * (in->max_grupo + 1) + k];
}

/* Portado de Upload-SuperDrone/power-consumption-calculation/energymodel.cpp.
 * Mantem constantes, grade de velocidades e reserva dos autores.
 * Duracao inclui preparacao e duas subidas/descidas.
 */
static double k2(const InstanciaPDSTSPC *in, size_t k) {
    return sqrt(2 * in->energia.densidade_ar * k * in->energia.numero_helices *
                3.14159 * in->energia.raio_helice_m * in->energia.raio_helice_m);
}

static double potencia_vertical(const InstanciaPDSTSPC *in, size_t k, double peso, double v) {
    double t = 9.81 * (in->energia.estrutura_kg + in->bateria_kg + peso / k);
    double coef = k2(in,k);
    return 0.8554 * t * (v/2 + sqrt(v*v/4 + t/(coef*coef))) + 0.3177 * pow(t,1.5);
}

static double potencia_voo(const InstanciaPDSTSPC *in, size_t k, double peso, double v) {
    double t = 9.81 * (in->energia.estrutura_kg + in->bateria_kg + peso/k);
    double c5 = 0.0279 / 0.15 * in->energia.raio_helice_m;
    double alpha;
    for (alpha = 0; alpha < 90; alpha += 0.5) {
        double rad = alpha * 3.14159 / 180;
        double vc = v * cos(rad);
        if (fabs(tan(rad) * (t - c5*vc*vc) - 0.0296*v*v) <= 5) {
            double a = t - c5*vc*vc, b = 0.0296*v*v;
            return (0.8554/k2(in,k) + 0.3177) * pow(a*a + b*b,0.75) + 0.0296*v*v*v;
        }
    }
    return NAN;
}

static double corrente_efetiva(const InstanciaPDSTSPC *in, double corrente) {
    return corrente * pow(corrente / in->capacidade_ah, in->energia.peukert - 1);
}

static OpcaoDrone calcular_missao(const InstanciaPDSTSPC *in, size_t k, double peso, double metros) {
    OpcaoDrone melhor = {INFINITY, 0, 0, 0};
    double ts = in->energia.altura_m / in->energia.subida_ms;
    double td = in->energia.altura_m / in->energia.descida_ms;
    double is = potencia_vertical(in,k,peso,in->energia.subida_ms) / in->tensao_v;
    double id = potencia_vertical(in,k,peso,in->energia.descida_ms) / in->tensao_v;
    double vs = potencia_vertical(in,k,0,in->energia.subida_ms) / in->tensao_v;
    double vd = potencia_vertical(in,k,0,in->energia.descida_ms) / in->tensao_v;
    double restante = in->capacidade_ah -
        (corrente_efetiva(in,is) + corrente_efetiva(in,vs))*ts/3600 -
        (corrente_efetiva(in,id) + corrente_efetiva(in,vd))*td/3600;
    double reserva = 0.1 * in->capacidade_ah, melhor_inverso = INFINITY;
    double velocidades[121], correntes[121], efetivas[121], v;
    size_t n = 0, j;
    if (!isfinite(restante) || restante < reserva || is > in->corrente_max_a) return melhor;
    for (v = 28; v >= 4 && n < 121; v -= 0.2) {
        velocidades[n] = v;
        correntes[n] = potencia_voo(in,k,0,v) / in->tensao_v;
        efetivas[n] = corrente_efetiva(in,correntes[n]);
        ++n;
    }
    for (v = 4; v <= 28; v += 0.2) {
        double corrente = potencia_voo(in,k,peso,v) / in->tensao_v;
        double apos_ida = restante - corrente_efetiva(in,corrente)*metros/v/3600;
        if (!isfinite(apos_ida) || apos_ida < reserva || corrente > in->corrente_max_a) continue;
        for (j = 0; j < n; ++j) {
            double apos_volta = apos_ida - efetivas[j]*metros/velocidades[j]/3600;
            double inverso = 1/v + 1/velocidades[j];
            double consumo = in->capacidade_ah - apos_volta;
            if (!isfinite(apos_volta) || apos_volta < reserva || correntes[j] > in->corrente_max_a) continue;
            if (inverso < melhor_inverso || (inverso == melhor_inverso && consumo < melhor.consumo_ah)) {
                melhor_inverso = inverso;
                melhor.velocidade_ida_ms = v;
                melhor.velocidade_volta_ms = velocidades[j];
                melhor.consumo_ah = consumo;
            }
        }
    }
    if (isfinite(melhor_inverso))
        melhor.tempo_min = ceil(in->energia.preparacao_min + (metros*melhor_inverso + 2*(ts+td))/60);
    return melhor;
}

int preprocessar_instancia(InstanciaPDSTSPC *in, char *erro, size_t tamanho) {
    size_t i,j,k,n,total,opcoes;
    if (!in || !in->nos || in->numero_nos < 2 || !in->numero_drones ||
        !in->energia.max_combinacao) return falhar(erro,tamanho,"instancia nao carregada");
    liberar_preprocessamento(in);
    n = in->numero_nos;
    in->max_grupo = in->numero_drones < in->energia.max_combinacao ? in->numero_drones : in->energia.max_combinacao;
    if (n > SIZE_MAX/n || n*n > SIZE_MAX/sizeof(double) ||
        n > SIZE_MAX/(in->max_grupo+1) || n*(in->max_grupo+1) > SIZE_MAX/sizeof(OpcaoDrone))
        return falhar(erro,tamanho,"matrizes excedem o limite de memoria");
    total = n*n;
    opcoes = n*(in->max_grupo+1);
    in->distancias_caminhao_km = calloc(total,sizeof(double));
    in->tempos_caminhao_min = calloc(total,sizeof(double));
    in->distancias_drone_km = calloc(n,sizeof(double));
    in->opcoes_drone = calloc(opcoes,sizeof(OpcaoDrone));
    if (!in->distancias_caminhao_km || !in->tempos_caminhao_min ||
        !in->distancias_drone_km || !in->opcoes_drone) {
        liberar_preprocessamento(in);
        return falhar(erro,tamanho,"memoria insuficiente para preprocessar");
    }
    for (i = 0; i < opcoes; ++i) in->opcoes_drone[i].tempo_min = INFINITY;
    for (i = 0; i < n; ++i) {
        in->distancias_drone_km[i] = hypot(in->nos[i].x_km - in->nos[0].x_km,
                                          in->nos[i].y_km - in->nos[0].y_km);
        for (j = 0; j < n; ++j) {
            double d = fabs(in->nos[i].x_km-in->nos[j].x_km) + fabs(in->nos[i].y_km-in->nos[j].y_km);
            double t = calcular_Tij(d,in->velocidade_caminhao_kmh);
            if (!isfinite(t) || !isfinite(in->distancias_drone_km[i]*1000)) {
                liberar_preprocessamento(in);
                return falhar(erro,tamanho,"coordenadas ou velocidade geram tempos invalidos");
            }
            in->distancias_caminhao_km[i*n+j] = d;
            in->tempos_caminhao_min[i*n+j] = t;
        }
        if (i == 0) continue;
        for (k = 1; k <= in->max_grupo; ++k) {
            OpcaoDrone *op = &in->opcoes_drone[i*(in->max_grupo+1)+k];
            *op = calcular_missao(in,k,in->nos[i].peso_kg,in->distancias_drone_km[i]*1000);
            if (isfinite(op->tempo_min)) {
                if (!in->nos[i].minimo_drones) in->nos[i].minimo_drones = k;
                in->nos[i].maximo_drones = k;
            }
        }
    }
    return 1;
}

void imprimir_instancia(const InstanciaPDSTSPC *in) {
    size_t i, exclusivos = 0;
    printf("PDSTSP-C: %zu clientes + deposito; %zu drones; caminhao %.2f km/h\n",
           in->numero_nos-1,in->numero_drones,in->velocidade_caminhao_kmh);
    printf("Bateria: %.2f kg, %.2f Ah, %.2f V; corrente maxima %.2f A\n",
           in->bateria_kg,in->capacidade_ah,in->tensao_v,in->corrente_max_a);
    if (!in->opcoes_drone) return;
    for (i = 1; i < in->numero_nos; ++i) if (!in->nos[i].minimo_drones) ++exclusivos;
    printf("Preprocessamento: km, kg e minutos; grupos de ate %zu drones\n",in->max_grupo);
    printf("Clientes: %zu somente caminhao; %zu com alguma opcao de drones\n",
           exclusivos,in->numero_nos-1-exclusivos);
    printf("Trajeto 0 -> 1: %.6f km; %.6f min\n",in->distancias_caminhao_km[1],tempo_caminhao_min(in,0,1));
}

void inicializar_solucao(Solucao *s) { if (s) memset(s,0,sizeof *s); }

void liberar_solucao(Solucao *s) {
    size_t i;
    if (!s) return;
    for (i = 0; i < s->numero_missoes; ++i) free(s->missoes[i].drones);
    free(s->missoes);
    free(s->rota);
    inicializar_solucao(s);
}

static double custo_rota(const InstanciaPDSTSPC *in, const Solucao *s) {
    size_t i;
    double total = 0;
    for (i = 1; i < s->tamanho_rota; ++i)
        total += tempo_caminhao_min(in,s->rota[i-1],s->rota[i]);
    return total;
}

static void dois_opt(const InstanciaPDSTSPC *in, Solucao *s) {
    int mudou;
    do {
        size_t i,j;
        mudou = 0;
        for (i = 1; i+2 < s->tamanho_rota && !mudou; ++i) {
            for (j = i+1; j+1 < s->tamanho_rota; ++j) {
                double antes = tempo_caminhao_min(in,s->rota[i-1],s->rota[i]) +
                               tempo_caminhao_min(in,s->rota[j],s->rota[j+1]);
                double depois = tempo_caminhao_min(in,s->rota[i-1],s->rota[j]) +
                                tempo_caminhao_min(in,s->rota[i],s->rota[j+1]);
                if (depois < antes - 1e-9) {
                    size_t a = i,b = j;
                    while (a < b) {
                        size_t aux = s->rota[a]; s->rota[a++] = s->rota[b]; s->rota[b--] = aux;
                    }
                    mudou = 1;
                    break;
                }
            }
        }
    } while (mudou);
    s->tempo_caminhao_min = custo_rota(in,s);
}

/* Ordena os IDs por disponibilidade, para escolher os k primeiros. */
static void ordenar_drones(const double *livre, size_t *ordem, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        size_t j = i;
        ordem[i] = i;
        while (j && livre[ordem[j-1]] > livre[ordem[j]]) {
            size_t aux = ordem[j]; ordem[j] = ordem[j-1]; ordem[j-1] = aux;
            --j;
        }
    }
}

int heuristica(const InstanciaPDSTSPC *in, Solucao *s, char *erro, size_t tamanho) {
    unsigned char *visitado;
    double *livre, fim_drones = 0;
    size_t *ordem, n, i,j;
    if (!s || !in || !in->opcoes_drone || !in->tempos_caminhao_min)
        return falhar(erro,tamanho,"preprocesse a instancia antes da heuristica");
    inicializar_solucao(s);
    n = in->numero_nos;
    s->rota = calloc(n+1,sizeof(size_t));
    s->missoes = calloc(n-1,sizeof(MissaoDrone));
    visitado = calloc(n,1);
    livre = calloc(in->numero_drones,sizeof(double));
    ordem = calloc(in->numero_drones,sizeof(size_t));
    if (!s->rota || !s->missoes || !visitado || !livre || !ordem) goto memoria;
    visitado[0] = 1;
    s->tamanho_rota = n+1;
    for (i = 1; i < n; ++i) {
        size_t melhor = 0;
        double menor = INFINITY;
        for (j = 1; j < n; ++j) if (!visitado[j]) {
            double t = tempo_caminhao_min(in,s->rota[i-1],j);
            if (t < menor) { menor = t; melhor = j; }
        }
        s->rota[i] = melhor;
        visitado[melhor] = 1;
    }
    dois_opt(in,s);
    s->makespan_min = s->tempo_caminhao_min;
    for (;;) {
        size_t posicao = 0, grupo = 0, k;
        double melhor = s->makespan_min, inicio = 0, fim = 0;
        ordenar_drones(livre,ordem,in->numero_drones);
        for (i = 1; i+1 < s->tamanho_rota; ++i) {
            size_t cliente = s->rota[i];
            double economia = tempo_caminhao_min(in,s->rota[i-1],cliente) +
                tempo_caminhao_min(in,cliente,s->rota[i+1]) -
                tempo_caminhao_min(in,s->rota[i-1],s->rota[i+1]);
            for (k = 1; k <= in->max_grupo; ++k) {
                const OpcaoDrone *op = opcao_drone(in,cliente,k);
                double t0, t1, objetivo;
                if (!isfinite(op->tempo_min)) continue;
                t0 = livre[ordem[k-1]];
                t1 = t0 + op->tempo_min;
                objetivo = fmax(s->tempo_caminhao_min-economia,fmax(fim_drones,t1));
                if (objetivo < melhor - 1e-9) {
                    melhor = objetivo; posicao = i; grupo = k; inicio = t0; fim = t1;
                }
            }
        }
        if (!posicao) break;
        {
            MissaoDrone *missao = &s->missoes[s->numero_missoes];
            missao->drones = calloc(grupo,sizeof(size_t));
            if (!missao->drones) goto memoria;
            missao->cliente = s->rota[posicao];
            missao->quantidade_drones = grupo;
            missao->inicio_min = inicio; missao->fim_min = fim;
            ++s->numero_missoes;
            for (k = 0; k < grupo; ++k) {
                missao->drones[k] = ordem[k]; livre[ordem[k]] = fim;
            }
            fim_drones = fmax(fim_drones,fim);
            memmove(s->rota+posicao,s->rota+posicao+1,
                    (s->tamanho_rota-posicao-1)*sizeof(size_t));
            --s->tamanho_rota;
            dois_opt(in,s);
            s->makespan_min = fmax(s->tempo_caminhao_min,fim_drones);
        }
    }
    free(visitado); free(livre); free(ordem);
    if (!validar_solucao(in,s,erro,tamanho)) { liberar_solucao(s); return 0; }
    return 1;
memoria:
    free(visitado); free(livre); free(ordem);
    liberar_solucao(s);
    return falhar(erro,tamanho,"memoria insuficiente para construir solucao");
}

int validar_solucao(const InstanciaPDSTSPC *in, const Solucao *s, char *erro, size_t tamanho) {
    unsigned char *atendido;
    size_t i,j,k,l;
    double caminhao = 0, fim = 0;
    const char *motivo = "solucao invalida";
    if (!in || !s || !s->rota || !in->opcoes_drone || !in->tempos_caminhao_min ||
        s->tamanho_rota < 2 || s->tamanho_rota > in->numero_nos+1 ||
        s->numero_missoes >= in->numero_nos || (s->numero_missoes && !s->missoes) ||
        s->rota[0] != 0 || s->rota[s->tamanho_rota-1] != 0)
        return falhar(erro,tamanho,"rota deve iniciar e terminar no deposito");
    atendido = calloc(in->numero_nos,1);
    if (!atendido) return falhar(erro,tamanho,"memoria insuficiente para validar");
    for (i = 1; i < s->tamanho_rota; ++i) {
        size_t cliente = s->rota[i];
        if (cliente >= in->numero_nos) goto falha;
        if (i+1 < s->tamanho_rota) {
            if (!cliente || atendido[cliente]) { motivo = "cliente repetido na rota"; goto falha; }
            atendido[cliente] = 1;
        }
        caminhao += tempo_caminhao_min(in,s->rota[i-1],cliente);
    }
    for (i = 0; i < s->numero_missoes; ++i) {
        const MissaoDrone *m = &s->missoes[i];
        const OpcaoDrone *op;
        if (!m->cliente || m->cliente >= in->numero_nos || atendido[m->cliente]) {
            motivo = "cliente repetido ou invalido nas missoes"; goto falha;
        }
        op = opcao_drone(in,m->cliente,m->quantidade_drones);
        if (!op || !m->drones || !isfinite(op->tempo_min) ||
            !isfinite(m->inicio_min) || !isfinite(m->fim_min) || m->inicio_min < 0 ||
            fabs(m->fim_min-m->inicio_min-op->tempo_min) > 1e-7) {
            motivo = "missao inviavel ou duracao incorreta"; goto falha;
        }
        atendido[m->cliente] = 1;
        fim = fmax(fim,m->fim_min);
        for (j = 0; j < m->quantidade_drones; ++j) {
            if (m->drones[j] >= in->numero_drones) goto falha;
            for (k = 0; k < j; ++k) if (m->drones[j] == m->drones[k]) {
                motivo = "drone duplicado no grupo"; goto falha;
            }
            for (k = 0; k < i; ++k) {
                const MissaoDrone *anterior = &s->missoes[k];
                if (m->inicio_min < anterior->fim_min-1e-9 && anterior->inicio_min < m->fim_min-1e-9)
                    for (l = 0; l < anterior->quantidade_drones; ++l)
                        if (m->drones[j] == anterior->drones[l]) {
                            motivo = "missoes sobrepostas para o mesmo drone"; goto falha;
                        }
            }
        }
    }
    for (i = 1; i < in->numero_nos; ++i) if (!atendido[i]) {
        motivo = "cliente nao atendido"; goto falha;
    }
    if (!isfinite(s->tempo_caminhao_min) || !isfinite(s->makespan_min) ||
        fabs(caminhao-s->tempo_caminhao_min) > 1e-7 || fabs(fmax(caminhao,fim)-s->makespan_min) > 1e-7) {
        motivo = "tempo da rota ou makespan inconsistente"; goto falha;
    }
    free(atendido);
    return 1;
falha:
    free(atendido);
    return falhar(erro,tamanho,motivo);
}

void imprimir_solucao(const Solucao *s) {
    size_t i,j;
    printf("Solucao inicial valida (heuristica, sem garantia de otimo)\nRota:");
    for (i = 0; i < s->tamanho_rota; ++i) printf(" %zu",s->rota[i]);
    printf("\nTempo do caminhao: %.2f min\n",s->tempo_caminhao_min);
    for (i = 0; i < s->numero_missoes; ++i) {
        const MissaoDrone *m = &s->missoes[i];
        printf("Cliente %zu: drones",m->cliente);
        for (j = 0; j < m->quantidade_drones; ++j) printf(" %zu",m->drones[j]);
        printf("; inicio %.2f; fim %.2f min\n",m->inicio_min,m->fim_min);
    }
    printf("Makespan: %.2f min\n",s->makespan_min);
}
