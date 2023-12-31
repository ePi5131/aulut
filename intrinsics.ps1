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
    $ret = ($b70 -shr 16) # AVX-512F
    $ret = $ret -band ($b70 -shr 17) # AVX-512DQ
    $ret = $ret -band ($b70 -shr 30) # AVX-512BW
    $ret = $ret -band ($b70 -shr 31) # AVX-512VL
    $ret -band 1

    # [Avx512F]::IsSupported -and [Avx512VL]::IsSupported -and [Avx512DQ]::IsSupported -and [Avx512BW]::IsSupported # .NET 8
}

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
