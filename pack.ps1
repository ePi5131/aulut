function GetFilePaths([string] $cfg) {
    return @(
        ".\LICENSE"
        ".\credits.md"
        ".\aulut\aulut.anm"
        ".\aulut\aulut_save.anm"
        ".\aulut\aulut_std.obj"
        ".\Output\Release({0})\aulut.dll" -f $cfg
    )
}

$configurations = @("SSE2","SSE4","AVX2","AVX512")

if (-not (Test-Path -LiteralPath ".\Pack")) {
    New-Item -Path Pack -ItemType Directory
}

for ($i = 0; $i -lt $configurations.Count; $i++) {
    Compress-Archive -Path (GetFilePaths $configurations[$i]) -DestinationPath (".\Pack\aulut_{0}.zip" -f $configurations[$i]) -Force
}