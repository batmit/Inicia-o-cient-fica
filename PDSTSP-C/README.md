# PDSTSP-C em C

O programa agora trabalha exclusivamente com as 60 instâncias **PDSTSP-C**
de Nguyen e Hà, distribuídas pelo ORLab. A leitura anterior de mFSTSP e dos
TXT adaptados foi retirada. Esses dados antigos permanecem no disco, mas
não são usados pelo programa nem pelo script de download.

Fonte: https://orlab.com.vn/download/instances-for-the-parallel-drone-scheduling-traveling-salesman-problem-with-collective-drones/
Artigo: https://doi.org/10.1287/trsc.2022.1192

## Executar

Na pasta `PDSTSP-D`:

```powershell
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror main.c cv.c -lm -o pdstspc.exe
./pdstspc.exe instancias/PDSTSP-C/Upload-SuperDrone/instances/small/15-r-e.txt
```

Sem argumentos, o programa pede o caminho. `--preprocessar` executa somente
a leitura e a preparação das matrizes, sem construir solução.
O arquivo `other-params.txt` é procurado junto da instância e em até duas
pastas acima. Fora da estrutura original, informe:

```powershell
./pdstspc.exe caminho/instancia.txt --parametros caminho/other-params.txt --preprocessar
```

`./testar_leitura.ps1` compila, testa as 60 instâncias e atualiza também
`pdstspd.exe` como cópia do novo executável, para não manter um binário antigo.
`./testar_modelo.ps1` requer `g++` e compara o modelo energético portado para C
contra o C++ original. `./baixar_instancias.ps1` baixa apenas PDSTSP-C,
verifica SHA-256 e extrai o pacote original.

## Leitura e unidades

| Campo do arquivo | Campo em `InstanciaPDSTSPC` | Unidade |
|---|---|---|
| `NUM_DRONES` | `numero_drones` | quantidade |
| `TRUCK_SPEED` | `velocidade_caminhao_kmh` | km/h |
| `BATTERY_WEIGHT` | `bateria_kg` | kg |
| `BATTERY_CAPACITY` | `capacidade_ah` | Ah por bateria |
| `MAX_DISCHARGE_CURRENT` | `corrente_max_a` | A |
| `VOLTAGE` | `tensao_v` | V |
| `node x y weight` | `nos[i].id`, `x_km`, `y_km`, `peso_kg` | ID, km, km, kg |

O depósito é o nó 0. Os IDs devem ser consecutivos desde 0; a quantidade de
nós é obtida lendo a tabela até EOF, sem depender do nome do arquivo.
O leitor aceita linhas vazias, comentários `#`/`%` e parâmetros fora de ordem,
mas rejeita campos desconhecidos, duplicados, ausentes, não finitos e fora
dos limites. Parâmetros inteiros têm um limite defensivo de 1024.

`other-params.txt` fornece preparação, limite de drones por grupo, peso máximo
do gerador, massa da estrutura, número e raio das hélices, expoente de Peukert,
altura de voo, velocidades verticais e densidade do ar. `max_weight` é
preservado como metadado; não é tratado como capacidade individual de um drone.

## Pré-processamento

1. `distancias_caminhao_km[i*n+j]`: distância Manhattan entre os nós.
2. `tempos_caminhao_min[i*n+j]`: `60 * distancia / velocidade_caminhao_kmh`.
3. `distancias_drone_km[i]`: distância euclidiana do depósito ao cliente.
4. `opcoes_drone[i*(max_grupo+1)+k]`: duração, velocidades de ida/volta e
   consumo para uma missão com `k` drones. `max_grupo = min(numero_drones,
   max_combination)`. Combinações inviáveis têm tempo `INFINITY`.

O código energético foi portado do arquivo `energymodel.cpp` do pacote,
preservando constantes, busca angular, velocidades entre 4 e 28 m/s em
incrementos de 0,2 e reserva de bateria de 10%. A distância é convertida para
metros antes dessa chamada; o retorno é sem carga e cada missão começa com
bateria nova. Não se supõe que todos os valores entre `minimo_drones` e
`maximo_drones` sejam viáveis: consulte cada entrada da tabela.

A duração integrada atualmente é:

```
ceil(preparacao_min + (distancia_m/v_ida + distancia_m/v_volta
                      + 2*altura_m/v_subida + 2*altura_m/v_descida) / 60)
```

**Limite da reprodução do benchmark:** o pacote contém o cálculo energético,
mas não o código que integra esses resultados ao escalonamento. Velocidades,
consumo e viabilidade foram comparados com o código original em 117 combinações.
A fórmula de duração acima é explícita e inclui preparação e voo vertical;
não foi confirmada como a convenção exata que gerou todos os valores de
`bks.txt`. Por exemplo, a missão isolada do cliente 13 de `15-r-e` resulta em
27 min aqui e consta como 26 min no BKS. Não reporte gaps contra os BKS como
comparações equivalentes até reconciliar essa diferença. A rota publicada
do caminhão dessa instância, por outro lado, reproduz exatamente 92 min.

## Solução inicial

`heuristica` constrói uma rota com todos os clientes por vizinho mais próximo,
aplica 2-opt e transfere clientes para missões de drones quando isso reduz o
makespan. Para cada transferência, avalia os grupos viáveis e seleciona os
`k` drones que ficam disponíveis mais cedo. Todos começam e terminam a missão
juntos; caminhão e drones operam em paralelo a partir do depósito.

É uma heurística inicial gulosa, não um solver exato nem a meta-heurística
Ruin-and-Recreate do artigo. Pode parar em um ótimo local e não preenche
lacunas anteriores nas agendas dos drones. O caminhão tem capacidade e
autonomia ilimitadas neste modelo, e não foram acrescentados tempos de
serviço do caminhão inexistentes no arquivo.

`validar_solucao` confere cobertura única dos clientes, retorno ao depósito,
viabilidade e duração das missões, IDs dos drones, ausência de sobreposições
e consistência dos tempos e do makespan.

## API

```c
InstanciaPDSTSPC in;
Solucao solucao;
char erro[512];
inicializar_instancia(&in);
inicializar_solucao(&solucao);
if (ler_instancia(caminho, &in, erro, sizeof erro) &&
    preprocessar_instancia(&in, erro, sizeof erro) &&
    heuristica(&in, &solucao, erro, sizeof erro)) {
    imprimir_instancia(&in);
    imprimir_solucao(&solucao);
}
liberar_solucao(&solucao);
liberar_instancia(&in);
```

Libere a estrutura antes de ler outra instância ou construir outra solução.
O pré-processamento pode ser repetido: suas matrizes antigas são liberadas.
Arquivos originais do pacote não foram modificados.
