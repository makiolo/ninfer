# Global status

What we have and what is missing, per area, in one or two lines per row with the ID that covers each gap. Updated in
the same commit that closes each task; the detail lives in the plans.

| Area | We have | Missing (ID) |
|---|---|---|
| Ejecución en AMD R9700 | Diana `r9700` en CMake y presets `r9700-*` (R97-01, `d3302962`); dependencias host con vcpkg y generación de `r9700-*` (R97-03, `21297e2d`); cabecera `core/cuda_compat.h` con FP8 e4m3fn/BF16 verificados en gfx1201 (R97-02, `36f36b4a`); smoke de dispositivo en la ASUS (R97-04, `de5cce05`) — hito 1; lectura de pesos con IOCP en Windows (R97-05, `168398d7`); topología, señales y logging en Windows (R97-06, `92ae1c50`); servidor HTTP y medios en Windows (R97-07, `535693be`); primitivas comunes de GPU en HIP wave32 (R97-09, `dec8d874`); caché KV BF16 y FP8 en HIP (R97-14, `acadec71`); lineales Q4/Q5/Q8 en HIP (R97-10, `7f252ce2`) | Host Windows (R97-08), Ops (R97-11..13, R97-15 y `linear/swiglu`); ISSUE-001, ISSUE-002, host Windows (R97-05..08), Ops HIP (R97-09..15), runtime y e2e (R97-16..18), rendimiento (R97-19..21) |
| Uso de las 2 R9700 para un modelo | Nada: TP solo en GLM5Next/Qwen4Exp, en Linux, por RDMA/NCCL | Medición y gate (R97-22), distribuido en Windows (R97-23), comunicación por el host (R97-24), TP2 de Qwen3.5 (R97-25), equivalencia (R97-26), rendimiento (R97-27) |
