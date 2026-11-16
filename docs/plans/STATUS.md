# Global status

What we have and what is missing, per area, in one or two lines per row with the ID that covers each gap. Updated in
the same commit that closes each task; the detail lives in the plans.

| Area | We have | Missing (ID) |
|---|---|---|
| Ejecución en AMD R9700 | Diana `r9700` en CMake y presets `r9700-*` (R97-01, `d3302962`) | Dependencias host y smoke (R97-02..04), host Windows (R97-05..08), Ops HIP (R97-09..15), runtime y e2e (R97-16..18), rendimiento (R97-19..21) |
| Uso de las 2 R9700 para un modelo | Nada: TP solo en GLM5Next/Qwen4Exp, en Linux, por RDMA/NCCL | Medición y gate (R97-22), distribuido en Windows (R97-23), comunicación por el host (R97-24), TP2 de Qwen3.5 (R97-25), equivalencia (R97-26), rendimiento (R97-27) |
