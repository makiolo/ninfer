# Director HANDOFF — handoff 1

## Handoff
- Number: 1
- Outgoing: session fa05f620-2a38-4a05-bf82-38eff79c76ab, claude-opus-5-5, context 56.2 %
- Written: 2026-11-17T00:24:38+01:00
- HEAD at writing: 92cb69e21e7c7d588226bf902db1f70e49d5e394
- LAUNCH: docs/plans/current/LAUNCH.md · coordinator block: «Director prompt» (sección final de LAUNCH.md)
- Previous handoffs: `git log --follow -- docs/plans/current/HANDOFF.md`
- Accepted: session 58ebdb7d-a0db-4a15-b364-f77744ee186c, 2026-11-17T00:41:30+01:00, initial context 21.2 %

## Boot
0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
1. `D:/miniconda312/python.exe -X utf8 S:/Projects/concertino/skills/concertino/tools/claude_or.py handoff accept docs/plans/current/HANDOFF.md`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`S:/Projects/concertino/skills/concertino/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 92cb69e21e7c..HEAD`.
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
- Fase: PLAN_BUILD **cerrado** (hito 1: `r9700-ops` configura y el smoke de GPU pasa en la ASUS). HOST en curso; OPS
  e INTEGRATION sin empezar. Detalle en `docs/plans/current/EXECUTION.md` y registro de cierre de `PLAN_BUILD.md`.
- Cerradas: R97-01 (`92958cf5`, `d3302962`), R97-03 (`95e6402e`, `21297e2d`), R97-02 (oráculo `9ce05e73`, `62175136`,
  `36f36b4a`), R97-04 (`8a29f812`, `de5cce05`). Registros de cierre `222b306d`, `dc7429d2`, `a718ec76`, `92cb69e2`.
- Cerrada también R97-05 (`6b075a37`, `168398d7`; registro `a80fbc9d`).
- En revisión: R97-06 (autor terminado, intento 499 → commits `761d4440` CPU, `a0ade27f` logging, `92ae1c50` señales;
  revisión Sonnet 5.5 high = intento 501). El autor avisa: «falta JPEG al configurar» y `cpu_tensor` choca con
  `nvtx3/nvToolsExt.h` (ISSUE-001).
- Issue abierto: ISSUE-001 (`ninfer_cpu_executor`/`CUDA::nvtx3` en `r9700`); la parte `ninfer_cpu_executor` va en la
  ficha de R97-06; nvtx3 queda para quien porte `core/nvtx.h` (R97-08 o OPS).
- Patrón observado: cada entrega de Codex sale con 1 ronda de condiciones del revisor (toolchain, alcance); el director
  contrasta cada hallazgo y lanza un «intento 2» corto con `card-fix.md`.

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| r97-06-review | R97-06 | Claude Sonnet 5.5 high | lanza pid 14960 (attempt 501) | `.git/concertino/sprint-01/R97-06/review-events.jsonl` (resultado en el evento `type=result`), `lz-review.fin` | `.git/concertino/sprint-01/R97-06/review-card.md` (+ `card.md`) | contrastar hallazgos con el código; si hay condiciones, `card-fix.md` + intento 2 en Codex (patrón de R97-05); cerrar R97-06 en PLAN_HOST/STATUS; `attempt end 499/501` |

Lanzamiento de hijos: `lanza.py` + `hijo.py` (ver cualquier `lz-*.pid`/`*-events.jsonl` en `.git/concertino/sprint-01/<ID>/`);
autor Codex: `codex exec -m gpt-6.1-sol -c model_reasoning_effort=high --dangerously-bypass-approvals-and-sandbox -C <repo> --json -o <out> -`;
revisor: `claude -p --model claude-sonnet-5-5 --effort high --permission-mode bypassPermissions --output-format stream-json --verbose` con `hijo.py --claude`.
Pre-revisión de fichas: `claude-agy run --model gemini-3.8-flash-high --prompt-file <prompt>` (todas READY hasta ahora).

## Decisions
- Usuario (en sesión, 2026-11-16 ~22:05 UTC): el anuncio de congelación de la cartera no aplica a ninfer; el sprint
  continúa (EXECUTION.md).
- Director: R97-01 acepta «configura» parcial hasta R97-03 (PLAN_BUILD cierre).
- Director: hueco del plan entre BUILD y OPS: R97-03 (subida a L4) crea 16 `src/ops/**/r9700/sources.cmake` vacíos y
  mapea `CUDA::*`; **cada tarea de OPS rellena su manifiesto** (EXECUTION.md, PLAN_BUILD cierre).
- Director: R97-04 enlaza las fuentes de dispositivo de `core` en el smoke; el enlace de `ninfer_core` completo se
  verifica en R97-08 (PLAN_BUILD cierre).
- Director: toolchain `r9700` común en `cmake/NinferPlatform.cmake` (compiler-rt builtins, sin escaneo de módulos,
  reenvío generado de `<cuda_*.h>` a `core/cuda_compat.h`); cabeceras sin reenviar listadas en el cierre de R97-04 y en
  `.agent-tmp/R97-04/fix/cuda-includes.json`: son trabajo de las tareas de OPS dueñas.
- Para PLAN_OPS (EXECUTION.md): APIs PDL/cluster sin mapear, máscara de 32 bits en `normalization`, hipCUB en
  `sampling`.

## Open questions to the user
- Q-008 (decision, R97-22/R97-24..27, SPRINT-02): ¿se libera también la segunda GPU para tensor parallel? No bloquea SPRINT-01.

## Promoted knowledge
Nada nuevo en memoria: las decisiones y trampas de toolchain quedaron versionadas en el cierre de PLAN_BUILD, EXECUTION.md e ISSUE-001.

## Next step
Adoptar la revisión de R97-06 (intento 501) y cerrar R97-06. Después, con la cuota de
Claude reiniciada: lanzar R97-07 (servidor HTTP/Winsock y `media_acquire`; la parte de señales de `apps/serve/main.cpp`
es de R97-06) y empezar OPS por el **oráculo de R97-09** (Claude) antes de su autor Codex. Antes de cada lanzamiento,
`claude-or quota --weekly`: Codex estaba al 96 % (reinicio semanal 21-11; el usuario quiere hacer un reset de Codex
tras el reinicio de Claude, lo recuerda la raíz de quantdesk-backend). Si Codex se agota: parar implementación,
PushNotification y preguntar (LAUNCH).
