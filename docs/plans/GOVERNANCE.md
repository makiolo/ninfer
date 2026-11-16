# Responsibilities and governance — ninfer

Who owns each thing this project produces or consumes, who writes it, who decides and how another project asks for
changes. It is the reference for coordinating work across projects; every sprint reviews it when planning and
starting (skill `concertino`, `references/start.md` step 4) and copies the rows that affect it into its SPRINT.md §2.
Time dependencies between sprints live in [DEPENDENCIES.md](DEPENDENCIES.md).

Status: `decided (user, date)` or `proposed` (still to be asked to the user).

| Thing (document, code, data, artifact, decision) | Owner (project) | Writes | Decides / approves | Changes from another project | Status |
|---|---|---|---|---|---|
| Backlog, sprints and `docs/plans/` of ninfer | ninfer | ninfer sessions | user | note or issue in this repo | decided |
| Upstream NInfer code (`src/`, `apps/`, `cmake/`, `tests/`, `bench/`) and its CUDA targets | upstream `Neroued/ninfer` | ninfer sessions in this clone (port changes only) | user | none: no other project writes here | decided (user, 2026-11-16) |
| `r9700` target, HIP compatibility and AMD kernels (port) | ninfer | ninfer sessions | user | note or issue in this repo | decided (user, 2026-11-16) |
| `AGENTS.md` and stable docs in `docs/` (non-plans) | ninfer (upstream rules) | only when the user asks | user | — | decided (AGENTS.md) |
| Model artifact `out/qwen3_8_27b.ninfer` | ninfer | R97-17 downloads it | user | — | decided (user, 2026-11-16) |
| R9700 GPU dedicated to ninfer during SPRINT-01 (the other keeps its local-AI slot) | user's machine | ninfer sessions only | user | turn protocol (`cross-project.md`) | decided (user, 2026-11-16, Q-006) |
