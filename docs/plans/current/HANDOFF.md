# Director HANDOFF — handoff 4

## Handoff
- Number: 4
- Outgoing: session 8dd8dd4a-994d-4e58-af0e-0410fe296001, claude-fable-5-2, context 68.6 %
- Written: 2026-11-17T07:01:08+01:00
- HEAD at writing: 016b61b52bb52719c2aa1870fd9f8b6ddb8b081d
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
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 016b61b52bb5..HEAD`.
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
- Cerradas en total: R97-01..07 y R97-09 (ninguna nueva en este tramo). R97-28 parcial (`68f00ba3` + `79321953`;
  verificación final tras R97-18, lista en PLAN_INTEGRATION §5). Detalle de todo en `docs/plans/current/EXECUTION.md`.
- Entregada y en revisión: **R97-14** (`d2008b61`, `acadec71`; 172 casos en GPU, 0 omitidos; Hadamard exacta).
- Validación en cola sin modelo (el autor ya terminó): **R97-11** (`21cf7c6d`, `de3aa33f` = `math.cuh`; 0 suites en
  GPU todavía), **R97-13** (`f061bd98`, `bf4d695c`, `bdbec359`; FP8 198/198, BF16 oficial 136/180 por plazo, con
  arnés de diagnóstico 180/180), **R97-15** (`a35c3613`; 76/79 en GPU; los 3 fallos eran del test del oráculo,
  corregido por el director en `b86077c3`, sin ejecutar aún).
- Intento 2 de autor en curso: **R97-08** (parches de texto/Jinja autorizados; `e9f25fff`, `69e094d3`, `034bc7f9`),
  **R97-10** (solo aceptación numérica; compila 72/72, 0 casos numéricos), **R97-12** (`numeric/math.h` autorizado;
  `e9cc0212`).
- Pendientes: `linear/swiglu` (asignar al cerrar R97-10), INTEGRATION R97-16..18, verificación final de R97-28.
- Issues: ISSUE-001 (nvtx3 → R97-08), ISSUE-002 (árbol vcpkg: ya copiado a `G:\vcpkg_root\installed-ninfer-r9700`;
  borrar `.agent-tmp/R97-03/` tras aceptar R97-08), ISSUE-003 (media_acquire, backlog).
- **Cuello de botella: la cola de compilación** (2 ranuras para todos los proyectos, un candado por checkout). Un
  turno tarda 45-120 min. Por eso las validaciones van como entradas sin modelo (memoria `model-less-gates`).
- Cuota a las 06:01 UTC: Claude 43 %, Fable 58 %, Codex 48 % (con el 2,3-3,6 % de la semana: la cartera entera gasta
  rápido; Fable ≥ 85 % o Codex agotado piden decisión según el perfil y Q-010).
- Máquina: reinicio del driver de AMD (evento Display 4101, 03:55:27 UTC) durante un lote FP8 de R97-13, no repetido;
  RAM libre llegó a 9 MB durante un BF16 de diagnóstico (EXECUTION).

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-14-review | R97-14 | Claude Opus 5.5 high | lanza pid 39252 (attempt 528) | `R97-14/review-events.jsonl` (evento `type=result`), `lz-review.fin` | `R97-14/review-card.md` | contrastar cada hallazgo con el código; cerrar (registro en PLAN_OPS §5, `attempt end 528`, tablero) o intento corto con `card-fix.md` |
| r97-08-author2 | R97-08 | Codex gpt-6.1-sol high | lanza pid 28008 (attempt 525) | `R97-08/author2-output.md`, `lz-author2.fin` | `R97-08/card-fix.md` (+ `card.md`) | revisión Sonnet 5.5 high; revisar los 135 excluidos por causa; después borrar `.agent-tmp/R97-03/` |
| r97-10-author2 | R97-10 | Codex gpt-6-astra xhigh | lanza pid 4408 (attempt 527) | `R97-10/author2-output.md`, `lz-author2.fin` | `R97-10/card-fix.md` (+ `card.md`) | si deja `.agent-tmp/R97-10/run-acceptance.md`, lanzarla como gate sin modelo; revisión Opus 5.5 high; luego asignar `linear/swiglu` |
| r97-12-author2 | R97-12 | Codex gpt-6-astra xhigh | lanza pid 41572 (attempt 526) | `R97-12/author2-output.md`, `lz-author2.fin` | `R97-12/card-fix.md` (+ `card.md`) | revisión Opus 5.5 high (incluye `numeric/math.h`, `e9cc0212`) |
| r97-11-gate | R97-11 | sin modelo (cola) | lanza pid 44140, etiqueta `R97-11-gate` | `.agent-tmp/R97-11/gate.log`, `build-v2.log`, `math-probe.log`, `gpu-*-v2.log`; `R97-11/lz-gate.fin` | `R97-11/card.md` + `card-fix.md` | exit 0 → revisión Sonnet 5.5 high (incluye `math.cuh`, `de3aa33f`); exit ≠ 0 → intento 3 del autor con los logs (cerrar attempt 523 ya hecho: `blocked`) |
| r97-13-gate | R97-13 | sin modelo (cola) | lanza pid 15668, etiqueta `R97-13-gate` | `.agent-tmp/R97-13/gate.log`, `gpu-bf16.log`; `R97-13/lz-gate.fin` | `R97-13/card.md` | exit 0 → revisión Opus 5.5 high (atención densa, `convert.cuh`/`fp8.cuh`, traducciones de `cuda_compat.h`; reinicio de driver y RAM); exit ≠ 0 → intento 2 del autor |
| r97-15-gate | R97-15 | sin modelo (cola) | lanza pid 16140, etiqueta `R97-15-gate` | `.agent-tmp/R97-15/gate.log`, `gpu-sampling.log`, `gpu-run{1,2}.log`; `R97-15/lz-gate.fin` | `R97-15/card.md` | exit 0 (79/79, dos pasadas) → revisión Sonnet 5.5 high (incluye la corrección del director `b86077c3` al test del oráculo); exit ≠ 0 → intento 2 del autor |

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia pid 37076 en `ws://127.0.0.1:8791/`, lista
`.git/concertino/vigia/fins.txt` (al lanzar algo, sustituir o añadir su `.fin` ahí). Lanzamiento: `lanza.py` +
`hijo.py` (ver cualquier `lz-*-events.jsonl`); gates sin modelo: `lanza.py` + `build_queue.py run` +
`.agent-tmp/director/gate.py` (ver `R97-11/lz-gate-events.jsonl`). Intentos: `scripts/dashboard.py attempt
start|end` (`accepted|rejected|failed|blocked`). Pre-revisión de fichas: `tools/claude_agy.py run --model
gemini-3.8-flash-high --mode plan --prompt-file <ficha con el prefijo de R97-14/prereview-prompt.md>`. Modelo de
ficha de revisión: `R97-14/review-card.md`. Notas a autores Codex en marcha: `.agent-tmp/<ID>/DIRECTOR-NOTE.md`.
Trampas: texto Markdown solo con Write/Edit; órdenes de shell solo en ASCII; `cd` persiste entre órdenes de Bash.
Sin compromisos abiertos con otros directores.

