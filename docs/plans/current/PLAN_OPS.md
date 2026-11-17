# PLAN_OPS — Ops de Qwen3.8-27B en HIP (texto + MTP + KV FP8)

Sprint: SPRINT-01 · Grupo: OPS · Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F3.
Depende de: R97-02, R97-04. Bloquea: INTEGRATION, PERF.

## 1. Contexto y estado de partida

- 925 `.cu`/`.cuh` (~126k líneas); PTX inline en 57 ficheros (`ldmatrix`, `mma`, `cp.async`, TMA, `mbarrier`).
- Rutas sin PTX ya existentes: `src/ops/linear/kernels/{q4,q5,q8}/*_{gemv,simt}*`.
- Qwen3.8-27B `groupwise-int`: proyecciones Q4/Q5, embedding y cabeza `q8_g32_fp16`; arquitectura Qwen3.5 Dense
  (`docs/maintainer/qwen3_5-model.md`): Gated DeltaNet + atención softmax, SwiGLU, MTP.
- RDNA4: wave32, sin FP4 nativo, FP8 OCP (e4m3fn/e5m2).

## 2. Decisiones fijadas

- Cada Op declara su soporte en `r9700`; las rutas PTX se excluyen de la diana y la Op rechaza explícitamente lo que
  no soporta (AGENTS.md). Ni el modelo ni el runtime replican esa regla.
- Validación según `docs/maintainer/op-development.md`: referencia matemática independiente, ruta de producción real.
  La coherencia con otra implementación no prueba corrección.
- Toda tarea de este grupo es de riesgo silent-numeric: un oráculo distinto del autor prepara los casos de referencia
  (harness A7) antes de que el autor empiece.

## 3. Tareas

### R97-09 — Primitivas comunes (`src/ops/common`)
- Nivel L6 · silent-numeric · Autor: strong · Revisor: strong de otro proveedor.
- Shuffles, `__ballot` (máscara de 32 bits en wave32), reducciones de warp/bloque, atomics y utilidades de carga
  vectorizada en HIP.
- Aceptación: tests de reducción y scan con casos de borde (tamaños no múltiplos de 32, NaN, ±Inf).

### R97-10 — Lineales Q4/Q5/Q8 (GEMV y SIMT)
- Nivel L7 · silent-numeric · Autor: pareja fuerte (el de más cuota escribe) · Revisor: el otro de la pareja.
- Habilitar en `r9700` las rutas GEMV (decode) y SIMT (prefill) de Q4, Q5 y `q8_g32_fp16`; el selector de la Op
  rechaza las rutas MMA/TMA en esta diana.
- Aceptación: `ninfer_linear_projection_test --format q4|q5|q8` en verde en `r9700` contra la referencia
  independiente; caso explícito de rechazo de una ruta no soportada.

### R97-11 — Ops elementales del bloque
- Nivel L6 · silent-numeric · Autor: strong · Revisor: strong de otro proveedor.
- Embedding (Q8), RMSNorm, RoPE, SwiGLU, gated residual, elementwise, reducción.
- Aceptación: suites de cada Op en verde en `r9700`.

### R97-12 — Gated DeltaNet y convolución causal
- Nivel L7 · silent-numeric · Autor: pareja fuerte · Revisor: el otro de la pareja.
- `src/ops/linear_attention/{gated_delta_net,causal_conv}` para prefill y decode, con estado recurrente.
- Aceptación: suites de linear attention en verde, incluidas secuencias largas y estado continuado entre llamadas.

### R97-13 — Atención softmax densa (prefill y decode)
- Nivel L7 · silent-numeric · Autor: pareja fuerte · Revisor: el otro de la pareja.
- `src/ops/softmax_attention/dense` sobre caché paginada, sin `ldmatrix`/`mma` PTX en `r9700`.
- Aceptación: suites de atención densa en verde; casos de máscara causal, longitudes no alineadas y contexto largo.

### R97-14 — Caché KV BF16 y FP8
- Nivel L7 · silent-numeric · Autor: pareja fuerte · Revisor: el otro de la pareja.
- `src/ops/kv_cache` (append, materialize, `kernels/write/{bf16,fp8}.cuh`, `hadamard.cuh`) con FP8 OCP.
- Aceptación: suites de KV cache en verde para BF16 y FP8; error de cuantización dentro de la tolerancia del contrato.

### R97-15 — Sampling y aceptación MTP
- Nivel L6 · silent-numeric · Autor: strong · Revisor: strong de otro proveedor.
- `src/ops/sampling` (greedy, top-k/top-p, temperatura) y la verificación/aceptación de borradores MTP.
- Aceptación: suites de sampling en verde; test determinista de aceptación MTP.

## 4. Orden y paralelismo

