# Quota and cost measurement per task, rail and sprint — ninfer

Protocol of the `concertino` skill, in force since 2026-11-16. Goal: budget each sprint with evidence from the previous
ones. Subscriptions and reading tools: CONCERTINO.md "Costs". Preparing registers does not activate a sprint.

## 1. What we measure and where

The unit is the **percentage point (pp) of a provider's weekly quota**: going from 20 % to 23 % consumes 3 pp. Each
provider, account, bar and limit regime is kept separately. Never add different providers, nor a general bar and a
sub-quota that measures part of the same usage. Short windows (e.g. 5 h) are recorded for availability, never
converted into weeks.

Canonical account register in [usage/](usage/README.md), shared across sprints so a concurrent interval is never
counted twice: `observations.csv` (quota readings), `intervals.csv` (non-overlapping pairs and their difference),
`allocations.csv` (a single distribution of each interval), `tokens.csv` (tokens per segment and API equivalent),
`tariffs.csv` (versioned prices), `subscriptions.csv` (monthly fees) and `tasks.csv` (acceptance and coverage per
task).

Each dossier has a `USAGE.md`, a derived report with budget/actual/deviation, coverage and links to the register, a
cut-off date and a commit. The register is never moved when archiving. Private evidence stays out of Git
(`.git/concertino/usage/`), without prompts, credentials or authentication data.

## 2. Owners and capture moments

One **measurement coordinator per account** integrates the CSVs; sub-agents never write the register at the same
time. Take a fresh reading: when a session or rail starts; before assigning each task or attempt; every ~30 min with
concurrent work or when the task, model or provider changes; when finishing, pausing, failing or handing off; and at
end of day, at a reset or at sprint close. If the agent cannot read a bar, ask the user for the used percentage, the
time and the next reset; without data, `missing` (never 0).

## 3. Intervals

A measurable interval needs the same account/bar/regime/reset, fresh readings and time order.
`delta_pp = used_end − used_start`; negative, or crossing a reset without endpoints → `gap`. A difference of 0 pp on
a bar with 1 pp resolution does not prove zero cost. Round only when displaying.

## 4. Attribution without double-counting parallel work

Distribute each delta **only once**, with shares that add up to 1: `isolated` (a single task uses the account),
`estimated` (concurrent tasks, split by a documented weight), `unattributed` (residue). `quota_pp = delta_pp × share`.
Reserved targets: `OVERHEAD` (coordination), `COORD` (cross-cutting rail), `UNATTRIBUTED`, `EXTERNAL` (proven
activity outside the project). Phases `planning`, `execution`, `close`, `release`, reported separately.

## 5. Dollars

- Imputed subscription: `USD = monthly fee × (7/30) × (pp/100)`. It spreads the fee to compare tasks; it is not an
  invoice.
- API equivalent: tokens × versioned price, per segment and de-duplicated per response. No price → gap, never 0 USD.
- Real spend (`billed_usd`): only what is proven. The four magnitudes are never added together.

## 6. Report and budget

Per task, add all its segments (children, roles, attempts) with columns per provider; then group by rail and sprint.
`Σ measured delta_pp = Σ assigned quota_pp` per provider. Publish the attributable subtotal, residue and gaps. To
budget the next sprint use only accepted tasks with full coverage that are comparable (same difficulty scheme,
provider, regime and profile): n, minimum, median, maximum (P75 with n ≥ 5). Base = median; prudent = P75 or range.
Coordination and close go apart, with an explicit reserve for new kinds of work. Equivalent weeks are not a deadline.
