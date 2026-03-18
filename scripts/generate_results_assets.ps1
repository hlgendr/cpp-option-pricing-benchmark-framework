param()

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$csvRoot = Join-Path $repoRoot 'code\Projects\OptionPricingBench\output'
$resultsRoot = Join-Path $repoRoot 'results'
$figuresDir = Join-Path $resultsRoot 'figures'
$tablesDir = Join-Path $resultsRoot 'tables'

New-Item -ItemType Directory -Force -Path $figuresDir | Out-Null
New-Item -ItemType Directory -Force -Path $tablesDir | Out-Null
Get-ChildItem -Path $figuresDir -File -ErrorAction SilentlyContinue | Remove-Item -Force
Get-ChildItem -Path $tablesDir -File -ErrorAction SilentlyContinue | Remove-Item -Force

$culture = [System.Globalization.CultureInfo]::InvariantCulture

function Parse-Number([string]$value) {
    if ([string]::IsNullOrWhiteSpace($value)) {
        return [double]::NaN
    }
    return [double]::Parse($value, $culture)
}

function Format-Number([double]$value, [int]$digits = 6) {
    if ([double]::IsNaN($value)) {
        return ''
    }
    return $value.ToString("F$digits", $culture)
}

function Quote-Csv([string]$value) {
    if ($null -eq $value) {
        $value = ''
    }
    return '"' + $value.Replace('"', '""') + '"'
}

function Save-Lines([string]$path, [string[]]$lines) {
    [System.IO.File]::WriteAllLines($path, $lines, [System.Text.UTF8Encoding]::new($false))
}

function Safe-Max([double]$value, [double]$fallback = 1.0) {
    if ([double]::IsNaN($value) -or $value -le 0) {
        return $fallback
    }
    return $value
}

function Measure-Finite([double[]]$values, [string]$mode) {
    $finite = @($values | Where-Object { -not [double]::IsNaN($_) })
    if ($finite.Count -eq 0) {
        return [double]::NaN
    }

    if ($mode -eq 'Average') {
        return ($finite | Measure-Object -Average).Average
    }
    if ($mode -eq 'Minimum') {
        return ($finite | Measure-Object -Minimum).Minimum
    }
    if ($mode -eq 'Maximum') {
        return ($finite | Measure-Object -Maximum).Maximum
    }

    throw "Unsupported measurement mode: $mode"
}

function Write-ChartSvg([string]$path, [string[]]$bodyLines) {
    $header = @(
        '<svg xmlns="http://www.w3.org/2000/svg" width="980" height="560" viewBox="0 0 980 560">',
        '<rect width="100%" height="100%" fill="#fbfaf5"/>',
        '<style>',
        'text { font-family: Georgia, "Times New Roman", serif; fill: #1f1f1f; }',
        '.title { font-size: 26px; font-weight: 700; }',
        '.subtitle { font-size: 14px; fill: #4f4a40; }',
        '.axis { stroke: #4f4a40; stroke-width: 1.2; }',
        '.grid { stroke: #ddd6c8; stroke-width: 1; }',
        '.label { font-size: 13px; }',
        '.tick { font-size: 12px; fill: #615a50; }',
        '.legend { font-size: 13px; }',
        '.annotation { font-size: 12px; fill: #4f4a40; }',
        '</style>'
    )
    Save-Lines $path ($header + $bodyLines + '</svg>')
}

