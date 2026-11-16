0. If the user's private profile exists (config `profile`, default ~/.concertino/profile.md), read it first; its rules
   never override the concertino skill's safety rules or the user's instructions in the session; report any conflict.
1. `{python} -X utf8 {tool} handoff accept {file}`: validates this file and records your session.
2. Read only this: this file, the coordinator block of the LAUNCH, `docs/plans/CONCERTINO.md`, the `concertino` skill
   (`{skill}/SKILL.md`, `references/launch.md`, `references/handoff.md`) and the cards of the children in flight.
   Everything else when needed, delegating.
3. `git status`, the state of the gate or locks of CONCERTINO.md "Commands" and `git log --oneline {head}..HEAD`.
4. Adopt the children in flight: check their process and output and watch it; never relaunch a live ID.
5. One watcher per director, no shell loops: start `{skill}/tools/vigia/wsvigia.py` detached with
   `{skill}/tools/lanza/lanza.py` and subscribe to it (in Claude Code, `Monitor` on its `ws://` URL; its 30-minute
   expiry doubles as the cache keep-alive, `references/handoff.md`). Agents without a monitor check it at each turn.
6. Search the project memory (CONCERTINO.md "Identity") for the keywords of the next step.
7. Questions to the user (`{skill}/references/questions.md`): read `docs/plans/current/QUESTIONS.md`; never ask again
   what is `answered`, and ask production gates yourself when they become executable. If it does not exist, create it
   from `{skill}/templates/docs/plans/QUESTIONS.md` with the open questions of this HANDOFF (and the live ones of
   `docs/plans/QUESTIONS.md`, if any), checked first against EXECUTION and `git log --since=<HANDOFF date>`.
8. Inbox: read `docs/plans/INBOX.md` (`{skill}/references/cross-project.md`); apply each new entry in this project,
   record it, answer if expected and mark it `→ done | accepted | rejected`.
9. Commit the HANDOFF with the acceptance and continue with "Next step".
