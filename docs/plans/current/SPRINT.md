# SPRINT-01 — Port de NInfer a AMD R9700 (ROCm/HIP, Windows nativo)

**Status:** Active
**Planning started:** 2026-11-16
**Start:** 2026-11-16
**End:** pending

## 1. Goal

Ejecutar Qwen3.8-27B (`groupwise-int`) en una AMD Radeon AI PRO R9700 sobre Windows nativo con HIP SDK 7.2:
generación de texto con MTP y caché KV FP8 desde `apps/ninfer`, con cada Op validada contra su referencia
independiente y la perplejidad comprobada. La optimización para RDNA4 va en SPRINT-02 (Q-007).

## 2. Responsibilities and governance

Filas de [GOVERNANCE.md](../GOVERNANCE.md) que afectan a este sprint.

| Thing | Owner | Writes | Decides | Changes from another project |
|---|---|---|---|---|
| Código upstream (`src/`, `apps/`, `cmake/`, `tests/`, `bench/`) | upstream `Neroued/ninfer` | sesiones ninfer (solo cambios del port) | usuario | ninguno |
| Diana `r9700`, compatibilidad HIP y kernels AMD | ninfer | sesiones ninfer | usuario | nota o issue en este repo |
| `AGENTS.md` y docs estables | ninfer | solo a petición del usuario | usuario | — |
| Artefacto `out/qwen3_8_27b.ninfer` | ninfer | R97-17 | usuario | — |
| GPU R9700 dedicada a ninfer (Q-006) | máquina del usuario | solo sesiones ninfer durante el sprint | usuario | — |

## 3. Group plans

| Plan | Group | Tasks | Depends on |
|---|---|---|---|
| [PLAN_BUILD.md](PLAN_BUILD.md) | Toolchain y diana `r9700` | R97-01..04 | — |
| [PLAN_HOST.md](PLAN_HOST.md) | Lado CPU en Windows | R97-05..08 | R97-01, R97-03 |
| [PLAN_OPS.md](PLAN_OPS.md) | Ops HIP de Qwen3.8-27B | R97-09..15 | R97-02, R97-04 |
| [PLAN_INTEGRATION.md](PLAN_INTEGRATION.md) | Runtime y validación end-to-end | R97-16..18 | HOST, OPS |

Plan de origen: `backlog/PLAN_ROCM_R9700.md`, conservado en la etiqueta `archive/2026-11-16/sprint-01-sources`
(`git show archive/2026-11-16/sprint-01-sources:docs/plans/backlog/PLAN_ROCM_R9700.md`) y retirado del backlog al
activar: todas sus tareas están en planes de grupo. Entran R97-01..18; R97-19..21 (PERF) van a
[SPRINT-02](../next/SPRINT.md) por decisión del usuario (Q-007).

## 4. External dependencies

Ninguna ([DEPENDENCIES.md](../DEPENDENCIES.md)).

## 5. Decisions (resolved GAPs)

| GAP | Question | Decision (who, date) |
|---|---|---|
| Q-001 | Sistema operativo del port | Windows nativo (usuario, 2026-11-16) |
| Q-002 | Alcance del primer hito | 27B texto + MTP + KV FP8 (usuario, 2026-11-16) |
| Q-003 | Organización del código | Mismo árbol, diana `r9700` (usuario, 2026-11-16) |
| Q-004 | Artefacto de pesos | Descargarlo cuando toque, a `out/` (usuario, 2026-11-16) |
| Q-005 | Remoto para commits/push | Rama local `fork_R9700`, sin push a `origin` (usuario, 2026-11-16) |
| Q-006 | GPU dedicada frente a ranuras de IA local | Una R9700 fija y exclusiva para ninfer (usuario, 2026-11-16); cuál, al iniciar |
| Q-007 | PERF en este sprint o en SPRINT-02 | Dividir: PERF a SPRINT-02 (usuario, 2026-11-16) |
| Q-009 | Qué R9700 se dedica a ninfer | ASUS, índice HIP 0 (usuario, 2026-11-16) |
| Q-010 | Proveedores al iniciar | Claude (~4 %) orquesta, Codex implementa (usuario, 2026-11-16) |

## 6. Exclusions

| What | Reason | Destination |
|---|---|---|
| PERF: R97-19..21 (línea base, GEMV RDNA4, prefill WMMA) | Q-007: SPRINT-01 cierra al funcionar correcto | SPRINT-02 (next/) |
| Vision (encoder, FFmpeg/JPEG) | Fuera del primer hito (Q-002) | backlog (tras SPRINT-01) |
| Segunda GPU / tensor parallel TP2 | Sin RCCL ni P2P en Windows; requiere hito de 1 GPU | SPRINT-02 (next/PLAN_TP.md) |
| DFlash / DFlash2 | Fuera del primer hito | backlog |
| NVFP4 y artefactos `_nvfp4` | Sin FP4 nativo en RDNA4 | backlog |
| MoE, offload de expertos, Flash Next, GLM | Otros modelos | backlog |
| Builds CUDA en esta máquina | El upstream solo compila en Linux | ninguno (solo revisión de configuración) |

## 7. Order, lanes and mutexes

- Orden: BUILD → (HOST ∥ OPS) → INTEGRATION.
- Carriles: HOST y OPS en paralelo tras R97-04 (ficheros disjuntos); dentro de OPS, R97-10..14 en paralelo tras R97-09.
- Mutex: `CMakeLists.txt`/`cmake/`/`CMakePresets.json` (una tarea a la vez); `src/core/` (R97-02 antes que el resto);
  `src/ops/common/` (solo R97-09); **GPU: un solo descendiente con GPU a la vez**, en la R9700 dedicada (Q-006),
  la ASUS (HIP 0), seleccionada con `HIP_VISIBLE_DEVICES=0`; la Sapphire (HIP 1) sigue para la IA local.
- Antes de la primera tarea con GPU: el usuario descarga el modelo de LM Studio de la ASUS (Q-009).

## 8. Definition of Done

- Suite completa de CONCERTINO.md "Commands" en verde en `r9700-dev` (tests de GPU en serie), con la lista explícita de
  tests no aplicables a `r9700` y su motivo.
- Cada Op tocada validada contra referencia independiente en la ruta de producción `r9700`.
- `ninfer` genera con Qwen3.8-27B, con MTP y KV FP8; perplejidad dentro de la tolerancia fijada por el oráculo.
- Registro de cierre por tarea (commit, tests, desviaciones); STATUS.md e issues al día.

## 9. Capacity and budget

- Capacidad: director + 3 descendientes activos; solo uno con GPU a la vez.
- Presupuesto: sin comparables en este proyecto (primer sprint). Escenario hipotético por nivel en `USAGE.md` al
  preparar el LAUNCH; medición pendiente.
- LAUNCH: [LAUNCH.md](LAUNCH.md). Presupuesto y lectura inicial: [USAGE.md](USAGE.md).