## Decisions
- Usuario (vigentes, EXECUTION y LAUNCH): GPU ASUS (HIP 0) dedicada; Codex implementa y Claude orquesta y revisa
  (Q-010); cupo dinámico; push a `fork` (`git push fork fork_R9700`) tras cada commit de registro, hijos sin push.
  **Q-008 respondida** («Sí, exclusivas»): las dos R9700 para ninfer en R97-22 y desde R97-24; aplicada en
  `next/SPRINT.md` §4 y `next/PLAN_TP.md` §5 (`200c5a1c`); avisar a los demás proyectos antes de R97-22.
- Director, este tramo (todas en EXECUTION):
  - Reparto de cabeceras compartidas de `src/ops/common` que R97-09 no cubrió: `numeric/convert.cuh` y
    `quant/fp8.cuh` → R97-13; `numeric/math.cuh` → R97-11; `numeric/math.h` (solo la anotación de dispositivo) → R97-12.
  - Tests corregidos por el director sin cambiar casos ni esperados: `tests/ops/linear/projection/cases_bf16.cpp`
    (`std::vector<int> boundaries`, defecto del upstream; en `3402c172`) y `tests/ops/sampling/r9700_edges.cpp`
    (targets de filas greedy sin penalizaciones; `b86077c3`).
  - R97-08: autorizados `src/text/CMakeLists.txt` (`UTF8PROC_STATIC`, `if(WIN32)`) y
    `third_party/llama-jinja/jinja/value.cpp` (`localtime_s`).
  - R97-15: fuera de `r9700` la aceptación dispersa (DFlash2), `candidate_selector_path`, `linear_topk` y
    `accept_vocabulary_*`; su autor se lanzó sin esperar a R97-10..14 (rutas disjuntas).
  - Validaciones como entradas de cola sin modelo; pasada de sintaxis fuera de la cola antes de pedir turno.

## Open questions to the user
- Q-011 (decision, R97-08 y R97-10..15): ¿cómo se reparte la cola de compilación entre proyectos? No bloquea.

## Promoted knowledge
Memoria del proyecto: `model-less-gates` (validaciones sin modelo con la cola saturada y notas a los autores Codex
por `DIRECTOR-NOTE.md`). Lo demás quedó versionado en EXECUTION, PLAN_INTEGRATION §5 y QUESTIONS.

## Next step
Adoptar los 7 procesos (comprobar pids y `.fin`), armar `Monitor` sobre `ws://127.0.0.1:8791/` y atender el primero
que termine. Lo más probable es la revisión de R97-14: leer su evento `type=result`, contrastar cada hallazgo con el
código y cerrar R97-14 (fila en PLAN_OPS §5, commit de registro, push) o lanzar un intento corto. Para cada gate sin
modelo que acabe en 0, escribir su ficha de revisión con el modelo de `R97-14/review-card.md` y lanzarla.
