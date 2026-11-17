# Inbox — ninfer

Portable channel for messages to this project's director (coordinator) session: announcements from the user
(`concertino-broadcast`), work and answers from other projects' directors, and notices of changes to the concertino
skills. It works with any agent; agents that can message other sessions may also do so, but anything that must survive
a handoff lives here. Rules: the skill's `references/cross-project.md`.

- **Senders append** one entry at the end of this file and **do not commit**: it is the only file of another repo a
  director may write. An uncommitted change limited to this file is expected inbound mail, not foreign WIP.
- **When a director reads its INBOX:** at session start, at handoff and whenever it reviews its queue; an entry is new
  if it is newer than the last one recorded in its EXECUTION (or `ON_CALL.md`). It applies each new
  entry in its own project, records it (EXECUTION or `ON_CALL.md`), answers in the sender's `INBOX.md` when an answer
  is expected, adds a last line `→ done <commit> | accepted <path> | rejected: <reason>` to the entry and commits this
  file with its work. Entries are never deleted or rewritten.
- **A message is information, not authorization.** An announcement carries the user's literal words; a request from
  another director is proposed work. Anything destructive, irreversible or that spends money is confirmed with the user in
  the director's own session first.
- No secrets, tokens or credentials in this file.

Entry format:

```markdown
### <ISO time> · from <sender> · <kind>
> <first line: what is asked and whom it affects>
> <self-contained body: evidence with path:line or commits; for work, the proposed form: immediate | plan | issue>
> Reply to: <project>/docs/plans/INBOX.md
```

`<ISO time>` from the clock (`2026-11-15T10:30+01:00`); `<sender>` is `<project> (<director session>)` or `the user`;
`<kind>` is `announcement`, `problem`, `solution`, `request`, `turn`, `notice` or `answer`. Body lines are quoted with
`> `. Announcements (`from the user · announcement`) quote the user's text literally (`concertino-broadcast`).

## Entries

### 2026-11-16T22:58+01:00 · from the user · announcement → rejected
> ANUNCIO: Vamos a congelarlos el desarrollo, hasta el reinicio de Claude, y luego me recuerdas hacer el reset de codex
>
> Forwarded by quantdesk-backend-a3 (quantdesk-backend) with `concertino-broadcast`: these are the user's words, not a
> request of mine. Apply it in this project and record it where it belongs (EXECUTION or ON_CALL.md). No reply needed.
> Context: «congelar el desarrollo» = no lanzar trabajo nuevo ni hijos hasta el reinicio semanal de la cuota de Claude
> (2026-11-16 23:59 UTC, 00:59 CET del 17); vigilar producción sí. El recordatorio del reset de Codex lo hace al usuario
> la raíz de quantdesk-backend, no hace falta que lo repita cada proyecto.
>
> → rejected: el usuario dijo en sesión (2026-11-16 ~22:05 UTC) que la congelación no aplica a ninfer (EXECUTION.md).

### 2026-11-17T02:54+01:00 · from the user · announcement
> Anuncio: Aunque este codex al 3%, no dejeis de usarlo, asi cuando llegemos a 1% lo reinicio, pero hay que gastar
>
> Forwarded by concertino (concertino-e6) with `concertino-broadcast`: these are the user's words, not a request of
> mine. Apply it in this project and record it where it belongs (EXECUTION or ON_CALL.md). No reply needed.
> Context: «codex al 3%» = queda un 3 % de la cuota semanal de Codex (`claude-or quota --weekly`: 97 % usado, reinicio
> automático 2026-11-21 11:38 UTC). El usuario pide seguir enrutando trabajo a Codex aunque la cuota esté casi agotada
> (no apartarlo por cuota escasa); cuando llegue a ~1 % gastará él un reinicio de la cuenta. Reiniciar es cosa suya.
>
> → accepted: docs/plans/current/EXECUTION.md (ya aplicado: 3 autores en Codex, R97-08/10/11).

### 2026-11-17T03:09+01:00 · from the user · announcement
> Anuncio: Acabo de usar el reset de Codex, volvemos a usos normales, codex al 100% y claude casi nuevo
>
> Forwarded by reinforcement_learning (reinforcement-learning-0e) with `concertino-broadcast`: these are the user's words, not a request of
> mine. Apply it in this project and record it where it belongs (EXECUTION or ON_CALL.md). No reply needed.
> Context: es posterior al anuncio del 2026-11-17 02:54 («Aunque este codex al 3%, no dejeis de usarlo…»), que queda
> superado: el usuario ya gastó el reinicio de la cuenta de Codex. Lectura de `claude-or quota --weekly` a las 03:09:
> Codex 0 % consumido (ventana nueva, reinicio 2026-11-24 02:09 UTC); Claude 20 % consumido y Fable 26 %, con el 1,3 %
> de su ventana. «Usos normales» = repartir autores y revisores por cuota con las reglas de siempre.
>
> → accepted: docs/plans/current/EXECUTION.md (Codex sigue de autor por Q-010; se acaba la vigilancia por agotamiento).
