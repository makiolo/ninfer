# Execution harness — ninfer

Minimum rules for all delegated work (skill `concertino`, `references/delegation.md` and `references/card.md`). Each
rule says how it is checked. A critical rule that lives only in text becomes a mechanical control as soon as there is
a way (A9). Adjust them to the project with date and reason; analyses that justify them are kept in this folder
(`ANALYSIS_<date>.md`…).

- **A1. Data egress.** Sending code to an API is already taking data out. Credentials, `.env` and sensitive real data
  never leave. Authorised destinations: «the project's subscriptions». Restricted list: «fill in».
  *Check:* the card's `Egress:` field and the route actually used.
- **A2. Complete card, checked before launch** (skill `concertino`, `references/card.md`): observable result, out of
  scope, constraints, invariants, edge cases, verifiable acceptance, authorised paths, stop condition and an excerpt
  of the rules. A cheap read-only model first lists the doubts that would change contract, security or acceptance.
  *Check:* its output is saved next to the card.
- **A3. Three labels per task:** difficulty L, risk (`low`, `medium`, `high`, `silent-numeric`) and human attention
  (`S`, `M`, `L`).
- **A4. Independent reviewer with clean context:** a different concrete model from the author; it receives the card,
  the diff and related code, never the conversation or the author's report.
- **A5. Acceptance protected from the author:** the author may add tests, never modify or delete existing ones nor the
  gate/CI configuration, unless `Test changes: allowed (reason)` is reviewed separately.
  *Check:* `git diff --name-status <base>` before accepting.
- **A6. Evidence belongs to the gate and to a fixed version of the code:** gate output with `HEAD` and the tree
  fingerprint before and after; if they changed in between, it does not count.
- **A7. Silent-numeric risk:** an oracle different from the author prepares reference cases first; the regression
  fails red before the fix.
- **A8. Reviewable deliveries:** split by verifiable behaviour; attention `L` or > 400 changed lines forces a split or
  a justification.
- **A9. Ratchet at every close:** "failure → cause → control → proof that it detects it".
- **A10. Measure consistency, not the best attempt:** attempts, rejections and escalations are recorded.
- **A11. Director handoff before its context fills up** (`references/handoff.md`): zones 70/80/85 %, HANDOFF ≤ 200 lines
  with volatile state only, a single director writing, cache keep-alive.
  Successor boot: [BOOT.md](BOOT.md); after a shutdown: [COLD_BOOT.md](COLD_BOOT.md).
- **A12. The director decides** what has a clear recommended option within what is authorised and writes it down; it
  keeps asking about scope, public contracts, data, publishing and anything irreversible.
