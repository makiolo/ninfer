# Director HANDOFF — handoff 3

## Handoff
- Number: 3
- Outgoing: session 7465607f-8fd1-4ee1-bd16-b45d644937e5, claude-opus-5-5, context 70.9 %
- Written: 2026-11-17T04:11:24+01:00
- HEAD at writing: 8feb44d136e1a6bbc39ebc9daa3e7c876e6e0bbb
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
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 8feb44d136e1..HEAD`.
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
- Cerradas en total: R97-01..07 y R97-09 (en este tramo: R97-07 `535693be`, registro `c7200927`; R97-09 `dec8d874`,
  registro `e4486663`). Registros en PLAN_HOST/PLAN_OPS §5; detalle en `docs/plans/current/EXECUTION.md`.
- Oráculos aceptados (Claude, solo tests nuevos): R97-10 `0ab0acdc`, R97-11 `778d8840`+`3c796e59`, R97-12 `96d31157`,
  R97-13 `b5117b4b`, R97-14 `d6fe1304`. En R97-10..15 el oráculo audita la referencia FP64 existente de las suites y
  añade casos de riesgo `r9700`; no reescribe.
- En curso: 6 autores Codex (R97-08, 10, 11, 12, 13, 14) y R97-28 (docs, Claude). Pendientes: R97-15 (sampling/MTP,
  tras R97-10..14), `linear/swiglu` (sacada de R97-11; asignar al cerrar R97-10), INTEGRATION R97-16..18.
- Issues: ISSUE-001 (parte nvtx3 → R97-08), ISSUE-002 (árbol vcpkg en `.agent-tmp/R97-03`, → R97-08; borrar ese árbol
  tras aceptar), ISSUE-003 (media_acquire, backlog).
- Cuello de botella: la cola de compilación (un candado por checkout, 2 ranuras globales, 4 núcleos). Las fichas
  nuevas obligan a compilar solo objetivos propios; R97-08/10/11 se lanzaron sin esa regla.

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-08-author | R97-08 | Codex gpt-6.1-sol high | lanza pid 25796 (attempt 511) | `.git/concertino/sprint-01/R97-08/author-output.md`, `lz-author.fin` | `R97-08/card.md` | revisión Sonnet 5.5 high; si ISSUE-002 queda resuelto, borrar `.agent-tmp/R97-03/` |
| r97-10-author | R97-10 | Codex gpt-6-astra xhigh | lanza pid 22300 (attempt 513) | `R97-10/author-output.md`, `lz-author.fin` | `R97-10/card.md` | revisión Opus 5.5 high (L7); luego asignar `linear/swiglu` |
| r97-11-author | R97-11 | Codex gpt-6.1-sol high | lanza pid 24304 (attempt 514) | `R97-11/author-output.md`, `lz-author.fin` | `R97-11/card.md` | revisión Sonnet 5.5 high |
| r97-12-author | R97-12 | Codex gpt-6-astra xhigh | lanza pid 30760 (attempt 519) | `R97-12/author-output.md`, `lz-author.fin` | `R97-12/card.md` | revisión Opus 5.5 high |
| r97-13-author | R97-13 | Codex gpt-6-astra xhigh | lanza pid 26548 (attempt 518) | `R97-13/author-output.md`, `lz-author.fin` | `R97-13/card.md` (dueña de kernels de escritura KV y de `cudaMemcpy2D`/`Default`) | revisión Opus 5.5 high |
| r97-14-author | R97-14 | Codex gpt-6-astra xhigh | lanza pid 19288 (attempt 520) | `R97-14/author-output.md`, `lz-author.fin` | `R97-14/card.md` | revisión Opus 5.5 high |
| r97-28-author | R97-28 | Claude Sonnet 5.5 high | lanza pid 33356 (attempt 521) | `R97-28/author-events.jsonl`, `lz-author.fin` | `R97-28/card.md` | el director contrasta cada afirmación con el alcance; verificación final tras R97-18 |

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia pid 37076 en `ws://127.0.0.1:8791/`, lista
`.git/concertino/vigia/fins.txt`. Lanzamiento: `lanza.py` + `hijo.py` (ver cualquier `lz-*-events.jsonl`); revisores
con `hijo.py --claude` y `claude -p --model ... --effort high --permission-mode bypassPermissions --output-format
stream-json --verbose`; intentos con `S:/Projects/concertino/skills/concertino/scripts/dashboard.py attempt start|end`
(resultado `accepted|rejected|failed|blocked`). Pre-revisión de fichas: `tools/claude_agy.py run --model
gemini-3.8-flash-high`. Trampas: texto Markdown solo con Write/Edit; órdenes de shell solo en ASCII (memoria
`bash-heredoc-backticks`). Resultado de un hijo Claude: evento `type=result` de su `*-events.jsonl`.

## Decisions
- Usuario: GPU ASUS (HIP 0) reservada; aviso enviado a los 8 INBOX (EXECUTION). «Lanza con Codex, quiero gastarlo»;
  reset de Codex hecho (0 %, reinicio 24-11). Cupo **dinámico** (LAUNCH.md). Subir todo al fork: remoto `fork`
  (`git@github.com:makiolo/ninfer.git`, SSH), rama `fork_R9700`; el director hace `git push fork fork_R9700` tras cada
  commit de registro; hijos sin push. R9700 en README/bench/tests (`8a792933`) y tarea R97-28 «asumiendo el sprint
  implementado» (PLAN_INTEGRATION). Objetivo: Qwen3.8-27B (arquitectura `Qwen3_5ForCausalLM`, `src/models/qwen3_5`).
- Director (EXECUTION): oráculos de Ops auditan + amplían; `gated_residual`, drafter DFlash, Vision fuera de R97-11..14;
  R97-13 porta los kernels de escritura KV que consume R97-14; `gdn_input_proj_conv_*`/`gdn_control` en R97-12;
  NaN y |x|>1,8e19 fuera de contrato; ninfer no hace revisión de alertas.

## Open questions to the user
- Q-008 (decision, R97-22/R97-24..27, SPRINT-02): ¿se libera también la segunda GPU para tensor parallel? No bloquea SPRINT-01.

## Promoted knowledge
Memoria del proyecto: `bash-heredoc-backticks` (comillas invertidas en heredocs y órdenes no ASCII rompen la
herramienta Bash). Lo demás quedó versionado en EXECUTION, PLAN_* §5 e issues.

## Next step
Adoptar los 7 hijos (comprobar sus pids y `lz-*.fin`), armar `Monitor` sobre `ws://127.0.0.1:8791/` y, en cuanto
termine cada autor, leer su informe, escribir su `review-card.md` (modelo de las de R97-07/R97-09) y lanzar la revisión
(L6 Sonnet 5.5 high, L7 Opus 5.5 high); contrastar hallazgos y cerrar o lanzar intento corto con `card-fix.md`.
