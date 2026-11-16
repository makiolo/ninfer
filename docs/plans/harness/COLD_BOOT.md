
## Boot

You are the new director after an interruption. Nobody writes in the repo on behalf of the previous director: it died.
1. Read this report, the last HANDOFF and the LAUNCH coordinator block it names.
2. `git status` and the state of the gate or locks of CONCERTINO.md. If a lock belongs to a dead process, follow the
   project's gate policy.
3. Interrupted children: look at their output and at the WIP of their ID in the tree. What is finished is reviewed
   and accepted with the normal protocol; what was left halfway is kept and the ID is relaunched from its card with
   the partial output as context. Never discard WIP without looking at it. WIP of other live sessions is not yours.
4. Rebuild a new HANDOFF:
   `{python} -X utf8 {tool} handoff new {file} --from-session {session}`,
   fill it in with the rebuilt state, commit, then `handoff accept {file}`.
5. Read `docs/plans/INBOX.md` and the live QUESTIONS register (`{skill}/references/questions.md`).
6. Continue with its "Next step".
