# C++ Option Pricing Benchmark Framework - Project Map

This file explains how the current benchmark framework is organized and which parts are core to the showcased project.

## Main Source Tree

The benchmark source lives under:

- [code/](code/)

## Benchmark Framework Layers

### `code/ResumeCore/Common/`

Link:

- [ResumeCore/Common/](code/ResumeCore/Common/)

Purpose:

- define the shared European vanilla option input type
- define method configuration for Binomial, MC, and FDM
- define the common result payload used across methods and studies

Key files:

- `VanillaOptionSpec.hpp`
- `MethodConfig.hpp`
- `PricingResult.hpp`
- `BenchmarkCase.hpp`
- `BenchmarkRow.hpp`

### `code/ResumeCore/Adapters/`

Link:

- [ResumeCore/Adapters/](code/ResumeCore/Adapters/)

Purpose:

- wrap each legacy pricing engine behind a shared benchmark-facing interface
- preserve core numerical logic while normalizing outputs
- keep method-specific quirks out of the runner layer

Key files:

- `ExactPricer.*`
- `BinomialPricer.*`
- `MCPricer.*`
- `FDMPricer.*`

### `code/ResumeCore/Experiments/`

Link:

- [ResumeCore/Experiments/](code/ResumeCore/Experiments/)

Purpose:

- define the standard benchmark case pack
- define binomial, MC variance, and FDM study configurations
- run the core experiment workflows
- write structured CSV outputs

Key files:

- `ExperimentPack.*`
- `ExperimentRunner.*`

### `code/ResumeCore/Validation/`

Link:

- [ResumeCore/Validation/](code/ResumeCore/Validation/)

Purpose:

- hold the lightweight validation harness
- check exact-price anchors, parity, benchmark sanity, tolerances, and MC V2 sanity
- write CSV and Markdown validation summaries

Key files:

- `ValidationHarness.*`

### `code/ResumeCore/App/`

Link:

- [ResumeCore/App/](code/ResumeCore/App/)

Purpose:

- provide the CLI entrypoint
- expose `benchmark`, `mc_variance`, `validation`, and `all` run modes
- coordinate which outputs are refreshed in each mode

Key file:

- `main.cpp`

## Build / Execution Layer

### `code/Projects/OptionPricingBench/`

Link:

- [Projects/OptionPricingBench/](code/Projects/OptionPricingBench/)

Purpose:

- hold the Visual Studio solution and project
- build the benchmark executable
- store raw experiment outputs in `output/`

Important files:

- `OptionPricingBench.sln`
- `OptionPricingBench/OptionPricingBench.vcxproj`
- `output/*.csv`
- `output/validation_summary.md`

## Archived Standalone Solutions

### `code/archive_projects/`

Link:

- [archive_projects/](code/archive_projects/)

Purpose:

- preserve the original standalone Visual Studio solutions as archived references
- keep them available for comparison, backtracking, or historical review
- remove them from the main repository foreground so the benchmark framework remains the clear primary entrypoint

## Underlying Numerical Engines

These directories still contain the underlying implementations reused by the adapter layer.

### `code/VI.3/`

- closed-form Black-Scholes-style pricing
- current benchmark uses `PlainOption/EuropeanOption.*`

### `code/VI.4/`

- Monte Carlo option data and payoff plumbing
- current benchmark uses `OptionData.hpp`

### `code/VI.5/`

- PDE / finite-difference infrastructure
- current benchmark uses `ParabolicPDE.hpp`, `Mesher.hpp`, `FDM.hpp`, and `FDMDirector.hpp`

### `code/VI.6/`

- binomial lattice machinery
- current benchmark uses `Option.hpp`, `BinomialMethod.*`, `BinomialLatticeStrategy.*`, and lattice helpers

### `code/UtilitiesDJD/`

- shared utilities
- current benchmark mainly reuses the RNG stack under `RNG/`

## Extensions

### `code/extensions/american/`

Reserved space for American / perpetual option material that still has technical value but is not part of the current benchmark headline.

### `code/extensions/cir/`

Reserved space for CIR-related material and future side exploration.

### `code/extensions/excel_demo/`

Reserved space for Excel-based demo material. Preserved, but intentionally outside the main GitHub story.

## Archive

### `code/archive/`

Stores historical supporting material that is not part of the main benchmark narrative.

Current archived material includes:

- `archive/reports/`

## Result Packaging Layer

### `results/figures/`

GitHub-facing SVG figures derived from the latest raw CSV outputs.

### `results/tables/`

Condensed CSV and Markdown summaries used to support the README and interview narrative.

### `scripts/generate_results_assets.ps1`

Reads the latest raw outputs and regenerates the packaged `results/` assets.

## Practical Narrative

For GitHub, resume, and interview purposes, the main story should begin with:

1. `OptionPricingBench.sln`
2. `ResumeCore/`
3. `results/`

Historical reference entrypoints remain available, but they are no longer the recommended way to enter the project.
