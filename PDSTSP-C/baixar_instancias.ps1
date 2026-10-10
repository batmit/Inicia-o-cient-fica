$ErrorActionPreference = 'Stop'
$destino = Join-Path $PSScriptRoot 'instancias/PDSTSP-C'
$url = 'https://orlab.com.vn/download/instances-for-the-parallel-drone-scheduling-traveling-salesman-problem-with-collective-drones/?wpdmdl=9555'
$esperado = 'DF3167668B8CAC108C51ABD84FECBACF98FE4529B3BAFBED85A1CCEB50B84A61'
New-Item -ItemType Directory -Force -Path $destino | Out-Null
$temporario = Join-Path ([IO.Path]::GetTempPath()) ('pdstspc-' + [guid]::NewGuid() + '.zip')
Invoke-WebRequest $url -OutFile $temporario -TimeoutSec 90
if ((Get-FileHash $temporario -Algorithm SHA256).Hash -ne $esperado) {
    throw 'O pacote remoto mudou. Confira a fonte antes de substituir os dados locais.'
}
Add-Type -AssemblyName System.IO.Compression.FileSystem
$arquivo = [IO.Compression.ZipFile]::OpenRead($temporario)
try {
    $raiz = [IO.Path]::GetFullPath($destino) + [IO.Path]::DirectorySeparatorChar
    foreach ($entrada in $arquivo.Entries) {
        $alvo = [IO.Path]::GetFullPath((Join-Path $destino $entrada.FullName))
        if (-not $alvo.StartsWith($raiz,[StringComparison]::OrdinalIgnoreCase)) {
            throw 'Caminho fora da pasta de destino no ZIP'
        }
    }
} finally { $arquivo.Dispose() }
Copy-Item -LiteralPath $temporario -Destination (Join-Path $destino 'pacote_original.zip') -Force
Expand-Archive -LiteralPath $temporario -DestinationPath $destino -Force
Write-Output 'Baixado e verificado: pacote oficial PDSTSP-C (60 instancias).'
