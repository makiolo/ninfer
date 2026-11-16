# PLAN_BUILD — Toolchain y diana `r9700`

Sprint: SPRINT-01 · Grupo: BUILD · Fuente: `backlog/PLAN_ROCM_R9700.md` (etiqueta `archive/2026-11-16/sprint-01-sources`) §F1 (2026-11-16, sin commit previo).
Depende de: nada. Bloquea: HOST, OPS, INTEGRATION, PERF.

## 1. Contexto y estado de partida

- `cmake/NinferPlatform.cmake` solo acepta `rtx5090` (`sm_120a`) y `dgx_spark` (`sm_121a`) y exige Linux.
  `CMakeLists.txt` declara `project(ninfer LANGUAGES C CXX CUDA)` y `cmake/Dependencies.cmake` usa
  `find_package(CUDAToolkit)` y `pkg_check_modules` (hwloc, liburing, libcurl, FFmpeg).
- `src/core/platform.h.in` define las capacidades por diana (`NINFER_SHARED_PHYSICAL_MEMORY`, `NINFER_EXPERT_OFFLOAD`).
  74 ficheros usan las macros `NINFER_TARGET_*`.
- HIP SDK 7.2 en `S:\Apps\AMD\ROCm` (clang 21, `hipcc`, rocWMMA, hipCUB, hipBLASLt). CMake 3.28.0-rc5.

## 2. Decisiones fijadas

- Mismo árbol: diana nueva `NINFER_TARGET=r9700` (`gfx1201`, Windows/AMD64). La build CUDA no cambia de comportamiento.
- Los `.cu` existentes se compilan como HIP en la diana `r9700`; una sola cabecera de `core` resuelve nombres de
  runtime y tipos. No hay `#ifdef` de plataforma repartidos por las Ops (AGENTS.md: cada módulo decide su soporte).

## 3. Tareas

### R97-01 — Diana `r9700` en CMake, presets y `platform.h`
- Nivel L5 · Riesgo medio · Autor: strong (`claude-or route --level 5`) · Revisor: otro proveedor.
- Añadir `r9700` a `NinferPlatform.cmake` (arquitectura `gfx1201`, Windows/AMD64), lenguaje HIP para esa diana,
  `NINFER_TARGET_R9700` y capacidades en `platform.h.in` (`SHARED_PHYSICAL_MEMORY 0`, `EXPERT_OFFLOAD 0`),
  presets `r9700-release|dev|ops` en `CMakePresets.json`. Compilar `.cu` como HIP (`LANGUAGE HIP`) solo en `r9700`.
- Comprobar si CMake 3.28.0-rc5 soporta HIP en Windows; si no, BLOCKED con la versión mínima necesaria (la
  actualización la decide el usuario).
- Aceptación: `cmake --preset r9700-ops` configura; los presets CUDA siguen generando la misma configuración (diff
  de `CMakeCache` o revisión de la lógica, porque no se pueden construir en esta máquina).

### R97-02 — Cabecera de compatibilidad HIP (runtime y tipos)
- Nivel L6 · Riesgo silent-numeric (tipos FP8/BF16) · Autor: strong · Revisor: strong de otro proveedor · Oráculo:
  casos de conversión FP8 e4m3fn/BF16 contra tablas de bits.
- Una cabecera en `src/core/` que mapea `cuda*`→`hip*` (streams, eventos, memoria, graphs, errores) y los tipos
  `__nv_bfloat16`, `__nv_fp8_e4m3` (OCP e4m3fn en gfx12), intrínsecos de conversión usados por el código.
- Aceptación: test de conversión exhaustivo de los 256 valores FP8 y de redondeo BF16 contra la referencia de bits;
  compila un `.cu` representativo de cada carpeta de `src/ops` hasta el primer error propio de PTX (lista en el informe).

### R97-03 — Dependencias host con vcpkg
- Nivel L3 · Riesgo bajo · Autor: medium · Revisor: medium de otro proveedor.
- Resolver hwloc, libcurl, libjpeg-turbo y FFmpeg (opcional mientras Vision quede fuera) con vcpkg en Windows;
  `Dependencies.cmake` usa `find_package`/`pkg_check_modules` según la diana. liburing queda fuera de `r9700` (R97-05).
- Aceptación: configuración `r9700-dev` encuentra todas las dependencias sin rutas absolutas en el repo.

### R97-04 — Smoke de dispositivo en `gfx1201`
- Nivel L3 · Riesgo bajo · Autor: medium · Revisor: medium de otro proveedor.
- Compilar y enlazar la biblioteca de `core` y un test de kernel trivial (escritura/lectura y stream) en `gfx1201`.
- Aceptación: `ctest --test-dir build/r9700-ops -R <smoke>` en verde en la GPU asignada (lock de GPU, Q-006).

## 4. Orden y paralelismo

R97-01 → R97-02 → R97-04; R97-03 en paralelo con R97-02 (ficheros disjuntos: `cmake/Dependencies.cmake` frente a
`src/core/`). Mutex: `CMakeLists.txt` y `cmake/` los escribe solo una tarea a la vez.

## 5. Registro de cierre

| ID | Commit | Tests | Desviaciones |
|---|---|---|---|
| R97-01 | `92958cf5`, `d3302962` | `cmake --preset r9700-ops` pasa `project(HIP)` `gfx1201`, plataforma, `platform.h` y `find_package(hip)`; primer error `hwloc>=2.5` (`Dependencies.cmake:16`, R97-03). Revisión Sonnet 5.5 high: ACCEPT WITH CONDITIONS (3 corregidas en `d3302962`). CUDA: variables idénticas por `cmake -P` (no construible aquí). | Aceptación «configura» parcial por diseño: la configuración completa se verifica al cerrar R97-03. Requiere entorno MSVC + `HIP_PATH`. |
