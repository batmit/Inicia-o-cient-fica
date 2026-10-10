/* Compara a portabilidade em C com o modelo original dos autores (C++). */
#include <cassert>
#include <cmath>
#include <cstdio>
#include "energymodel.h"
extern "C" {
#include "cv.h"
}

int main(int argc, char **argv) {
    assert(argc==2);
    InstanciaPDSTSPC in;
    char erro[512];
    assert(ler_instancia(argv[1],&in,erro,sizeof erro));
    assert(preprocessar_instancia(&in,erro,sizeof erro));
    const ParametrosEnergia &p=in.energia;
    EnergyModel model(p.altura_m,p.subida_ms,p.descida_ms,p.densidade_ar,p.peukert);
    size_t comparacoes=0;
    for (size_t i=1;i<in.numero_nos;i+=in.numero_nos>30 ? 17 : 1) {
        for (size_t k=1;k<=in.max_grupo;++k) {
            DroneConfig dc((int)k,p.estrutura_kg,in.bateria_kg,(int)p.numero_helices,
                p.raio_helice_m,in.capacidade_ah,in.tensao_v,in.corrente_max_a);
            auto ref=model.get_optimal_power(dc,in.nos[i].peso_kg,in.distancias_drone_km[i]*1000);
            const OpcaoDrone *op=opcao_drone(&in,i,k);
            assert((ref[0]>0)==std::isfinite(op->tempo_min));
            if(ref[0]>0) {
                assert(std::fabs(ref[0]-op->velocidade_ida_ms)<1e-8);
                assert(std::fabs(ref[1]-op->velocidade_volta_ms)<1e-8);
                assert(std::fabs(ref[2]-op->consumo_ah)<1e-8);
            }
            ++comparacoes;
        }
    }
    printf("OK: %zu combinacoes comparadas com o modelo energetico original.\n",comparacoes);
    liberar_instancia(&in);
}
