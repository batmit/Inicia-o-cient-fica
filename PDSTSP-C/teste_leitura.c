#include "cv.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void testar_sincronizacao(void) {
    InstanciaPDSTSPC in;
    Solucao s;
    Cliente nos[4] = {{0}};
    double matriz[16];
    OpcaoDrone opcoes[12];
    size_t rota[] = {0,3,0}, grupo1[] = {0,1}, grupo2[] = {0};
    MissaoDrone missoes[] = {{1,2,grupo1,0,10},{2,1,grupo2,10,15}};
    char erro[512];
    size_t i;
    inicializar_instancia(&in); inicializar_solucao(&s);
    for (i = 0; i < 16; ++i) matriz[i] = i/4 == i%4 ? 0 : 2;
    for (i = 0; i < 12; ++i) opcoes[i] = (OpcaoDrone){INFINITY,0,0,0};
    opcoes[1*3+2].tempo_min = 10; opcoes[2*3+1].tempo_min = 5;
    in.numero_nos=4; in.numero_drones=2; in.max_grupo=2; in.nos=nos;
    in.tempos_caminhao_min=matriz; in.opcoes_drone=opcoes;
    s.rota=rota; s.tamanho_rota=3; s.missoes=missoes; s.numero_missoes=2;
    s.tempo_caminhao_min=4; s.makespan_min=15;
    assert(validar_solucao(&in,&s,erro,sizeof erro));
    missoes[1].inicio_min=0; missoes[1].fim_min=5; s.makespan_min=10;
    assert(!validar_solucao(&in,&s,erro,sizeof erro));
    missoes[1].inicio_min=10; missoes[1].fim_min=15; s.makespan_min=15;
    grupo1[1]=0;
    assert(!validar_solucao(&in,&s,erro,sizeof erro));
    grupo1[1]=1; missoes[1].cliente=1;
    assert(!validar_solucao(&in,&s,erro,sizeof erro));
    missoes[1].cliente=2; s.makespan_min=NAN;
    assert(!validar_solucao(&in,&s,erro,sizeof erro));
}

int main(int argc, char **argv) {
    InstanciaPDSTSPC in;
    Solucao s;
    char erro[512];
    size_t i,j,k;
    double rota_sequencial=0;
    assert(argc==2);
    assert(ler_instancia(argv[1],&in,erro,sizeof erro));
    assert(preprocessar_instancia(&in,erro,sizeof erro));
    assert(in.numero_nos>=16 && in.numero_nos<=201);
    assert(in.velocidade_caminhao_kmh==30);
    assert(in.nos[0].peso_kg==0);
    for (i=0;i<in.numero_nos;++i) {
        size_t min=0,max=0;
        assert(in.nos[i].id==i);
        assert(tempo_caminhao_min(&in,i,i)==0);
        for (j=0;j<in.numero_nos;++j) {
            double d=fabs(in.nos[i].x_km-in.nos[j].x_km)+fabs(in.nos[i].y_km-in.nos[j].y_km);
            assert(fabs(tempo_caminhao_min(&in,i,j)-2*d)<1e-9);
            assert(tempo_caminhao_min(&in,i,j)==tempo_caminhao_min(&in,j,i));
        }
        for (k=1;k<=in.max_grupo;++k) {
            const OpcaoDrone *op=opcao_drone(&in,i,k);
            assert(!isnan(op->tempo_min));
            if (isfinite(op->tempo_min)) {
                assert(i>0 && op->tempo_min>=in.energia.preparacao_min);
                assert(floor(op->tempo_min)==op->tempo_min);
                assert(op->consumo_ah<=0.9*in.capacidade_ah+1e-9);
                assert(op->velocidade_ida_ms>=4 && op->velocidade_ida_ms<=28+1e-9);
                assert(op->velocidade_volta_ms>=4-1e-9 && op->velocidade_volta_ms<=28);
                if (!min) min=k;
                max=k;
            }
        }
        assert(in.nos[i].minimo_drones==min && in.nos[i].maximo_drones==max);
        if (i) rota_sequencial+=tempo_caminhao_min(&in,i-1,i);
    }
    rota_sequencial+=tempo_caminhao_min(&in,in.numero_nos-1,0);
    assert(isnan(calcular_Tij(1,0)) && isnan(calcular_Tij(-1,30)));
    assert(isnan(tempo_caminhao_min(&in,in.numero_nos,0)));
    assert(!opcao_drone(&in,1,0) && !opcao_drone(&in,1,in.max_grupo+1));
    assert(heuristica(&in,&s,erro,sizeof erro));
    assert(validar_solucao(&in,&s,erro,sizeof erro));
    assert(s.makespan_min<=rota_sequencial+1e-9);
    liberar_solucao(&s);
    if (strstr(argv[1],"15-r-e.txt")) {
        size_t rota_bks[]={0,2,5,15,12,8,6,9,3,10,1,7,0};
        double t=0;
        for(i=1;i<sizeof rota_bks/sizeof *rota_bks;++i)
            t+=tempo_caminhao_min(&in,rota_bks[i-1],rota_bks[i]);
        assert(t==92); /* rota publicada em bks.txt */
        printf("Missao 13/1: %.0f min; sequencia 11,14,4/2: %.0f min\n",
            opcao_drone(&in,13,1)->tempo_min,
            opcao_drone(&in,11,2)->tempo_min+opcao_drone(&in,14,2)->tempo_min+opcao_drone(&in,4,2)->tempo_min);
        printf("Voo horizontal 13/1: %.9f min\n",in.distancias_drone_km[13]*1000/60*
            (1/opcao_drone(&in,13,1)->velocidade_ida_ms+1/opcao_drone(&in,13,1)->velocidade_volta_ms));
        assert(preprocessar_instancia(&in,erro,sizeof erro)); /* reutilizacao sem vazamento */
    }
    liberar_instancia(&in);
    assert(!in.nos && !in.opcoes_drone && !in.tempos_caminhao_min);
    liberar_instancia(&in);
    testar_sincronizacao();
    return 0;
}
