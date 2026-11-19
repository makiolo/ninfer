# Director HANDOFF — handoff 7

## Handoff
- Number: 7
- Outgoing: session 3391608d-8ff1-48ae-a5d3-13967de26c1e, claude-opus-5-5, context 31.0 %
- Written: 2026-11-19T02:28:32+01:00
- HEAD at writing: 5af7273c745a0df8300ed5f9c885e41c8c95cf1e
- LAUNCH: docs/plans/current/LAUNCH.md · coordinator block: «Director prompt» (sección final de LAUNCH.md)
- Previous handoffs: `git log --follow -- docs/plans/current/HANDOFF.md`
- Accepted: session 7549b0d9-0dc0-410d-b73e-d50809c998d9, 2026-11-19T02:29:23+01:00, initial context 30.8 %

## Boot
0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
(python = the interpreter of ~/.concertino/config.json "python"; ~ = the user's home, $HOME in PowerShell;
paths without a root are relative to this repo's root)
1. `python -X utf8 S:/Projects/concertino/skills/concertino/tools/claude_or.py handoff accept docs/plans/current/HANDOFF.md`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`S:/Projects/concertino/skills/concertino/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline 5af7273c745a..HEAD`.
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
- Recuperación en frío: la sesión `3391608d` murió al minuto de arrancar con «Credit balance is too low» (usaba la
  clave de API, no la suscripción). `ANTHROPIC_API_KEY` está en el entorno de usuario de Windows (HKCU); esta sesión
  no la tiene y los hijos se lanzan con `env -u ANTHROPIC_API_KEY`. El relevo 7 anterior quedó sin rellenar y se
  rehízo desde cero.
- Cerradas en total: R97-01..07, R97-09..15, R97-29, R97-30 y, en el tramo de la sesión `698c3d33`, **R97-08**
  (`1e4cff7a`, `010003c8`, ISSUE-006 al backlog) y **R97-12** (`babdb77e`; condiciones C1/C2 pasadas a R97-31).
  ISSUE-002: árbol temporal borrado (`9c58363d`); queda pendiente de verificación con la configuración limpia de
  R97-31 (punto 7 de su revisión). Detalle en `docs/plans/current/EXECUTION.md`.
- **R97-31** (`ninfer_ops` completo y suites oficiales): entrega del autor (Codex `gpt-6.1-sol` high) `5af7273c`, sin
  push. Gate sin modelo del director: exit 0 (`.agent-tmp/R97-31/gate.log`; `ctest -L ops` en la ASUS, 30 de 30
  suites, 0 fallos, 3 627 s). Revisión lanzada en este relevo (ver «In flight»). **Falta registrar en EXECUTION** el
  gate verde y el lanzamiento de la revisión.
- Pendientes sin empezar: INTEGRATION R97-16, R97-17, R97-18 y la verificación final de R97-28.
- Pesos: `out/qwen3_8_27b.ninfer` descargado y con SHA-256 comprobado (R97-17 no descarga nada).
- Cuota a las 01:28 UTC del 19-11: Claude 68 %, Fable 92 %, Codex 69 %. Fable > 85 %: los relevos salen en
  `claude-opus-5-5 high` (perfil). Claude < 85 %: revisiones y oráculos aún pueden ir a Claude (Q-012).

## In flight
| Child | ID | Model | Process or task | Output | Task card | Pending check |
|---|---|---|---|---|---|---|
| review | R97-31 | claude-opus-5-5 high | lanza pid 28420 (hijo 28120), `R97-31/lz-review` | `R97-31/review-events.jsonl`; `R97-31/lz-review.fin` | `R97-31/review-card.md` (pre-revisada READY) | veredicto en el evento `type=result`; ACCEPT o condiciones sin código → cierre de R97-31 (fila PLAN_OPS §5, EXECUTION, `attempt end`, push de `5af7273c` y del registro); REJECT → ficha de corrección para el autor |

Rutas relativas a `.git/concertino/sprint-01/`. Vigía: wsvigia lanzado de nuevo (lanza pid 7696) en
`ws://127.0.0.1:8791/`; `fins.txt` = `R97-31/lz-review.fin`. Órdenes de lanzamiento, gates sin modelo, cierres y
trampas: las mismas del relevo 6 (`git show 2e936550:docs/plans/current/HANDOFF.md`, sección «In flight»). Trampa
nueva: una orden de Bash con caracteres no ASCII (p. ej. una flecha) falla con exit 127.

## Decisions
- Usuario, vigentes: las del relevo 6 (GPU ASUS dedicada, Q-008, Q-010, Q-011, Q-012 opción B; LAUNCH y EXECUTION).
  En este tramo, la pregunta «continua sin api key ?» (sesión `3391608d`): se interpreta como seguir con la
  suscripción y sin la clave de API; aplicado al director y a los hijos.
- Director (sesión `698c3d33`, en EXECUTION): R97-08 cerrada con el `static_assert`; R97-12 cerrada con C1/C2 a
  R97-31; decisiones de alcance de R97-31 (rechazos explícitos de Ops del drafter y de R97-15, `ctest -L ops` como
  aceptación, `build/r9700-ops`).
- Director, este relevo: revisión de R97-31 a Opus 5.5 high (Claude 68 % < 85 %).

## Open questions to the user
none

## Promoted knowledge
Nada nuevo: la trampa de los caracteres no ASCII en Bash ya está en la memoria `bash-heredoc-backticks` y en las
trampas del relevo 6.

## Next step
1. Registrar en EXECUTION el gate verde de R97-31 y el lanzamiento de su revisión; commit y `git push fork fork_R9700`.
2. Atender `R97-31/lz-review.fin` según «In flight».
3. Con R97-31 cerrada: ficha de R97-16 (runtime; reactiva `openai_schema_test` y `http_error_handler_test`) y después
   R97-17 (carga y primera generación con `out/qwen3_8_27b.ninfer`), lo que el usuario espera.
Antes de cada revisión u oráculo con Claude, leer la cuota (Q-012).
