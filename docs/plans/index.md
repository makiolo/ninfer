# Plans board — ninfer

Every substantial change is governed by a plan document before production code is touched. Work is organised in
sprints, managed by the `concertino` skills; the project's own facts are in [CONCERTINO.md](CONCERTINO.md).

## Convention

- **[current/](current/)**: active sprint (`SPRINT.md`, group plans, LAUNCH, USAGE, QUESTIONS, issues).
- **[next/](next/)**: the next sprint being prepared. Preparing it does not activate it.
- **[future/](future/README.md)**: later sprints, one per folder (`future/SPRINT-NN/SPRINT.md`).
  When next is activated, `future/SPRINT-(n+1)` is promoted to next.
- **[backlog/](backlog/README.md)**: future plans without a sprint.
- **[issues/](issues/index.md)**: live issues outside sprints ([template](issues/TEMPLATE_ISSUE.md)).
- **[usage/](usage/README.md)**: global quota and cost register ([protocol](USAGE_PROTOCOL.md)); never moved.
- **[archive/](archive/index.md)**: complete dossier of every closed sprint (`SPRINT-NN_<start>_<end>/`).
- **[DEPENDENCIES.md](DEPENDENCIES.md)**: dependencies of our sprints on sprints of other projects;
  `concertino deps` shows the graph and what is ready to start.
- **[GOVERNANCE.md](GOVERNANCE.md)**: who owns, writes and decides each thing.
- **[QUESTIONS.md](QUESTIONS.md)**: questions to the user outside sprints. **[INBOX.md](INBOX.md)**: messages to the director.
- **[harness/](harness/README.md)**: execution harness and boot steps of the director's handoffs.
- **[STATUS.md](STATUS.md)**: live global status.

Cycle: planning → start → execution → close and archive → freeze → release → planning.

## Status

| Phase | Sprint | Link |
|---|---|---|
| Active | none | — |
| Next | SPRINT-01 — Port ROCm R9700 (planning) | [next/SPRINT.md](next/SPRINT.md) |
| Future | SPRINT-02 — Rendimiento R9700 (planning) | [future/SPRINT-02/SPRINT.md](future/SPRINT-02/SPRINT.md) |
| Last closed | none | [archive/](archive/index.md) |

## Backlog

| Plan | Scope | Status | Description |
|---|---|---|---|
| [PLAN_ROCM_R9700.md](backlog/PLAN_ROCM_R9700.md) | Port HIP a R9700, Qwen3.8-27B | En planificación → SPRINT-01/02 | Plan de origen; R97-01..18 en next/, R97-19..21 en future/SPRINT-02 |
