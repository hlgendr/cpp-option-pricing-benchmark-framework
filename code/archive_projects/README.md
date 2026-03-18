# Archived Standalone Reference Solutions

This directory preserves the original standalone Visual Studio solutions that predate the unified benchmark framework.

These solutions are kept for reference, comparison, and historical context:

- `Binomial/`
- `Exact/`
- `ExcelVisualisation/`
- `FDM/`
- `MC/`

They are no longer part of the primary GitHub-facing workflow.

Some archived solutions that retain Excel output paths may require these environment variables before building:

- `OFFICE_MSO_DLL`
- `OFFICE_VBE6EXT_OLB`
- `OFFICE_EXCEL_EXE`

The main project entrypoint is:

- [../Projects/OptionPricingBench/](../Projects/OptionPricingBench/)

The shared benchmark layer lives under:

- [../ResumeCore/](../ResumeCore/)