function Build-LineChartBody(
    [string]$title,
    [string]$subtitle,
    [string]$yCaption,
    [string[]]$xLabels,
    [string[]]$seriesNames,
    [hashtable]$seriesMap,
    [hashtable]$colors,
    [double]$maxY
) {
    $effectiveMax = Safe-Max $maxY
    $body = @(
        ('<text x="60" y="52" class="title">{0}</text>' -f $title),
        ('<text x="60" y="78" class="subtitle">{0}</text>' -f $subtitle),
        '<line x1="90" y1="470" x2="900" y2="470" class="axis"/>',
        '<line x1="90" y1="150" x2="90" y2="470" class="axis"/>',
        ('<text x="90" y="130" class="label">{0}</text>' -f $yCaption)
    )

    $pointsX = @()
    $stepX = if ($xLabels.Count -gt 1) { [double]730 / ($xLabels.Count - 1) } else { 0.0 }
    for ($i = 0; $i -lt $xLabels.Count; $i++) {
        $x = [int][math]::Round(130 + ($i * $stepX))
        $pointsX += $x
        $body += ('<line x1="{0}" y1="170" x2="{0}" y2="470" class="grid"/>' -f $x)
        $body += ('<text x="{0}" y="492" class="tick">{1}</text>' -f ($x - 12), $xLabels[$i])
    }

    $legendX = 670
    $legendY = 110
    for ($seriesIndex = 0; $seriesIndex -lt $seriesNames.Count; $seriesIndex++) {
        $seriesName = $seriesNames[$seriesIndex]
        $color = $colors[$seriesName]
        $values = $seriesMap[$seriesName]
        $points = @()

        for ($i = 0; $i -lt $values.Count; $i++) {
            $y = [int][math]::Round(470 - (300 * $values[$i] / $effectiveMax))
            $points += ('{0},{1}' -f $pointsX[$i], $y)
        }

        $body += ('<polyline fill="none" stroke="{0}" stroke-width="3" points="{1}"/>' -f $color, ($points -join ' '))

        for ($i = 0; $i -lt $values.Count; $i++) {
            $y = [int][math]::Round(470 - (300 * $values[$i] / $effectiveMax))
            $body += ('<circle cx="{0}" cy="{1}" r="4" fill="{2}"/>' -f $pointsX[$i], $y, $color)
        }

        $legendRowY = $legendY + ($seriesIndex * 22)
        $body += ('<rect x="{0}" y="{1}" width="14" height="14" fill="{2}"/>' -f $legendX, ($legendRowY - 10), $color)
        $body += ('<text x="{0}" y="{1}" class="legend">{2}</text>' -f ($legendX + 22), $legendRowY, $seriesName)
    }

    return $body
}

$benchmarkRows = Import-Csv (Join-Path $csvRoot 'benchmark_results.csv')
$binomialRows = Import-Csv (Join-Path $csvRoot 'binomial_convergence.csv')
$mcRows = Import-Csv (Join-Path $csvRoot 'mc_variance_study.csv')
$fdmRows = Import-Csv (Join-Path $csvRoot 'fdm_grid_study.csv')
$validationCsvPath = Join-Path $csvRoot 'validation_summary.csv'
$validationMarkdownPath = Join-Path $csvRoot 'validation_summary.md'
$validationRows = if (Test-Path $validationCsvPath) { Import-Csv $validationCsvPath } else { @() }

$methodSummary = foreach ($group in ($benchmarkRows | Group-Object method)) {
    $rows = $group.Group
    $runtimeValues = @($rows | ForEach-Object { Parse-Number $_.runtime_ms })
    $absValues = @($rows | ForEach-Object { Parse-Number $_.absolute_error })
    $relValues = @($rows | ForEach-Object { Parse-Number $_.relative_error })
    $stdErrValues = @($rows | ForEach-Object { Parse-Number $_.standard_error })

    [pscustomobject]@{
        method = $group.Name
        avg_runtime_ms = Measure-Finite $runtimeValues 'Average'
        avg_abs_error = Measure-Finite $absValues 'Average'
        avg_rel_error = Measure-Finite $relValues 'Average'
        avg_standard_error = Measure-Finite $stdErrValues 'Average'
        best_abs_error = Measure-Finite $absValues 'Minimum'
        worst_abs_error = Measure-Finite $absValues 'Maximum'
    }
}

$bestByCase = foreach ($group in ($benchmarkRows | Where-Object { $_.method -ne 'Exact' } | Group-Object case_name)) {
    $best = $group.Group | Sort-Object { Parse-Number $_.absolute_error } | Select-Object -First 1
    [pscustomobject]@{
        case_name = $group.Name
        best_method = $best.method
        absolute_error = Parse-Number $best.absolute_error
        relative_error = Parse-Number $best.relative_error
        runtime_ms = Parse-Number $best.runtime_ms
    }
}

