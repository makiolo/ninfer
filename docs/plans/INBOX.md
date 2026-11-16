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
