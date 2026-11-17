# Director HANDOFF — handoff 5

## Handoff
- Number: 5
- Outgoing: session c988ac27-276b-47fc-bf1f-9af9da268148, claude-fable-5-2, context 68.0 %
- Written: 2026-11-17T08:34:26+01:00
- HEAD at writing: ccda505a8d84b2380e5edd1ce9c8fbb86a31a649
- LAUNCH: docs/plans/current/LAUNCH.md · coordinator block: «Director prompt» (sección final de LAUNCH.md)
- Previous handoffs: `git log --follow -- docs/plans/current/HANDOFF.md`

## Boot
0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
(python = the interpreter of ~/.concertino/config.json "python"; ~ = the user's home, $HOME in PowerShell;
paths without a root are relative to this repo's root)
1. `python -X utf8 S:/Projects/concertino/skills/concertino/tools/claude_or.py handoff accept docs/plans/current/HANDOFF.md`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`S:/Projects/concertino/skills/concertino/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline ccda505a8d84..HEAD`.
4. Adopt the children in flight: check their process and output and watch it; never relaunch a live ID.
5. One watcher per director, no shell loops: start `S:/Projects/concertino/skills/concertino/tools/vigia/wsvigia.py` detached with
   `S:/Projects/concertino/skills/concertino/tools/lanza/lanza.py` and subscribe to it (in Claude Code, `Monitor` on its `ws://` URL; its 30-minute
   expiry doubles as the cache keep-alive, `references/handoff.md`). Agents without a monitor check it at each turn.
6. Search the project memory (CONCERTINO.md "Identity") for the keywords of the next step.
7. Questions to the user (`S:/Projects/concertino/skills/concertino/references/questions.md`): read `docs/plans/current/QUESTIONS.md`; never ask again
   what is `answered`, and ask production gates yourself when they become executable. If it does not exist, create it
   from `S:/Projects/concertino/skills/concertino/templates/docs/plans/QUESTIONS.md` with the open questions of this HANDOFF (and the live ones of
   `docs/plans/QUESTIONS.md`, if any), checked first against EXECUTION and `git log --since=<HANDOFF date>`.
8. Inbox: read `docs/plans/INBOX.md` (`S:/Projects/concertino/skills/concertino/references/cross-project.md`); apply each new entry in this project,
   record it, answer if expected and mark it `→ done | accepted | rejected`.
9. Commit the HANDOFF with the acceptance and continue with "Next step".

## State
- Cerradas en total: R97-01..07, R97-09 y, en este tramo, **R97-10** (`7f252ce2`), **R97-13** (`bdbec359`) y
  **R97-14** (`acadec71`); filas en PLAN_OPS §5. R97-28 parcial (verificación final tras R97-18, PLAN_INTEGRATION §5).
  Detalle de todo en `docs/plans/current/EXECUTION.md`.
- **Tres tareas nuevas en PLAN_OPS** (huecos del plan, `bc7cf4d1`): **R97-29** lineales compuestas (split, swiglu,
  add, pair; L7; absorbe `linear/swiglu`), **R97-30** secuencia e indexado del camino MTP (L5) y **R97-31**
  `ninfer_ops` completo en `r9700` y suites oficiales (L5; tras R97-11, 12, 15, 29 y 30; antes de R97-16; resuelve
  ISSUE-005). Motivo: `ninfer_ops` entero no enlaza hoy en `r9700` (MoE, PLE, QSA, `pdl.cuh`, `mma.cuh`); cada
  familia portada se valida con una biblioteca propia (`ninfer_*_r9700`) y sus tests enlazados a ella.
- En curso: R97-08 (autor, intento 3), R97-11 (gate tras la corrección del oráculo `637377e9`), R97-12 (autor,
  intento 3; 0 de 9 suites ejecutadas hasta ahora), R97-15 (gate relanzado), R97-29 (oráculo), R97-30 (autor,
  intento 2).
- Pendientes: autor de R97-29 (tras su oráculo), R97-31, INTEGRATION R97-16..18, verificación final de R97-28.
- Issues: ISSUE-001 (nvtx3 → R97-08), ISSUE-002 (borrar `.agent-tmp/R97-03/` tras aceptar R97-08; ya copiado a
  `G:\vcpkg_root\installed-ninfer-r9700`), ISSUE-003 (backlog), **ISSUE-004** (atención con más de 128 consultas y
  contexto largo: un solo lanzamiento, riesgo de TDR sin medir; lo mide R97-16), **ISSUE-005** (suite completa de
  `ctest` en `r9700`; R97-31).
- Cola de compilación: sigue siendo el freno (2 ranuras para todos los proyectos, un candado por checkout). Q-011
  respondida: orden de llegada. Cuando ninfer tiene ranura pasan varias entradas seguidas.
- Cuota a las 07:34 UTC: Claude 53 %, Fable 73 %, Codex 56 %, con el 4,5 % de la semana (a las 06:10 UTC: 44 %,
  60 % y 49 %). **Q-012 abierta.** Regla del perfil: Fable > 95 % → el siguiente relevo sale en `claude-opus-5-5 high`.

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-08-author3 | R97-08 | Codex gpt-6.1-sol high | lanza pid 41628 (attempt 534) | `R97-08/author3-output.md`, `lz-author3.fin` | `R97-08/card-fix-3.md` | `ctest -L host` en verde → revisión Sonnet 5.5 high (escribir la ficha con el modelo de `R97-10/review-card.md`; revisar los excluidos por causa); sin plazo → gate sin modelo con `attempt2.py build` y `ctest`; tras aceptar, borrar `.agent-tmp/R97-03/` (mirar antes qué hay) |
| r97-12-author3 | R97-12 | Codex gpt-6-astra xhigh | lanza pid 29864 (attempt 531) | `R97-12/author3-output.md`, `lz-author3.fin` | `R97-12/card-fix-3.md` | nueve suites en verde → revisión Opus 5.5 high (incluye `numeric/math.h` `e9cc0212`, la biblioteca propia y el `constexpr`→`const` de dos tests); sin plazo → gate sin modelo con `.agent-tmp/R97-12/run-gate.py build` y `gpu` |
| r97-30-author2 | R97-30 | Codex gpt-6.1-sol high | lanza pid 7212 (attempt 535) | `R97-30/author2-output.md`, `lz-author2.fin` | `R97-30/card-fix.md` (+ `card.md`) | dos suites en verde → revisión Sonnet 5.5 high (independencia de las suites, `__grid_constant__` en `cuda_compat.h`, `elementwise/sources.cmake`); sin plazo → gate sin modelo con `.agent-tmp/R97-30/run-gate.py` |
| r97-29-oracle | R97-29 | Claude Opus 5.5 high | lanza pid 11052 (attempt 532) | `R97-29/oracle-events.jsonl` (evento `type=result`), `lz-oracle.fin` | `R97-29/oracle-card.md` | contrastar el informe; escribir la ficha del autor (modelo `R97-14/card.md`; Codex gpt-6-astra xhigh; biblioteca propia en `r9700`; epílogo de `rowsplit.cuh:73`; formas reales del 27B según el informe; `run-gate.py` sin plazos), pre-revisarla y lanzarla; `attempt end 532` |
| r97-15-gate | R97-15 | sin modelo (cola) | lanza pid 34448, etiqueta `R97-15-gate` | `.agent-tmp/R97-15/gate.log`, `gpu-run{1,2}.log`; `R97-15/lz-gate2.fin` | `R97-15/card.md` | comprobar que `gate.log` no dice `NOT RUN` (plazo ampliado con `R9715_DEADLINE_UTC`; si la variable no llegó, relanzar poniéndola dentro de la orden); exit 0 (79/79, dos pasadas) → lanzar `R97-15/review-card.md` (lista y pre-revisada; Sonnet 5.5 high); exit ≠ 0 → intento 2 del autor |
| r97-11-gate | R97-11 | sin modelo (cola) | lanza pid 37668, etiqueta `R97-11-gate` | `.agent-tmp/R97-11/gate.log`; `R97-11/lz-gate2.fin` | `R97-11/card.md` + `card-fix.md` | exit 0 (normalización 447/447) → lanzar `R97-11/review-card.md` (lista; Sonnet 5.5 high; incluye la corrección del oráculo `637377e9` y el uso de sondas); exit ≠ 0 → intento 3 del autor con el log |

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia pid 37076 en `ws://127.0.0.1:8791/`, lista
`.git/concertino/vigia/fins.txt` (al lanzar algo, sustituir o añadir su `.fin` ahí). Lanzamiento de hijos: `lanza.py`
+ `hijo.py` (órdenes exactas en cualquier `lz-*-events.jsonl`: Claude con `--claude -- claude -p --model ...`, Codex
con `-- codex exec -m ... -o <salida>`). Gates sin modelo: `lanza.py` + `build_queue.py run` +
`.agent-tmp/director/gate.py <log> <orden> [-- <orden>]`. Intentos: `scripts/dashboard.py attempt start|end`. Cierre
de una tarea: fila en PLAN_OPS §5, entrada en EXECUTION, `STATUS.md`, `attempt end`, `task --status closed`,
`update`, commit y `git push fork fork_R9700`. Pre-revisión de fichas: `tools/claude_agy.py run --model
gemini-3.8-flash-high --mode plan --prompt-file <prefijo de R97-14/prereview-prompt.md + ficha>`.
Trampas: texto Markdown solo con Write/Edit; órdenes de shell solo en ASCII y cortas (las largas no caben en
Windows); `cd` persiste entre órdenes de Bash; los scripts de validación de los autores pueden llevar un plazo dentro
(R97-15: la validación salió con 124 sin ejecutar nada); los errores `HTTP 403 ... rmcp` en el stderr de Codex son de
un MCP y no afectan. Sin compromisos abiertos con otros directores.

## Decisions
- Usuario (vigentes, EXECUTION y LAUNCH): GPU ASUS (HIP 0) dedicada; Codex implementa y Claude orquesta y revisa
  (Q-010); cupo dinámico; push a `fork` (`git push fork fork_R9700`) tras cada commit de registro, hijos sin push;
  Q-008 (las dos R9700 para ninfer en SPRINT-02; avisar a los demás proyectos antes de R97-22). **Q-011 respondida**
  en este tramo: «Seguir por orden de llegada (Recomendado)» (`85a498d8`).
- Director, este tramo (todas en EXECUTION):
  - R97-29, R97-30 y R97-31 añadidas a PLAN_OPS como trabajo necesario del objetivo (análisis de uso real del
    camino de texto del 27B con MTP; `bc7cf4d1`).
  - R97-10, R97-13 y R97-14 cerradas con condiciones que no piden cambiar código: ISSUE-004, ISSUE-005, y los
    casos nuevos de los oráculos sin ejecutar en CUDA (esta máquina no puede; pendiente anotado en PLAN_OPS §5).
  - R97-11: los tres casos de RMSNorm que fallaban tenían una cota inalcanzable; la corrigió el oráculo
    (`637377e9`), no el autor. Su validación usa sondas porque `ninfer_ops` no enlaza; va a la revisión.
  - R97-12: biblioteca propia de la familia en `r9700` y `constexpr`→`const` en tres líneas de dos tests.
  - R97-30: sin oráculo propio (transformaciones exactas); `elementwise/position.cu` y el cambio mínimo de
    `elementwise/sources.cmake` pasan a sus rutas; `__grid_constant__` vacío en `core/cuda_compat.h`.
  - R97-08: los dos tests host que dependen del motor se excluyen de `r9700` con su motivo.

## Open questions to the user
- Q-012 (decision, general): qué hace ninfer cuando la cuota semanal de Claude se acerque al final (revisores,
  oráculos y director son Claude). No bloquea todavía; si no se decide, se aplica la opción A.

## Promoted knowledge
Nada nuevo en la memoria: lo aprendido en este tramo es del sprint y quedó versionado (EXECUTION, PLAN_OPS §3 y §5,
ISSUE-004, ISSUE-005, QUESTIONS). Sigue vigente la memoria del proyecto `model-less-gates`.

## Next step
Adoptar los seis procesos (comprobar pids y `.fin`), armar `Monitor` sobre `ws://127.0.0.1:8791/` y atender el
primero que termine. Lo más probable es el oráculo de R97-29: contrastar su informe y lanzar al autor de R97-29. Las
fichas de revisión de R97-11 y R97-15 ya están escritas: se lanzan en cuanto su gate salga con 0. Antes de cada
revisión con Claude, leer la cuota y mirar si Q-012 tiene respuesta.