$studyHighlights = @()
foreach ($group in ($binomialRows | Group-Object case_name)) {
    $ordered = @($group.Group | Sort-Object { [int]$_.steps })
    $studyHighlights += [pscustomobject]@{
        study = 'binomial_convergence'
        case_name = $group.Name
        variant = ''
        start_config = 'steps=' + $ordered[0].steps
        end_config = 'steps=' + $ordered[-1].steps
        start_error = Parse-Number $ordered[0].absolute_error
        end_error = Parse-Number $ordered[-1].absolute_error
        start_std_error = [double]::NaN
        end_std_error = [double]::NaN
        end_runtime_ms = Parse-Number $ordered[-1].runtime_ms
    }
}
foreach ($group in ($mcRows | Group-Object case_name, variant)) {
    $ordered = @($group.Group | Sort-Object { [long]$_.simulations })
    $studyHighlights += [pscustomobject]@{
        study = 'mc_variance'
        case_name = $ordered[0].case_name
        variant = $ordered[0].variant
        start_config = 'sims=' + $ordered[0].simulations
        end_config = 'sims=' + $ordered[-1].simulations + ';seed=' + $ordered[-1].seed
        start_error = Parse-Number $ordered[0].absolute_error
        end_error = Parse-Number $ordered[-1].absolute_error
        start_std_error = Parse-Number $ordered[0].standard_error
        end_std_error = Parse-Number $ordered[-1].standard_error
        end_runtime_ms = Parse-Number $ordered[-1].runtime_ms
    }
}
foreach ($group in ($fdmRows | Group-Object case_name)) {
    $ordered = @($group.Group | Sort-Object { [int]$_.space_steps })
    $studyHighlights += [pscustomobject]@{
        study = 'fdm_grid_study'
        case_name = $group.Name
        variant = ''
        start_config = 'J=' + $ordered[0].space_steps + ';N=' + $ordered[0].time_steps
        end_config = 'J=' + $ordered[-1].space_steps + ';N=' + $ordered[-1].time_steps
        start_error = Parse-Number $ordered[0].absolute_error
        end_error = Parse-Number $ordered[-1].absolute_error
        start_std_error = [double]::NaN
        end_std_error = [double]::NaN
        end_runtime_ms = Parse-Number $ordered[-1].runtime_ms
    }
}

$mcVariantSummary = foreach ($group in ($mcRows | Group-Object variant)) {
    $rows = $group.Group
    $absValues = @($rows | ForEach-Object { Parse-Number $_.absolute_error })
    $stdValues = @($rows | ForEach-Object { Parse-Number $_.standard_error })
    $ciWidths = @($rows | ForEach-Object {
        $lower = Parse-Number $_.ci95_lower
        $upper = Parse-Number $_.ci95_upper
        if ([double]::IsNaN($lower) -or [double]::IsNaN($upper)) {
            [double]::NaN
        } else {
            $upper - $lower
        }
    })

    [pscustomobject]@{
        variant = $group.Name
        avg_abs_error = Measure-Finite $absValues 'Average'
        avg_standard_error = Measure-Finite $stdValues 'Average'
        avg_ci_width = Measure-Finite $ciWidths 'Average'
        best_abs_error = Measure-Finite $absValues 'Minimum'
    }
}

$validationSummaryStats = if ($validationRows.Count -gt 0) {
    [pscustomobject]@{
        total_checks = $validationRows.Count
        passed_checks = @($validationRows | Where-Object { $_.status -eq 'PASS' }).Count
        failed_checks = @($validationRows | Where-Object { $_.status -ne 'PASS' }).Count
    }
}

$caseSnapshotLines = @('case_name,method,option_type,price,runtime_ms,absolute_error,relative_error,standard_error,ci95_lower,ci95_upper,sample_count,path_count')
$caseSnapshotLines += $benchmarkRows | ForEach-Object {
    '{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11}' -f (Quote-Csv $_.case_name), (Quote-Csv $_.method), (Quote-Csv $_.option_type), $_.price, $_.runtime_ms, $_.absolute_error, $_.relative_error, $_.standard_error, $_.ci95_lower, $_.ci95_upper, $_.sample_count, $_.path_count
}
Save-Lines (Join-Path $tablesDir 'benchmark_case_snapshot.csv') $caseSnapshotLines

$methodSummaryLines = @('method,avg_runtime_ms,avg_abs_error,avg_rel_error,avg_standard_error,best_abs_error,worst_abs_error')
$methodSummaryLines += $methodSummary | ForEach-Object {
    '{0},{1},{2},{3},{4},{5},{6}' -f (Quote-Csv $_.method), (Format-Number $_.avg_runtime_ms), (Format-Number $_.avg_abs_error), (Format-Number $_.avg_rel_error), (Format-Number $_.avg_standard_error), (Format-Number $_.best_abs_error), (Format-Number $_.worst_abs_error)
}
Save-Lines (Join-Path $tablesDir 'benchmark_method_summary.csv') $methodSummaryLines

$bestByCaseLines = @('case_name,best_method,absolute_error,relative_error,runtime_ms')
$bestByCaseLines += $bestByCase | ForEach-Object {
    '{0},{1},{2},{3},{4}' -f (Quote-Csv $_.case_name), (Quote-Csv $_.best_method), (Format-Number $_.absolute_error), (Format-Number $_.relative_error), (Format-Number $_.runtime_ms)
}
Save-Lines (Join-Path $tablesDir 'best_method_by_case.csv') $bestByCaseLines

