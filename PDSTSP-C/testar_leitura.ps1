$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    & gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror main.c cv.c -lm -o pdstspc.exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao' }
    & gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror teste_leitura.c cv.c -lm -o teste_leitura.exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar testes' }
    $base = 'instancias/PDSTSP-C/Upload-SuperDrone'
    $arquivos = @(Get-ChildItem "$base/instances" -Recurse -Filter '*.txt')
    if ($arquivos.Count -ne 60) { throw 'Esperadas 60 instancias PDSTSP-C' }
    foreach ($arquivo in $arquivos) {
        & ./teste_leitura.exe $arquivo.FullName
        if ($LASTEXITCODE -ne 0) { throw "Falha: $($arquivo.Name)" }
    }
    $temp = Join-Path ([IO.Path]::GetTempPath()) ('pdstspc-test-' + [guid]::NewGuid())
    New-Item -ItemType Directory -Path $temp | Out-Null
    $original = Get-Content "$base/instances/small/15-r-e.txt" -Raw
    $params = (Resolve-Path "$base/other-params.txt").Path
    $invalido = Join-Path $temp 'invalido.txt'
    $casos = @(
        ($original -replace 'TRUCK_SPEED, 30.00','TRUCK_SPEED, 0'),
        ($original -replace 'NUM_DRONES, 3','NUM_DRONES, 1.5'),
        ($original -replace 'VOLTAGE, 12.8','VOLTAGE, nan'),
        ($original -replace 'NUM_DRONES, 3',"NUM_DRONES, 3`nNUM_DRONES, 3"),
        ($original -replace '(?m)^\s*1\s+7.00','2 7.00'),
        ($original -replace '0.54','-0.54'),
        ($original -replace '(?m)^BATTERY_CAPACITY.*\r?\n',''),
        ($original -replace 'BATTERY_WEIGHT, 1.55','BATTERY_WEIGHT,   ')
    )
    foreach ($caso in $casos) {
        [IO.File]::WriteAllText($invalido,$caso)
        $null = & ./pdstspc.exe $invalido --parametros $params --preprocessar 2>&1
        if ($LASTEXITCODE -eq 0) { throw 'Entrada invalida foi aceita' }
    }
    [IO.File]::WriteAllText($invalido,$original)
    $null = & ./pdstspc.exe $invalido --preprocessar 2>&1
    if ($LASTEXITCODE -eq 0) { throw 'Parametros ausentes foram aceitos' }
    $null = & ./pdstspc.exe $invalido --parametros $params --preprocessar
    if ($LASTEXITCODE -ne 0) { throw 'Caminho explicito de parametros falhou' }
    Copy-Item -LiteralPath './pdstspc.exe' -Destination './pdstspd.exe' -Force
    Write-Output 'OK: 60 instancias, matrizes, energia, heuristica, sincronizacao e entradas invalidas.'
} finally { Pop-Location }
