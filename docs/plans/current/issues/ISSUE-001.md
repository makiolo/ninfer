# [ISSUE-001] Targets solo de rtx5090 enlazados sin condición en `r9700` (`ninfer_cpu_executor`, `CUDA::nvtx3`)

> **Type:** Defect\
> **Status:** Open\
> **Detected:** 2026-11-16\
> **Origin:** SPRINT-01 / PLAN_BUILD / R97-03 (revisión 491, Sonnet 5.5 high)\
> **Affected component:** CMake de `src/core`, `src/ops/moe/routed_swiglu/cpu`, `tests/cmake/CoreTests.cmake`\
> **Reference specification:** PLAN_BUILD R97-04 (enlazar `core`), PLAN_HOST R97-06, PLAN_OPS\
> **Severity:** Medium\
> **Destination:** Current sprint\
> **Priority and reason:** rompe el enlace de `core`/tests en `r9700`; no bloquea la generación\
> **Owner:** R97-04 (si el smoke enlaza `core`), si no la tarea que porte `core/cpu` o la Op `moe`\
> **Follow-up:** incluir en la ficha de R97-04 y, si no se resuelve allí, en R97-06 / PLAN_OPS\

---

## 1. Context and observed behaviour
- `cmake --preset r9700-ops` genera (`21297e2d`), pero `src/ops/moe/routed_swiglu/cpu/sources.cmake:35` y
  `tests/cmake/CoreTests.cmake:18,23` enlazan `ninfer_cpu_executor`, target que solo existe con `rtx5090`
  (`src/core/CMakeLists.txt:8`). En `r9700` CMake lo trata como librería: `-lninfer_cpu_executor.lib`
  (`build.ninja:6768` de la reconfiguración del revisor).
- `CUDA::nvtx3` se mapea a `ninfer_r9700_nvtx3_unsupported` (`cmake/Dependencies.cmake:27-30`): falla al enlazar, y
  `core/nvtx.h` incluye `<nvtx3/nvToolsExt.h>`, así que la compilación falla antes.

## 2. Divergence from the specification
- AGENTS.md: lo no soportado falla de forma explícita; aquí falla tarde y con un error de enlazador poco claro.

## 3. Minimal reproducible case
`cmake --fresh --preset r9700-ops -B <dir>` y buscar `ninfer_cpu_executor` en `<dir>/build.ninja`.
