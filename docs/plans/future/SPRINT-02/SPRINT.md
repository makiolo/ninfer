# SPRINT-02 — Rendimiento de NInfer en AMD R9700

**Status:** Planning
**Planning started:** 2026-11-16
**Start:** pending
**End:** pending

## 1. Goal

Medir y mejorar el decode y el prefill de Qwen3.8-27B en la R9700: línea base reproducible, GEMV Q4/Q5 ajustado a
RDNA4 y prefill con WMMA de gfx12, sin perder la corrección validada en SPRINT-01.

## 2. Responsibilities and governance

Las mismas filas que SPRINT-01 ([GOVERNANCE.md](../../GOVERNANCE.md)); revalidar al promoverlo a `next/`.

## 3. Group plans

| Plan | Group | Tasks | Depends on |
|---|---|---|---|
| [PLAN_PERF.md](PLAN_PERF.md) | Rendimiento RDNA4 | R97-19..21 | R97-18 (SPRINT-01) |

## 4. External dependencies

Ninguna. Premisa interna: SPRINT-01 cerrado con R97-18 aceptada.

## 5. Decisions (resolved GAPs)

| GAP | Question | Decision (who, date) |
|---|---|---|
| Q-007 | PERF en SPRINT-01 o SPRINT-02 | SPRINT-02 (usuario, 2026-11-16) |

## 6. Exclusions

Las de SPRINT-01 (Vision, segunda GPU, DFlash, NVFP4, otros modelos).

## 7. Order, lanes and mutexes

R97-19 → (R97-20 ∥ R97-21); la GPU dedicada se usa de uno en uno.

## 8. Definition of Done

- Suites de lineal y atención en verde en `r9700-dev`; mejoras medidas frente a R97-19 con condiciones reproducibles.
- Registro de cierre por tarea; STATUS.md e issues al día.

## 9. Capacity and budget

Pendiente; se presupuesta con los consumos medidos de SPRINT-01.
