# [ISSUE-005] Suite completa en `r9700`: casos fuera de alcance que se registran y fallan, y un rechazo sin caso

> **Type:** Defect (cobertura y registro de tests)\
> **Status:** Open\
> **Detected:** 2026-11-17\
> **Origin:** SPRINT-01 / PLAN_OPS / revisiones de R97-13 (530), R97-14 (528) y R97-10 (529)\
> **Affected component:** `tests/ops/softmax_attention`, `tests/ops/kv_cache`, registro de tests de `r9700`\
> **Reference specification:** CONCERTINO.md §3 («Full suite (DoD and close)»), PLAN_INTEGRATION R97-18\
> **Severity:** Medium\
> **Destination:** Current sprint (antes del DoD de R97-18)\
> **Priority and reason:** la suite completa de `ctest` no puede salir en verde mientras haya casos que llegan a Ops no portadas\
> **Owner:** a asignar (tarea corta de registro de tests, autor Codex)\
> **Follow-up:** se asigna cuando cierren las Ops (R97-11..13, R97-15) y se conozca la lista completa

## 1. Contexto y comportamiento observado
1. **Atención.** El catálogo BF16 de `ninfer_softmax_attention_test` tiene 57 casos fuera del alcance de `r9700`
   (`packed_attention` 40, `context_attention` 17). La validación de R97-13 los excluyó de forma explícita con
   `--suite`; la orden literal `--format bf16` falla porque esos casos llegan a los rechazos de
   `dense/r9700/unsupported.cpp`.
2. **Caché KV.** El rechazo explícito de INT8, NVFP4 y K8V4 de `src/ops/kv_cache/kernels/write.cu:155-159` no lo
   ejercita ningún caso: la atención rechaza antes, y solo se llega desde `kv_cache/append/dispatch.cu:43,65`.
3. **Dianas CUDA.** Los casos nuevos de los oráculos (R97-10: 598; R97-13: 83; y los de R97-11, R97-12, R97-14 y
   R97-15) se registran también en las dianas CUDA y nadie los ha ejecutado allí. Esta máquina no puede compilarlas
   (CONCERTINO.md §11): queda como pendiente para quien compile en Spark o 5090, no para este sprint.

## 2. Divergencia
AGENTS.md: lo no soportado se rechaza de forma explícita, y eso se cumple. Lo que falta es que el registro de tests de
`r9700` refleje el soporte declarado (como hizo R97-15 con sus cinco ficheros excluidos) y que cada rechazo tenga un
caso.

## 3. Caso mínimo
`ninfer_softmax_attention_test --format bf16` en `r9700`: falla en los casos de `packed_attention` y
`context_attention`.

## 4. Causa
Las suites se escribieron para dianas que portan todas las rutas; el port a `r9700` es parcial por diseño.

## 5. Criterios de resolución
- [ ] 1. En `r9700`, los casos de rutas no portadas no se registran o comprueban el rechazo explícito, con su motivo
      en la lista de excluidos (sin cambiar esperados ni cotas de los casos portados).
- [ ] 2. Caso de rechazo para `kv_cache_append` con INT8, NVFP4 y K8V4.
- [ ] 3. `ctest --test-dir build/r9700-dev --output-on-failure` en verde (DoD de R97-18).