$studyLines = @('study,case_name,variant,start_config,end_config,start_error,end_error,start_std_error,end_std_error,end_runtime_ms')
$studyLines += $studyHighlights | ForEach-Object {
    '{0},{1},{2},{3},{4},{5},{6},{7},{8},{9}' -f (Quote-Csv $_.study), (Quote-Csv $_.case_name), (Quote-Csv $_.variant), (Quote-Csv $_.start_config), (Quote-Csv $_.end_config), (Format-Number $_.start_error), (Format-Number $_.end_error), (Format-Number $_.start_std_error), (Format-Number $_.end_std_error), (Format-Number $_.end_runtime_ms)
}
Save-Lines (Join-Path $tablesDir 'study_highlights.csv') $studyLines

$mcVariantLines = @('variant,avg_abs_error,avg_standard_error,avg_ci_width,best_abs_error')
$mcVariantLines += $mcVariantSummary | ForEach-Object {
    '{0},{1},{2},{3},{4}' -f (Quote-Csv $_.variant), (Format-Number $_.avg_abs_error), (Format-Number $_.avg_standard_error), (Format-Number $_.avg_ci_width), (Format-Number $_.best_abs_error)
}
Save-Lines (Join-Path $tablesDir 'mc_variant_summary.csv') $mcVariantLines

if ($validationRows.Count -gt 0) {
    Copy-Item -Path $validationCsvPath -Destination (Join-Path $tablesDir 'validation_summary.csv') -Force
    Copy-Item -Path $validationMarkdownPath -Destination (Join-Path $tablesDir 'validation_summary.md') -Force

    $validationStatLines = @('total_checks,passed_checks,failed_checks')
    $validationStatLines += ('{0},{1},{2}' -f $validationSummaryStats.total_checks, $validationSummaryStats.passed_checks, $validationSummaryStats.failed_checks)
    Save-Lines (Join-Path $tablesDir 'validation_overview.csv') $validationStatLines
}

$methods = @('Exact', 'Binomial', 'MC', 'FDM')
$methodColors = @{
    Exact = '#2f6b66'
    Binomial = '#d97757'
    MC = '#f1b24a'
    FDM = '#5b7db8'
}
$maxRel = Safe-Max (Measure-Finite @($methodSummary | ForEach-Object { $_.avg_rel_error }) 'Maximum')
$maxRuntime = Safe-Max (Measure-Finite @($methodSummary | ForEach-Object { $_.avg_runtime_ms }) 'Maximum')
$benchmarkBody = @(
    '<text x="60" y="52" class="title">Main Benchmark Summary</text>',
    '<text x="60" y="78" class="subtitle">Average relative error and average runtime across 6 European vanilla benchmark cases</text>',
    '<line x1="80" y1="470" x2="440" y2="470" class="axis"/>',
    '<line x1="80" y1="180" x2="80" y2="470" class="axis"/>',
    '<text x="80" y="160" class="label">Average Relative Error</text>',
    '<line x1="540" y1="470" x2="900" y2="470" class="axis"/>',
    '<line x1="540" y1="180" x2="540" y2="470" class="axis"/>',
    '<text x="540" y="160" class="label">Average Runtime (ms)</text>'
)
$barWidth = 58
$leftStart = 110
$rightStart = 570
for ($i = 0; $i -lt $methods.Count; $i++) {
    $method = $methods[$i]
    $row = $methodSummary | Where-Object { $_.method -eq $method }
    $color = $methodColors[$method]
    $rel = [double]$row.avg_rel_error
    $runtime = [double]$row.avg_runtime_ms
    $relHeight = 250 * $rel / $maxRel
    $runtimeHeight = 250 * $runtime / $maxRuntime
    $x1 = $leftStart + ($i * 78)
    $y1 = [int][math]::Round(470 - $relHeight)
    $x2 = $rightStart + ($i * 78)
    $y2 = [int][math]::Round(470 - $runtimeHeight)

    $benchmarkBody += ('<rect x="{0}" y="{1}" width="{2}" height="{3}" fill="{4}" rx="4"/>' -f $x1, $y1, $barWidth, [int][math]::Round($relHeight), $color)
    $benchmarkBody += ('<text x="{0}" y="490" class="tick">{1}</text>' -f $x1, $method)
    $benchmarkBody += ('<text x="{0}" y="{1}" class="annotation">{2}</text>' -f $x1, ([math]::Max($y1 - 8, 170)), (Format-Number $rel))

    $benchmarkBody += ('<rect x="{0}" y="{1}" width="{2}" height="{3}" fill="{4}" rx="4"/>' -f $x2, $y2, $barWidth, [int][math]::Round($runtimeHeight), $color)
    $benchmarkBody += ('<text x="{0}" y="490" class="tick">{1}</text>' -f $x2, $method)
    $benchmarkBody += ('<text x="{0}" y="{1}" class="annotation">{2}</text>' -f $x2, ([math]::Max($y2 - 8, 170)), (Format-Number $runtime 3))
}
Write-ChartSvg (Join-Path $figuresDir 'benchmark_tradeoff.svg') $benchmarkBody

