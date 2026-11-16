# Sprint dependencies — ninfer

Contract read by the `concertino` skill (`concertino deps`, and when planning, starting, launching and closing).
Path: absolute path of the other repository, or `-` to use the path registered in the portfolio
(`~/.concertino/projects.json`, written by `concertino install`). Condition: `closed` (the remote sprint is in its
`archive/`) or `active` (it is in its `current/`; also met once it is closed). Status: `decided (user, YYYY-MM-DD)`
blocks activation when not met; `proposed` only warns. Each project declares only its incoming dependencies; the
outgoing ones are deduced by reading the other projects' `DEPENDENCIES.md`.

## Related projects

| Project | Path |
|---|---|

## Incoming dependencies

| Local sprint | Depends on | Condition | Reason | Status |
|---|---|---|---|---|

None yet.

## Task dependencies

Optional. Used when two sprints of different projects run at the same time and only one task waits for a remote
task (a sprint row would block the whole activation). Does not block activation.

| Local task (sprint) | Depends on (project, sprint, task) | Reason / meanwhile | Status |
|---|---|---|---|
