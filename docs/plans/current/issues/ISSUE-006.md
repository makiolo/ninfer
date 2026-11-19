# [ISSUE-006] Inyección de fallos de los tests host sin equivalente COFF en Windows

> **Type:** Improvement\
> **Status:** Open\
> **Detected:** 2026-11-17\
> **Origin:** SPRINT-01 / PLAN_HOST / R97-08 (revisión 547, Sonnet 5.5 high, condición 3)\
> **Affected component:** `tests/symbol_interpose.h` y los tres tests que la usan\
> **Reference specification:** PLAN_HOST R97-08; inventario `.agent-tmp/R97-08/excluded.md` (causa «Harness Linux no aplicable a Windows»)\
> **Severity:** Low\
> **Destination:** Backlog (fuera del objetivo de SPRINT-01; decisión del director, EXECUTION 2026-11-17)\
> **Priority and reason:** solo cobertura de rutas de fallo de I/O y de eventos en Windows; el camino normal está probado\
> **Owner:** por asignar (dueño de la infraestructura de tests host)\
> **Follow-up:** backlog tras SPRINT-01\

---

## 1. Context and observed behaviour
- `tests/symbol_interpose.h:3` incluye `<dlfcn.h>` e intercepta funciones con `dlsym(RTLD_NEXT)`; el CMake exporta
  los símbolos con `--export-dynamic-symbol`. El enlazador COFF de Windows no tiene equivalente, así que en `r9700`
  quedan fuera del filtro host:
  - `ninfer_artifact_writer_interop_test` (llama a `materialization_test`, interpone ELF y `statx`);
  - `ninfer_artifact_cpu_materialization_test` (fallos de I/O inyectados, `statx` en `materialization_io.cpp`);
  - `ninfer_ple_disk_prepare_host_test` (interpone `cudaEventRecord`; además depende de PLE, fuera del hito).
  - Añadidos en R97-17 (2026-11-19, `.agent-tmp/R97-17/acceptance.md`): `ninfer_qwen3_5_prefix_real_test` (interpone
    `cudaEventQuery`) y los que reutilizan su ejecutable, `ninfer_qwen3_5_terminal_publication_real_test` y
    `ninfer_qwen3_5_agent_continuation_real_test`; y `ninfer_artifact_materialization_test` (`dlfcn.h`, `statx`).
    Estos tres de Qwen3.5 cubren reutilización de prefijos y publicación de terminales con el artefacto real: subir la
    prioridad si `claude-local` (R97-32) depende de la reutilización de prefijos.
- Esperado: los mismos fallos inyectados se comprueban también en Windows.

## 2. Divergence from the specification
Ninguna invariante del producto rota; falta cobertura de las rutas de error de materialización en Windows.

## 3. Minimal reproducible case
`ctest --test-dir build/r9700-dev -R "artifact_writer_interop|artifact_cpu_materialization|ple_disk_prepare_host"`:
los tres no están registrados en `r9700` (excluidos en R97-08).

## 4. Root cause analysis
La interposición depende de la resolución dinámica de símbolos de ELF. En COFF haría falta otro mecanismo (punto de
inyección explícito en el módulo dueño, Detours o IAT hooking), manteniendo los mismos fallos inyectados.

## 5. Acceptance and resolution criteria
- [ ] 1. Los tres tests compilan y pasan en `r9700` con los mismos fallos inyectados que en Linux (sin quitar casos).
- [ ] 2. Sin coste en el camino normal (la inyección no entra en binarios de producto).
- [ ] 3. Las dianas CUDA (Linux) siguen igual.
- [ ] 4. No aplica documentación de producto.
