# SPRINT-02 — Rendimiento y tensor parallel TP2 de NInfer en AMD R9700

**Status:** Planning
**Planning started:** 2026-11-16
**Start:** pending
**End:** pending

## 1. Goal

Medir y mejorar el decode y el prefill de Qwen3.8-27B en la R9700: línea base reproducible, GEMV Q4/Q5 ajustado a
RDNA4 y prefill con WMMA de gfx12, sin perder la corrección validada en SPRINT-01. Además, ejecutar el mismo modelo en
las dos R9700 con tensor parallel TP2 para subir el prefill y los tok/s, si la medición previa (R97-22) lo justifica.

## 2. Responsibilities and governance

Las mismas filas que SPRINT-01 ([GOVERNANCE.md](../../GOVERNANCE.md)); revalidar al promoverlo a `next/`.

## 3. Group plans

| Plan | Group | Tasks | Depends on |
|---|---|---|---|
| [PLAN_PERF.md](PLAN_PERF.md) | Rendimiento RDNA4 | R97-19..21 | R97-18 (SPRINT-01) |
| [PLAN_TP.md](PLAN_TP.md) | Tensor parallel TP2 en 2 GPUs | R97-22..27 | R97-18, R97-19; R97-22 es gate del usuario |

## 4. External dependencies

Ninguna. Premisa interna: SPRINT-01 cerrado con R97-18 aceptada.

## 5. Decisions (resolved GAPs)

| GAP | Question | Decision (who, date) |
|---|---|---|
| Q-007 | PERF en SPRINT-01 o SPRINT-02 | SPRINT-02 (usuario, 2026-11-16) |
| — | Tensor parallel con las 2 GPUs | Entra en SPRINT-02 como PLAN_TP (usuario, 2026-11-16) |
| Q-008 | Segunda GPU exclusiva durante TP | **abierta** |

## 6. Exclusions

Las de SPRINT-01 salvo la segunda GPU, que entra con PLAN_TP (Vision, DFlash, NVFP4, otros modelos). Fuera de TP:
nodos remotos, EP y attention DP.

## 7. Order, lanes and mutexes

R97-19 → R97-22 (gate de TP) → (R97-20 ∥ R97-21 ∥ R97-23 ∥ R97-24) → R97-25 → R97-26 → R97-27. Las GPUs se usan de una
tarea en una; las dos GPUs exclusivas durante R97-22 y desde R97-24 (Q-008). `src/distributed/` lo escriben solo
R97-23 y R97-24, en ficheros disjuntos (`launch/bootstrap/transport` frente a `communication/`).

## 8. Definition of Done

- Suites de lineal y atención en verde en `r9700-dev`; mejoras medidas frente a R97-19 con condiciones reproducibles.
- TP2 (si R97-22 da luz verde): equivalencia numérica con 1 GPU y tabla de rendimiento TP2 frente a 1 GPU.
- Registro de cierre por tarea; STATUS.md e issues al día.

## 9. Capacity and budget

Pendiente; se presupuesta con los consumos medidos de SPRINT-01.
