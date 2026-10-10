$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $base = 'instancias/PDSTSP-C/Upload-SuperDrone'
    $temp = Join-Path ([IO.Path]::GetTempPath()) ('pdstspc-modelo-' + [guid]::NewGuid())
    New-Item -ItemType Directory -Path $temp | Out-Null
    Copy-Item "$base/power-consumption-calculation/energymodel.h" $temp
    $fonte = Get-Content "$base/power-consumption-calculation/energymodel.cpp" -Raw
    # Retira somente o metodo de plotagem/dependencia ausente; nao altera formulas.
    $inicio = $fonte.IndexOf('void EnergyModel::show_discharge_curve')
    $fim = $fonte.IndexOf('EnergyModel::EnergyModel')
    if ($inicio -lt 0 -or $fim -lt $inicio) { throw 'Estrutura do modelo original mudou' }
    $fonte = $fonte.Substring(0,$inicio) + $fonte.Substring($fim)
    $fonte = $fonte.Replace('#include "gnuplot-iostream.h"', '#include <cassert>')
    [IO.File]::WriteAllText((Join-Path $temp 'energymodel.cpp'),$fonte)
    $obj = Join-Path $temp 'cv.o'
    $exe = Join-Path $temp 'teste_modelo.exe'
    & gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -c cv.c -o $obj
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao C' }
    & g++ -std=c++11 -O2 -I $temp -I . teste_modelo.cpp (Join-Path $temp 'energymodel.cpp') $obj -o $exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao do modelo de referencia' }
    & $exe "$base/instances/small/15-r-e.txt"
    if ($LASTEXITCODE -ne 0) { throw 'Divergencia no modelo energetico (pequena)' }
    & $exe "$base/instances/large/198-c-c.txt"
    if ($LASTEXITCODE -ne 0) { throw 'Divergencia no modelo energetico (grande)' }
} finally { Pop-Location }