R97-09 primero. Después R97-10, R97-11, R97-12, R97-13 y R97-14 en paralelo (carpetas disjuntas), R97-15 al final.
Mutex: `src/ops/common/` (solo R97-09) y la GPU (un test de GPU a la vez; lock de GPU, Q-006).

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
| R97-09 | oráculo `040f2a3a`; `ff6599b3`, `80aaf2c5`, `dec8d874` | Oráculo independiente (Claude Sonnet 5.5): referencia C++ contrastada con Python (`Fraction`/`numpy`), 0 discrepancias en 1 154 casos. `ninfer_hip_common_primitives_test` en la ASUS (HIP 0, gfx1201, wave32) con PINNED: 874 797 checks / 0 fallos (EXACT 823 027, BOUND 5 688, PINNED 46 082), reejecutado por el revisor; sin FTZ (subnormales OK). Guarda wave32 comprobada por el director: compilación normal OK, `-mwavefrontsize64` falla con `static_assert`. Revisión Sonnet 5.5 high (507): ACCEPT WITH CONDITIONS (guarda resuelta en `dec8d874`). | Test del oráculo: solo `with_pow2` (compilación) y entrada con signo en atomics (decisión del director: `threadIdx.x` es unsigned también en CUDA; la referencia usa `int`). Shims en `core/cuda_compat.h` sobre el SDK HIP 7.2 (máscaras de 32 bits ensanchadas); `streaming_load`, `smem_addr` y políticas L2 `= delete` en `memory.cuh`; `prefetch_l2` omitido en AMD (pista sin efecto); fuera de alcance `atomicAdd(float4*)`, `mma.cuh`, `pipeline.cuh`. Deuda baja: `__reduce_{and,or,xor}_sync` con bucle de shuffles también con máscara completa (rendimiento, PERF); `HIP_VISIBLE_DEVICES=0` fijo en `tests/cmake/HipCompatTests.cmake:9` (orden de dispositivos de este PC). CUDA/Linux: revisión de lectura, sin compilar. |
| R97-10 | oráculo `0ab0acdc`; `563e4526`, `7adc091d`, `7f252ce2`; corrección del director `3402c172` | Oráculo independiente (Claude): fronteras de filas partidas y rechazos (155 casos Q4, 230 Q5, 213 Q8). En la ASUS (HIP 0, bus 6, gfx1201), HEAD `0f6833f4`: `ninfer_linear_projection_test --format q4|q5|q8` sin filtros, **654 + 514 + 1 742 casos**, 0 fallos, 0 omitidos, salida 0; GEMV y SIMT contra la referencia FP64 con las cotas originales (A16 2^-8, A8 0,04, A4 0,16; margen máximo 0,55 de la cota); 28 comprobaciones de rechazo Q4 en ejecutable aparte. Revisión Opus 5.5 high (529): ACCEPT WITH CONDITIONS, ninguna sobre el código; el director contrastó las condiciones con el código. | Solo en `r9700`: `src/ops/linear/sources.cmake` compila únicamente las rutas de proyección por filas partidas (`kernels/common/r9700/rowsplit.cuh`); MMA, TMA y sliced-K no se compilan; las formas no registradas y los formatos no portados (BF16, Q6, FP8, NVFP4, MXFP8) se rechazan con error explícito (`common/r9700/unsupported.cpp`). Test corregido por el director sin cambiar casos (`cases_bf16.cpp`, tipo del contenedor de fronteras). **Condición 1:** los casos nuevos del oráculo entran también en las dianas CUDA y nadie los ha ejecutado allí (esta máquina no puede); queda pendiente para quien compile en Spark o 5090. **Condición 2:** `split`, `swiglu`, `add`, `bias`, `pair` y `topk` de `src/ops/linear` no se compilan en `r9700`, y `rowsplit.cuh:73` fija el epílogo identidad: `linear/swiglu` necesita el epílogo como parámetro o un kernel propio; va en su ficha. Deuda baja: `#if` de plataforma en `projection/q{4,5,8}/shapes/n*_k{1152,4304,4608}.cpp` en vez del patrón de `r9700/shapes.cpp`. CUDA/Linux: revisión de lectura, sin compilar. |
| R97-14 | oráculo `d6fe1304`; `d2008b61`, `acadec71` (consume los kernels de escritura y las conversiones de `f061bd98`, de R97-13) | Oráculo independiente (Claude Opus 5.5): fronteras E4M3, 65 536 patrones BF16, Hadamard exacta, contraste 0. En la ASUS (HIP 0, bus 6, gfx1201, wave32), HEAD `7ce4982f`: `ninfer_kv_cache_append_test` y `ninfer_kv_cache_append_r9700_test`, **172 casos** (BF16 148 + 4, FP8 8 + 12), 0 omitidos, salida 0, igualdad byte a byte. Sonda de Hadamard: 272 filas, 69 632 valores FP32 exactos. Revisión Opus 5.5 high (528): ACCEPT, sin hallazgos bloqueantes; el director contrastó los dos hallazgos bajos con el código. | Solo en `r9700`: `src/ops/kv_cache` se compila como biblioteca propia (`ninfer_kv_cache_r9700`) con `-ffp-contract=off -fno-fast-math -fno-gpu-flush-denormals-to-zero` en `dispatch.cu` y `write.cu` (clang HIP contrae FMA por defecto); las suites enlazan esa misma biblioteca. Tests: solo el registro del oráculo y el enlace. Fuera de alcance: `context_materialize` (drafter). Deuda baja: el rechazo explícito de INT8/NVFP4/K8V4 en `kernels/write.cu:155-159` no tiene caso propio en la suite de KV (se pasa a la revisión de R97-13, dueña del fichero); `context_materialize` sigue en `ninfer_ops` sin los flags estrictos y los necesitará cuando se porte. Si la revisión de R97-13 cambia `write*.cu`, `convert.cuh` o `fp8.cuh`, se repiten las dos suites de KV. CUDA/Linux: revisión de lectura, sin compilar. |
