# Director HANDOFF — handoff 8

## Handoff
- Number: 8
- Outgoing: session 7549b0d9-0dc0-410d-b73e-d50809c998d9, claude-opus-5-5, context 71.0 %
- Written: 2026-11-19T06:01:43+01:00
- HEAD at writing: 6eb9f0773af9a7ce0821e138cba083c0f5853b75
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
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 6eb9f0773af9..HEAD`.
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
- **Hito alcanzado:** Qwen3.8-27B carga y genera texto en la ASUS (R97-17). Cerradas en este tramo, todas con
  revisión ACCEPT y fila en su §5: **R97-31** (`5af7273c` + limpieza `1ecb7417`; PLAN_OPS §5), **R97-33** (`448da3c4`,
  nueva: rechazo explícito de Ops de familias no portadas para que enlace el motor), **R97-16** (`48c75643`, `8ec0e337`,
  `4ef80b7e`, test del director `3f698a4c`), **R97-17** (`53484e6c`); filas en PLAN_INTEGRATION §5. ISSUE-002 cerrada;
  ISSUE-004 criterio 1 marcado; ISSUE-006 ampliada (tests ELF de R97-17); **ISSUE-007** nueva (FFmpeg sin PNG, backlog).
- **En curso:** R97-18 (oráculo y autor en paralelo, ver «In flight»). Pendientes: R97-32 (`claude-local`, tras R97-18),
  verificación final de R97-28 (tras R97-18). R97-19..21 (PERF) son de SPRINT-02.
- Todo el detalle en `docs/plans/current/EXECUTION.md` (entradas del 2026-11-19).
- **Cuota a las 05:00 UTC:** Claude 72 %, Fable 92 %, **Codex 84 %** con el 30 % de la semana (gasta deprisa; reinicio
  2026-11-24 02:09). Anuncio del usuario vigente: preferencia a Codex hasta el reinicio de Claude (2026-11-23 23:59) y
  Claude en proporción (~6 puntos/día). Si Codex pasa del ~95 %, gastar el reinicio guardado de Codex o cambiar de cuenta
  es cosa del usuario: pregunta **bloqueante** (gasta cuota) por la regla nueva de preguntas.
- Clave de API: `ANTHROPIC_API_KEY` está en el entorno de usuario de Windows; esta sesión no la tenía. Hijos de Claude
  con `env -u ANTHROPIC_API_KEY` (memoria `anthropic-api-key-env`).

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| oracle | R97-18 | gpt-6-astra xhigh (653) | lanza pid 28228, `R97-18/lz-oracle` | `R97-18/oracle-output.md`; `.agent-tmp/R97-18-oracle/` | `R97-18/oracle-card.md` | `reference-results.json` + `criterion.md` + `compare.py`; revisar que la cota sea viable con `bf16-ideal` (memoria `oracle-bf16-bound-feasibility`) |
| author | R97-18 | gpt-6.1-sol xhigh (654) | lanza pid 31772, `R97-18/lz-author` | `R97-18/author-output.md`; `.agent-tmp/R97-18/` | `R97-18/card.md` | PPL quick BF16/FP8, CLI MTP+FP8, tests con artefacto, `ctest` completo |

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia (lanza pid 7696) en `ws://127.0.0.1:8791/`; `fins.txt` =
`R97-18/lz-author.fin` y `R97-18/lz-oracle.fin`. Órdenes de lanzamiento: cualquier `lz-*-events.jsonl` (Codex:
`hijo.py ... -- codex exec -m <modelo> -c model_reasoning_effort=<e> ... -o <salida> -`). Gates sin modelo: `lanza.py` +
`build_queue.py run` + `.agent-tmp/director/gate.py <log> <orden> [-- <orden>]`; ejemplos en
`.agent-tmp/director/r97-16-33-gate.py` y en `R97-17/lz-gate-events.jsonl` (pasa `--targets` explícitos: los tests ELF
no compilan en Windows). Tablero: `S:/Projects/concertino/skills/concertino/scripts/dashboard.py`
(`attempt start|end`, `task --status`, `update --repo .`); los intentos 653/654 están abiertos. Pre-revisión de fichas:
`claude_agy.py run --model gemini-3.8-flash-high --mode plan` con las tres primeras líneas de
`R97-14/prereview-prompt.md` + ficha. Trampas: las órdenes de Bash con caracteres no ASCII (tildes, flechas, comillas
angulares) fallan con exit 127, así que se edita con Write/Edit o con un script `.py`; no hacer commit mientras corre un gate que
toma huella de HEAD; los avisos de vigías caducados repiten eventos viejos (mira el `.fin` y el log antes de actuar).
Sin compromisos abiertos con otros directores (reinforcement_learning y deeplearning_dev respetan la ASUS).

## Decisions
- Usuario, este tramo (EXECUTION): seguir con la suscripción, sin clave de API; anuncio de preferencia a Codex
  (INBOX 02:36, aceptado); **sin benchmarks ni comparaciones hasta optimizar** (memoria
  `no-benchmarks-before-optimizing`; aplicado en R97-17/18/32); preguntas bloqueantes u opcionales (INBOX 03:59);
  **R97-32**: `claude-local` con ninfer en **las dos GPU** (ASUS y Sapphire), elegir la GPU eligiendo el modelo desde
  Claude Code, órdenes para cargar y descargar (PLAN_INTEGRATION §3).
- Director (EXECUTION): R97-31 condiciones (a→R97-16, b hecha, c decidida); R97-33 creada (patrón de rechazo de R97-31,
  sin excluir familias en el registro); `state_store` era carrera del test (`3f698a4c`); TDR, nueve combinaciones sin
  lanzar; `ninfer_glm5next_protocol_test` a ISSUE-007; reparto de los tests de R97-17 (FP8/MTP y `loading_real` a R97-18,
  ELF a ISSUE-006); las Ops rechazadas del drafter son solo de DFlash (MTP no queda bloqueado); referencia de PPL
  independiente en CPU porque no hay valor publicado para `groupwise-int` ni hardware CUDA.

## Open questions to the user
none

## Promoted knowledge
Memoria del proyecto: `anthropic-api-key-env` y `no-benchmarks-before-optimizing` (nuevas). Lo demás quedó en
EXECUTION, PLAN_OPS/PLAN_INTEGRATION §3 y §5, e ISSUE-002/004/006/007.

## Next step
Atender los `.fin` de R97-18. Oráculo: revisar `criterion.md` (cota viable con `bf16-ideal`). Autor: gate sin modelo del
director (build, `ppl`, `cli`, `model`, `full`) y **comparación con el oráculo**:
`run-gate.py ppl-text .agent-tmp/R97-18-oracle/windows/<dominio>.txt bf16|fp8` + `compare.py`. Después, una revisión
Codex de otro modelo que el autor (`gpt-6-astra` si no lo usó el oráculo para revisarse a sí mismo; si no, `gpt-5.6-terra`
xhigh), cierre de R97-18, verificación final de R97-28 y ficha de **R97-32** (`claude-local`).
