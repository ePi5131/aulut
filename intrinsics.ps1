using namespace System.Runtime.Intrinsics.X86

function Invoke-AulutCpuId {
    param (
        [Parameter(Mandatory, Position=0)]
        [int] $Eax,
        [Parameter(Position=1)]
        [int] $Ecx=0
    )

    $cpuIdResult = [X86Base]::CpuId($Eax, $Ecx)
    $cpuIdResult.Item1
    $cpuIdResult.Item2
    $cpuIdResult.Item3
    $cpuIdResult.Item4
}

function Test-AulutSupportsAvx512 {
    $a70,$b70,$c70,$d70 = Invoke-AulutCpuId 7 0
    ($b70 -shr 16) -band 1
}

# if([Avx512]::IsSupported) { # .NET 8
if (Test-AulutSupportsAvx512) {
    "AVX512"
}
elseif ([Avx2]::IsSupported) {
    "AVX2"
}
elseif ([Sse41]::IsSupported) {
    "SSE4"
}
else {
    "SSE2"
}

Write-Host "Press any key to continue..." -NoNewline
$null = [System.Console]::ReadKey($true)
Write-Host
