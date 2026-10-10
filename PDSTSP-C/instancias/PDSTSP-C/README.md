# Instâncias originais do PDSTSP-C

Conjunto do problema com drones coletivos, de Minh Anh Nguyen e Minh Hoàng Hà:
*The Parallel Drone Scheduling Traveling Salesman Problem with Collective Drones*,
Transportation Science 57(4), 866–888, 2023.
DOI: https://doi.org/10.1287/trsc.2022.1192

Fonte oficial do download:
https://orlab.com.vn/download/instances-for-the-parallel-drone-scheduling-traveling-salesman-problem-with-collective-drones/

Baixado em 2026-09-16. Nome original: `Upload-SuperDrone.zip`.
O arquivo foi preservado como `pacote_original.zip` e extraído sem alterar o conteúdo.
SHA-256: `DF3167668B8CAC108C51ABD84FECBACF98FE4529B3BAFBED85A1CCEB50B84A61`.

- `Upload-SuperDrone/instances/small`: 30 instâncias pequenas.
- `Upload-SuperDrone/instances/large`: 30 instâncias grandes.
- `Upload-SuperDrone/other-params.txt`: parâmetros complementares originais.
- `Upload-SuperDrone/bks.txt`: arquivo de melhores resultados fornecido no pacote.
- `Upload-SuperDrone/power-consumption-calculation`: código original do modelo energético.

Total: 60 instâncias PDSTSP-C. Cada arquivo contém parâmetros dos veículos e uma
tabela `node x y weight`. Todos declaram `TRUCK_SPEED, 30.00` (30 km/h).
A quantidade de nós foi conferida em todos os arquivos, incluindo o depósito 0.

O programa principal foi adaptado para este formato PDSTSP-C; consulte
`../../README.md` para execução e limites do modelo. As instâncias mFSTSP baixadas anteriormente
continuam na pasta separada `../mFSTSP` e não fazem parte deste conjunto.