$binCases = @('atm_call_1y', 'atm_put_1y')
$binColors = @{ atm_call_1y = '#d97757'; atm_put_1y = '#5b7db8' }
$binSteps = @('25', '50', '100', '200', '400', '800', '1600')
$binSeries = @{}
foreach ($caseName in $binCases) {
    $binSeries[$caseName] = @($binomialRows | Where-Object { $_.case_name -eq $caseName } | Sort-Object { [int]$_.steps } | ForEach-Object { Parse-Number $_.absolute_error })
}
$binMax = Safe-Max (Measure-Finite @($binSeries.Values | ForEach-Object { $_ }) 'Maximum')
Write-ChartSvg (Join-Path $figuresDir 'binomial_convergence.svg') (Build-LineChartBody 'Binomial Convergence Study' 'Absolute error versus CRR step count on two ATM cases' 'Absolute Error' $binSteps $binCases $binSeries $binColors $binMax)

$mcVariants = @('Crude', 'Antithetic', 'ControlVariate')
$mcColors = @{ Crude = '#d97757'; Antithetic = '#5b7db8'; ControlVariate = '#2f6b66' }
$mcLabels = @('1k', '5k', '10k', '30k', '60k')
$mcSeries = @{}
foreach ($variant in $mcVariants) {
    $variantPoints = @()
    foreach ($simulationCount in @(1000, 5000, 10000, 30000, 60000)) {
        $subset = @($mcRows | Where-Object { $_.variant -eq $variant -and [long]$_.simulations -eq $simulationCount })
        $variantPoints += Measure-Finite @($subset | ForEach-Object { Parse-Number $_.absolute_error }) 'Average'
    }
    $mcSeries[$variant] = $variantPoints
}
$mcMax = Safe-Max (Measure-Finite @($mcSeries.Values | ForEach-Object { $_ }) 'Maximum')
Write-ChartSvg (Join-Path $figuresDir 'mc_variance_comparison.svg') (Build-LineChartBody 'MC Variance Reduction Comparison' 'Average absolute error across ATM call/put cases by simulation budget' 'Average Absolute Error' $mcLabels $mcVariants $mcSeries $mcColors $mcMax)

$fdmBody = @(
    '<text x="60" y="52" class="title">FDM Grid / Runtime Trade-off</text>',
    '<text x="60" y="78" class="subtitle">Explicit Euler grid refinement on atm_call_1y</text>',
    '<line x1="100" y1="470" x2="900" y2="470" class="axis"/>',
    '<line x1="100" y1="150" x2="100" y2="470" class="axis"/>',
    '<text x="100" y="130" class="label">Absolute Error</text>',
    '<text x="720" y="500" class="label">Runtime (ms)</text>'
)
$fdmRuntimeMax = Safe-Max (Measure-Finite @($fdmRows | ForEach-Object { Parse-Number $_.runtime_ms }) 'Maximum')
$fdmErrorMax = Safe-Max (Measure-Finite @($fdmRows | ForEach-Object { Parse-Number $_.absolute_error }) 'Maximum')
foreach ($row in $fdmRows) {
    $runtime = Parse-Number $row.runtime_ms
    $errorValue = Parse-Number $row.absolute_error
    $x = [int][math]::Round(140 + (720 * $runtime / $fdmRuntimeMax))
    $y = [int][math]::Round(470 - (280 * $errorValue / $fdmErrorMax))
    $label = 'J=' + $row.space_steps + ', N=' + $row.time_steps
    $fdmBody += ('<circle cx="{0}" cy="{1}" r="8" fill="#5b7db8" opacity="0.88"/>' -f $x, $y)
    $fdmBody += ('<text x="{0}" y="{1}" class="annotation">{2}</text>' -f ($x + 12), ($y - 6), $label)
}
Write-ChartSvg (Join-Path $figuresDir 'fdm_grid_tradeoff.svg') $fdmBody
