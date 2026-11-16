# USAGE — SPRINT-01

Informe derivado del registro global `docs/plans/usage/` según [USAGE_PROTOCOL.md](../USAGE_PROTOCOL.md).
Corte: 2026-11-16 21:27 UTC. Responsable de la medición: el director del sprint (cuenta Claude principal, Codex
prolite y Antigravity).

## Lectura inicial

| Proveedor | Barra | Usado | Reinicio (UTC) | Observación |
|---|---|---:|---|---|
| Claude | semanal, todos los modelos | 96 % | 2026-11-16 23:59 | ninfer-s01-start-claude-all |
| Claude | semanal Fable | 100 % | 2026-11-17 00:00 | ninfer-s01-start-claude-fable |
| Codex | semanal | 94 % | 2026-11-21 11:38 | ninfer-s01-start-codex |
| Antigravity | Gemini 4 Argon | 91,76 % | 2026-11-21 06:43 | ninfer-s01-start-agy-argon |
| Antigravity | Gemini 3.8 Flash | 39,10 % | 2026-11-22 02:01 | ninfer-s01-start-agy-flash |

El grupo «Claude and GPT models» de Antigravity aparece con 0 % sin una semántica clara (¿usado o restante?): es una
laguna, no una cuota disponible.

## Presupuesto

Sin muestras comparables (primer sprint del proyecto): **escenario hipotético**, sin precisión inventada.

| Bloque | Tareas | Niveles | Supuesto |
|---|---|---|---|
| BUILD | R97-01..04 | L3–L6 | configuración y cabecera de compatibilidad; muchas iteraciones de compilación |
| HOST | R97-05..08 | L4–L5 | cambios acotados en ~13 ficheros |
| OPS | R97-09..15 | L6–L7 | la mayor parte del coste: kernels, oráculos y revisiones de la pareja fuerte |
| INTEGRATION | R97-16..18 | L5–L6 | runtime, descarga de 19 GiB y validación end-to-end |

Supuesto prudente: varias semanas completas de cuota de Claude más la de Codex desde su reinicio. Se recalibra al
aceptar las primeras tareas de cada bloque.

## Real y desviación

Pendiente: se rellena con los segmentos medidos al aceptar cada tarea (`usage/tasks.csv`, `usage/intervals.csv`).
