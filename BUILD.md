# C++ Option Pricing Benchmark Framework - Build Guide

## Recommended Environment

- **IDE**: Visual Studio 2022
- **Platform**: `Win32`
- **Configurations**: `Debug`, `Release`
- **Toolset**: `v143`
- **Language standard**: C++17

For benchmark numbers and packaged results, use **`Release | Win32`**.

## Main Solution

Open:

- [code/Projects/OptionPricingBench/OptionPricingBench.sln](code/Projects/OptionPricingBench/OptionPricingBench.sln)

This is the main benchmark solution. Older standalone solutions are preserved under [code/archive_projects/](code/archive_projects/) as archived references, but they are no longer the recommended entrypoint.

## Dependencies

### Required for the benchmark line

- Visual Studio 2022 C++ workload
- MSVC `v143`
- Boost headers

The main project is configured to read Boost from:

- `$(BOOST_ROOT)`

So the expected setup is:

1. set `BOOST_ROOT` to your Boost installation, or
2. configure the include path in Visual Studio before building

### Not required for the current benchmark headline

- Excel / Office COM setup
- American extension material
- CIR extension material

## Build In Visual Studio

1. Open `OptionPricingBench.sln`.
2. Select `Release | Win32`.
3. Build `OptionPricingBench`.
4. Run one of the supported modes:

```text
OptionPricingBench.exe benchmark
OptionPricingBench.exe mc_variance
OptionPricingBench.exe validation
OptionPricingBench.exe all
```

Recommended refresh flow:

```text
OptionPricingBench.exe all
```

## Raw Output Location

The executable writes raw outputs to:

- [code/Projects/OptionPricingBench/output/](code/Projects/OptionPricingBench/output/)

Current raw outputs:

- `benchmark_results.csv`
- `binomial_convergence.csv`
- `fdm_grid_study.csv`
- `mc_variance_study.csv`
- `validation_summary.csv`
- `validation_summary.md`

## Generate GitHub-Facing Figures And Tables

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\generate_results_assets.ps1
```

This script reads the raw CSV outputs and refreshes:

- `results/figures/*.svg`
- `results/tables/*.csv`
- `results/tables/validation_summary.md`

## Known Notes

- The benchmark framework is intentionally scoped to **European vanilla options**.
- The main benchmark still compares exactly **four method families**: Exact, Binomial, MC, and FDM.
- The benchmark's default MC line now uses **Control Variate** as the primary variant.
- MC uses a fixed seed (`42`) for reproducible study outputs.
- The current FDM path is still the **explicit Euler** implementation; this repo does not claim a full PDE rewrite.
- You may still see Boost warning `C4819` depending on the local Boost header encoding. It does not currently block build or execution.

## Suggested Release Workflow

1. Build `Release | Win32`.
2. Run `OptionPricingBench.exe all`.
3. Run `scripts/generate_results_assets.ps1`.
4. Review:
   - [results/figures/](results/figures/)
   - [results/tables/](results/tables/)
   - [RESULTS_SUMMARY.md](RESULTS_SUMMARY.md)

