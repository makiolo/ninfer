# PLAN_INTEGRATION — Runtime y validación end-to-end

Sprint: SPRINT-01 · Grupo: INTEGRATION · Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F4–F5.
Depende de: HOST (R97-05..08) y OPS (R97-09..15). Bloquea: PERF.

## 1. Contexto y estado de partida

- El runtime usa CUDA Graph (28 ficheros con `cudaGraph`), streams, eventos, memoria pinned (`cudaMallocHost`/
  `cudaHostRegister`) y consultas de memoria para el planificador de capacidad.
- Windows: TDR mata kernels de más de ~2 s por defecto; WDDM reserva parte de la VRAM.
- Artefacto: `neroued/Qwen3.8-27B-NInfer`, `qwen3_8_27b.ninfer`, 20,437,521,664 bytes (19.03 GiB).

## 2. Decisiones fijadas

- Primer hito: texto + MTP + KV FP8, una GPU, sin Vision ni DFlash (usuario, 2026-11-16).
- El artefacto se descarga a `out/` solo en R97-17 (usuario, 2026-11-16).

## 3. Tareas

### R97-16 — Runtime en HIP
- Nivel L6 · Riesgo alto (ciclo de vida y concurrencia) · Autor: strong · Revisor: strong de otro proveedor.
- hipGraph para la captura y el replay del decode, streams y eventos, memoria pinned, consultas de memoria del
  planificador de capacidad (contando la reserva de WDDM). Documentar en el cierre el límite de TDR observado.
- Aceptación: tests de runtime y de capacidad en verde en `r9700`; test de captura y replay de graph.

### R97-17 — Carga del modelo y generación
- Nivel L5 · Riesgo medio · Autor: strong · Revisor: otro proveedor.
- Descargar `qwen3_8_27b.ninfer` a `out/` (comprobar tamaño exacto) y cargarlo; generación con `apps/ninfer`.
- Aceptación: `ninfer out/qwen3_8_27b.ninfer --prompt ... --max-context 8192 --max-tokens 256` produce texto
  coherente; tests de modelo Qwen3.5 (carga y generación) en verde con el artefacto explícito.

### R97-18 — Corrección numérica end-to-end y MTP/KV FP8
- Nivel L6 · silent-numeric · Autor: strong · Revisor: strong de otro proveedor.
- `ninfer-perplexity` en un corpus fijo, comparado con la referencia del model card o una ejecución CUDA
  documentada; generación con MTP y `--kv-dtype fp8`; tasa de aceptación MTP registrada.
- Aceptación: perplejidad dentro de la tolerancia acordada en la tarjeta (el oráculo la fija antes de ejecutar);
  tests MTP de `tests/models/qwen3_5` en verde.

## 4. Orden y paralelismo

R97-16 → R97-17 → R97-18, en serie (GPU y artefacto compartidos).

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
