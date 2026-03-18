# C++ Option Pricing Benchmark Framework - Results Summary

This summary is based on the latest **Release** outputs in:

- [code/Projects/OptionPricingBench/output/](code/Projects/OptionPricingBench/output/)

Packaged tables and figures were regenerated into:

- [results/figures/](results/figures/)
- [results/tables/](results/tables/)

## 1. Main Benchmark Trade-Off

Reference assets:

- [benchmark_tradeoff.svg](results/figures/benchmark_tradeoff.svg)
- [benchmark_method_summary.csv](results/tables/benchmark_method_summary.csv)
- [best_method_by_case.csv](results/tables/best_method_by_case.csv)

### Method-level summary

| Method | Avg runtime (ms) | Avg abs error | Avg rel error | Comment |
| --- | ---: | ---: | ---: | --- |
| Exact | 0.111021 | 0.000000 | 0.000000 | analytical baseline |
| Binomial | 17.438403 | 0.003377 | 0.000364 | strongest overall speed/accuracy balance |
| MC (Control Variate baseline) | 156.083264 | 0.005907 | 0.000602 | much stronger than the earlier crude-only MC line |
| FDM | 1119.516715 | 0.015487 | 0.001667 | accurate but still runtime-heavy in the explicit Euler setup |

### Main takeaways

- **Binomial remains the best all-around approximation method in the current benchmark pack.**
- **MC is now much more credible as a quant project component** because it reports uncertainty and uses variance reduction instead of a crude-simulation-only workflow.
- **FDM remains technically useful and accurate on several cases**, but the current explicit Euler implementation is still the slowest method in the benchmark.

## 2. Monte Carlo V2 Upgrade

Reference assets:

- [mc_variance_comparison.svg](results/figures/mc_variance_comparison.svg)
- [mc_variant_summary.csv](results/tables/mc_variant_summary.csv)
- [mc_variance_study.csv](code/Projects/OptionPricingBench/output/mc_variance_study.csv)

### Variant summary

| Variant | Avg abs error | Avg standard error | Avg 95% CI width | Best abs error |
| --- | ---: | ---: | ---: | ---: |
| Crude | 0.091678 | 0.154977 | 0.607512 | 0.000290 |
| Antithetic | 0.062828 | 0.079598 | 0.312025 | 0.001127 |
| Control Variate | 0.002914 | 0.002606 | 0.010214 | 0.000160 |

### What this means

- **Antithetic variates help**, but the improvement is moderate and still somewhat case-dependent.
- **Control variate dominates the other MC variants** in this setup.
- The control-variate line is not just slightly better; it reduces both absolute error and uncertainty by an order of magnitude relative to crude MC in the current study pack.

### Strong evidence from the current runs

For `atm_call_1y`:

- crude @ `60000` sims: abs error `0.035784`, standard error `0.060163`
- antithetic @ `60000` sims: abs error `0.017599`, standard error `0.030112`
- control variate @ `60000` sims: abs error `0.002768`, standard error `0.000986`

For `atm_put_1y`:

- crude @ `60000` sims: abs error `0.043100`, standard error `0.035529`
- antithetic @ `60000` sims: abs error `0.008566`, standard error `0.019235`
- control variate @ `60000` sims: abs error `0.000166`, standard error `0.000635`

This is the clearest V2 improvement story in the project.

## 3. Binomial Convergence

Reference assets:

- [binomial_convergence.svg](results/figures/binomial_convergence.svg)
- [study_highlights.csv](results/tables/study_highlights.csv)

Current highlights:

- `atm_call_1y`: error moves from `0.008871` at `25` steps to `0.001191` at `1600` steps
- `atm_put_1y`: error moves from `0.008338` at `25` steps to `0.001182` at `1600` steps

Interpretation:

- the trend improves overall with refinement
- the path is not perfectly monotonic, which is realistic and worth discussing in interviews

## 4. FDM Grid / Runtime Trade-Off

Reference assets:

- [fdm_grid_tradeoff.svg](results/figures/fdm_grid_tradeoff.svg)
- [study_highlights.csv](results/tables/study_highlights.csv)

Current highlight on `atm_call_1y`:

- error drops from `0.260633` at `J=40, N=8000` to `0.027572` at `J=120, N=72000`
- runtime rises to about `895.243792 ms` at the finest current grid in the latest run

Interpretation:

- grid refinement materially improves accuracy
- runtime increases sharply, making the accuracy/cost trade-off easy to explain visually

## 5. Validation Status

Reference assets:

- [validation_overview.csv](results/tables/validation_overview.csv)
- [validation_summary.csv](results/tables/validation_summary.csv)
- [validation_summary.md](results/tables/validation_summary.md)

Current status:

- **9 / 9 validation checks passed**

Checks currently cover:

- exact baseline repeatability
- exact-price regression anchors
- exact put-call parity
- benchmark output sanity
- binomial and FDM tolerance checks
- MC control-variate sanity checks

This adds a useful layer of engineering credibility without turning the project into a heavyweight testing framework.

## 6. Best Evidence To Surface In README Or Interviews

If space is limited, these are the strongest proof points:

1. **Unified benchmark:** one European vanilla case pack, four pricing families, shared CSV outputs.
2. **MC V2 upgrade:** crude, antithetic, and control-variate MC with standard error and 95% confidence interval reporting.
3. **Clear variance-reduction result:** control variate reduces average MC abs error from `0.091678` to `0.002914`.
4. **Validation discipline:** the benchmark currently passes `9 / 9` lightweight validation checks.
5. **Method trade-off clarity:** Binomial is still the best overall speed/accuracy compromise, while FDM and MC illustrate different numerical trade-offs.
