# Director HANDOFF — handoff 2

## Handoff
- Number: 2
- Outgoing: session 58ebdb7d-a0db-4a15-b364-f77744ee186c, claude-opus-5-5, context 35.8 %
- Written: 2026-11-17T01:06:56+01:00
- HEAD at writing: 39a3d3a2f75ba3f539ea0520fb2f56cdf9d9b56e
- LAUNCH: docs/plans/current/LAUNCH.md · coordinator block: «Director prompt» (sección final de LAUNCH.md)
- Previous handoffs: `git log --follow -- docs/plans/current/HANDOFF.md`
- Accepted: session 7465607f-8fd1-4ee1-bd16-b45d644937e5, 2026-11-17T01:12:13+01:00, initial context 33.0 %

## Boot
0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
1. `D:/miniconda312/python.exe -X utf8 S:/Projects/concertino/skills/concertino/tools/claude_or.py handoff accept docs/plans/current/HANDOFF.md`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`S:/Projects/concertino/skills/concertino/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 39a3d3a2f75b..HEAD`.
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
- Recuperación en frío tras la caída de la sesión 58ebdb7d (`.git/concertino/handoff/RECOVERY-20261117-010628.md`);
  ningún hijo perdido.
- Fase: PLAN_BUILD cerrado (hito 1). HOST: R97-05 (`168398d7`) y R97-06 (`92ae1c50`, registro `ba400e75`) cerradas;
  R97-07 entregada por Codex (`535693be`, intento 502, «Doubts: none»; `r9700-dev` configura con JPEG/FFmpeg/curl,
  compuerta host 3/3, enlace completo bloqueado por nvtx3 = ISSUE-001) y en revisión. OPS: oráculo de R97-09 en curso
  (WIP sin commit en `tests/ops/common/`). INTEGRATION sin empezar. Detalle en `docs/plans/current/EXECUTION.md`.
- Patrón: cada entrega de Codex sale con 1 ronda de condiciones del revisor; el director contrasta cada hallazgo y lanza
  un «intento 2» corto con `card-fix.md`.

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-10-author | R97-10 | Codex gpt-6-astra xhigh | lanza pid 22300 (attempt 513; oráculo `0ab0acdc`) | `.git/concertino/sprint-01/R97-10/author-events.jsonl`, `author-output.md`, `lz-author.fin` | `.git/concertino/sprint-01/R97-10/card.md` | revisión Opus 5.5 high (L7); después asignar `linear/swiglu` |
| r97-11-author | R97-11 | Codex gpt-6.1-sol high | lanza pid 24304 (attempt 514; oráculo `778d8840`, `3c796e59`) | `.git/concertino/sprint-01/R97-11/author-events.jsonl`, `author-output.md`, `lz-author.fin` | `.git/concertino/sprint-01/R97-11/card.md` | revisión Sonnet 5.5 high |
| r97-12-oracle | R97-12 | Claude Opus 5.5 high | lanza pid 39584 (attempt 515) | `.git/concertino/sprint-01/R97-12/oracle-events.jsonl`, `lz-oracle.fin` | `.git/concertino/sprint-01/R97-12/oracle-card.md` | revisar; ficha del autor Codex `gpt-6-astra` xhigh |
| r97-13-author | R97-13 | Codex gpt-6-astra xhigh | lanza pid 26548 (attempt 518; oráculo `b5117b4b`) | `.git/concertino/sprint-01/R97-13/author-events.jsonl`, `author-output.md`, `lz-author.fin` | `.git/concertino/sprint-01/R97-13/card.md` (dueña de los kernels de escritura KV bf16/fp8) | revisión Opus 5.5 high (L7) |
| r97-14-oracle | R97-14 | Claude Opus 5.5 high | lanza pid 8584 (attempt 517) | `.git/concertino/sprint-01/R97-14/oracle-events.jsonl`, `lz-oracle.fin` | `.git/concertino/sprint-01/R97-14/oracle-card.md` | revisar; ficha del autor Codex `gpt-6-astra` xhigh |
| r97-08-author | R97-08 | Codex gpt-6.1-sol high | lanza pid 25796 (attempt 511) | `.git/concertino/sprint-01/R97-08/author-events.jsonl`, `author-output.md`, `lz-author.fin` | `.git/concertino/sprint-01/R97-08/card.md` | revisión Sonnet 5.5 high; borrar `.agent-tmp/R97-03/` tras aceptar ISSUE-002 |

(Usuario 01:xx UTC: «Lanza con Codex, quiero gastarlo». R97-09 cerrada en `dec8d874`. Las suites de Ops ya tienen referencia FP64 independiente: en R97-10..15 el oráculo
audita esa referencia y añade casos de riesgo `r9700`, no reescribe. Codex al 97 %: si se agota, parar y preguntar.)

(Actualizado 00:3x UTC: R97-07 cerrada, oráculo de R97-09 aceptado.)

Vigía vivo: wsvigia pid 37076 en `ws://127.0.0.1:8791/`, lista `.git/concertino/vigia/fins.txt`.
Lanzamiento de hijos: `lanza.py` + `hijo.py` (ver `lz-*-events.jsonl` en `.git/concertino/sprint-01/<ID>/`);
autor Codex: `codex exec -m gpt-6.1-sol -c model_reasoning_effort=high --dangerously-bypass-approvals-and-sandbox -C <repo> --json -o <out> -`;
revisor: `claude -p --model claude-sonnet-5-5 --effort high --permission-mode bypassPermissions --output-format stream-json --verbose` con `hijo.py --claude`.
Intentos: `S:/Projects/concertino/skills/concertino/scripts/dashboard.py attempt start|end`.
Trampa: en esta máquina la herramienta Bash expande comillas invertidas dentro de heredocs aunque el delimitador vaya
entre comillas; escribir ficheros con texto Markdown solo con Write/Edit.

## Decisions
- Usuario (2026-11-17 ~02:00 UTC): subir todo a su fork. Remoto `fork` (`git@github.com:makiolo/ninfer.git`, por SSH),
  rama `fork_R9700`; el director hace `git push fork fork_R9700` tras cada commit de registro. Hijos: sin push.
- Usuario (2026-11-17 ~00:00 UTC): ninfer no hace la revisión de alertas de 8 h del perfil (EXECUTION.md y memoria).
- Usuario (2026-11-16 ~22:05 UTC): la congelación de la cartera no aplica a ninfer (EXECUTION.md; INBOX → rejected).
- Decisiones de BUILD/HOST anteriores: cierre de PLAN_BUILD, EXECUTION.md e ISSUE-001.

## Open questions to the user
- Q-008 (decision, R97-22/R97-24..27, SPRINT-02): ¿se libera también la segunda GPU para tensor parallel? No bloquea SPRINT-01.

## Promoted knowledge
Nada nuevo en memoria: la trampa de heredocs queda en este HANDOFF y en EXECUTION.md.

## Next step
Esperar al vigía: revisión de R97-07 (504) → contrastar y cerrar R97-07 (o intento 2); oráculo de R97-09 (503) →
revisarlo y lanzar el autor Codex de R97-09 tras `claude-or quota --weekly` (Codex al 96 %, reinicio 21-11 11:38; si se
agota: parar implementación, PushNotification y preguntar). El usuario quiere hacer un reset de Codex tras el reinicio
de Claude (lo recuerda la raíz de quantdesk-backend).
