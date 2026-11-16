# Port de NInfer a AMD Radeon AI PRO R9700 (ROCm/HIP, Windows nativo)

Plan temporal de trabajo (no es documentación estable).

## Decisiones tomadas

- SO: **Windows nativo**, HIP SDK 7.2 en `S:\Apps\AMD\ROCm` (clang 21, `gfx1201`).
- Primer hito: **Qwen3.8-27B `groupwise-int`, texto + MTP + caché KV FP8, 1 GPU**.
- Código: **mismo árbol**, nueva diana `NINFER_TARGET=r9700`; la build CUDA (rtx5090, dgx_spark) no se rompe.
- Pesos: `neroued/Qwen3.8-27B-NInfer` → `out/qwen3_8_27b.ninfer` (19 GiB), se descarga al llegar a la fase 5.

## Hechos de partida

- Hardware: 2× R9700, `gfx1201` (RDNA4), 32 GB, warp de 32, 32 CUs, ~640 GB/s.
- Device: 925 `.cu`/`.cuh`, ~126k líneas. PTX inline en 57 ficheros (`ldmatrix`, `mma`, `cp.async`, TMA,
  `mbarrier`); no se puede traducir y requiere un kernel AMD o la ruta SIMT/GEMV.
- Ya existen rutas SIMT/GEMV sin PTX para Q4/Q5/Q6/Q8/BF16/FP8 (`src/ops/linear/kernels/*/..._simt|gemv`).
- Host exclusivo de Linux (sin contar distributed, que queda OFF): `core/io/{file,read_queue}`,
  `core/cpu/system`, `product/logging/*`, `product/media_acquire`, `serve/*`, `apps/serve/main`,
  `runtime/engine/context_cache/context_cost`.
- En Windows ya hay CMake 3.28.0-rc5, Ninja y vcpkg (`G:\vcpkg_root`).

## Fases

### F1. Toolchain y diana `r9700`
- `cmake/NinferPlatform.cmake`: diana `r9700` (`gfx1201`, Windows/AMD64) y lenguaje HIP para esa diana.
  Los `.cu` se compilan como HIP (`set_source_files_properties(... LANGUAGE HIP)`) y se añaden presets `r9700-*`.
- `platform.h.in`: `NINFER_TARGET_R9700` y sus capacidades (`SHARED_PHYSICAL_MEMORY 0`, `EXPERT_OFFLOAD 0`
  al principio).
- Cabecera de compatibilidad de runtime: `cuda*`→`hip*` y tipos (`__nv_bfloat16`, `__nv_fp8_e4m3`→OCP e4m3fn de gfx12,
  `cudaStream_t`, eventos, graphs). Se resuelve en un único sitio de `core`, sin `#ifdef` repartidos.
- Dependencias con vcpkg: libjpeg-turbo, curl, ffmpeg (opcional si Vision queda fuera), hwloc.
- **Verificación:** se configura y compila `ninfer_core` y un test trivial de kernel en `gfx1201`.
- **Riesgo:** el soporte del lenguaje HIP de CMake en Windows quizá exija una versión más nueva que 3.28-rc5.

### F2. Host en Windows
- `core/io`: io_uring → `ReadFile` overlapped/IOCP (o lectura síncrona por bloques al principio), detrás del mismo
  contrato `read_queue`. `O_DIRECT` → `FILE_FLAG_NO_BUFFERING`.
- `core/cpu/system`: hwloc (portable) o `GetLogicalProcessorInformationEx`.
- Señales (`apps/serve`, `logging`) → `SetConsoleCtrlHandler` / `signal` de CRT.
- `serve/http_transport`: sockets POSIX → Winsock (cpp-httplib ya es portable).
- Compilador host: el `clang++` de ROCm (ABI MSVC). Hay que revisar los extensiones de GCC.
- **Verificación:** tests de host (artifact, frontend, tokenizer, protocolo) pasan en Windows.

### F3. Ops para Qwen3.8-27B (texto + MTP + KV FP8)
Por cada Op, la implementación declara qué soporta en `r9700`; las rutas PTX se excluyen
de la diana y la Op las rechaza explícitamente.
- Lineales Q4/Q5/Q8 (`q8_g32_fp16` en embedding/head): rutas GEMV (decode) y SIMT (prefill) primero.
- Gated DeltaNet + conv causal (`linear_attention`).
- Atención softmax densa: prefill y decode sobre caché paginada.
- `kv_cache`: escritura y lectura BF16 y FP8 (`kernels/write/fp8.cuh`, `fp8.cuh`, `hadamard.cuh`).
- RMSNorm, RoPE, SwiGLU, gated residual, embedding, elementwise, reducción, sampling (incluida la aceptación MTP).
- Primitivas: `__shfl*` (warp 32 = wave32 en RDNA4), `__ballot` (máscara de 32 bits), atomics.
- **Verificación:** test de cada Op contra referencia independiente (`docs/maintainer/op-development.md`),
  en la ruta de producción `r9700`.

### F4. Runtime
- CUDA Graph → hipGraph (captura del decode), streams/eventos, memoria pinned (`hipHostMalloc`),
  consultas de memoria para el planificador de capacidad.
- Revisar el TDR de Windows (kernels > 2 s) y la VRAM que reserva WDDM.
- **Verificación:** test de ciclo de vida del Engine, captura y replay de graph.

### F5. End-to-end y corrección
- Descargar `qwen3_8_27b.ninfer` a `out/`.
- `ninfer-perplexity`: comparar con las referencias del model card o de una ejecución CUDA.
- `ninfer` CLI: generación con y sin MTP, KV BF16 y FP8; tasa de aceptación MTP coherente.

### F6. Rendimiento
- Línea base: tok/s de decode (techo teórico ~30 tok/s sin MTP: 640 GB/s ÷ 19 GiB) y tok/s de prefill.
- GEMV Q4/Q5 ajustado a RDNA4 (cargas de 128 bits, ocupación).
- Prefill con WMMA de gfx12 (rocWMMA o `__builtin_amdgcn_wmma_*_gfx12`) para Q4/Q5→BF16 y atención.
- Las Ops de base se miden con `bench/` y el producto con los benchmarks de decode.

### Fuera de este hito
Vision, la segunda GPU (no hay RCCL en Windows), DFlash, NVFP4 (sin FP4 nativo en RDNA4), MoE y offload.
