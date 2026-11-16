# [ISSUE-XXX] Short title of the problem

> **Type:** Defect | Improvement\
> **Status:** Open | Investigating | Blocked | Partial | Pending verification | Resolved and verified\
> **Detected:** YYYY-MM-DD\
> **Origin:** SPRINT-NN / plan / task, or Outside sprint\
> **Affected component:** (module, service or client)\
> **Reference specification:** (plan and task, design document)\
> **Severity:** Critical | High | Medium | Low\
> **Destination:** Current sprint | Outside sprint — urgent | Backlog\
> **Priority and reason:** urgency, impact and known deadline (if any)\
> **Owner:** known area/person, or to be assigned\
> **Follow-up:** link to the sprint task or backlog plan entry; for urgent ones, the next action\

Keep a single card per problem and update `index.md` when status or destination changes. Record the finding even if
reproduction or cause are still missing, saying what is missing. For improvements, describe the proposed capability
instead of claiming a confirmed divergence. Recording does not automatically add new work to the sprint.

---

## 1. Context and observed behaviour
- Environment or client used, inputs and data involved.
- Observed versus expected behaviour.

## 2. Divergence from the specification
- Which invariant or contract is broken? Which design rule is violated?

## 3. Minimal reproducible case
Smallest code fragment, test or command that shows the problem in isolation.

## 4. Root cause analysis
Responsible layer, suspicious files and lines, limitation involved.

## 5. Acceptance and resolution criteria
- [ ] 1. Regression test added (red before, green after).
- [ ] 2. No negative impact on the performance of critical paths.
- [ ] 3. Parity verified across the affected clients.
- [ ] 4. Documentation or specification updated if needed.

Adapt these criteria to the layers actually affected; justify the ones that do not apply.

## 6. Resolution and verification
- **Resolution sprint:** SPRINT-NN or Outside sprint (independent of the origin).
- **Resolution date:** pending until acceptance is complete; then YYYY-MM-DD.
- **Commit(s) / diagnosis:** references of the fix or evidence for closing without code changes.
- **Verification:** date, commands/scenarios, results and evidence for each applicable criterion.
- **Pending:** list them; write none only if fully resolved and verified.

When the sprint closes, keep the card and evidence in `docs/plans/archive/SPRINT-NN_<start>_<end>/issues/`. A
partial, blocked or pending issue keeps a live card in the issue queue and a **NOT RESOLVED** snapshot in the archive,
linked both ways. If it is resolved outside a sprint, use `docs/plans/archive/OUTSIDE-SPRINT_<detected>_<resolved>/`.
