#!/bin/sh
# Recovers the director of this repo after a shutdown or a director that died without a handoff (skill concertino,
# references/handoff.md, "Cold recovery"): rebuilds the state from the transcript (report in
# .git/concertino/handoff/RECOVERY-<date>.md) and opens the successor. --dry-run writes the report but opens nothing.
# Written by concertino install; identical in every repo.
cd "$(dirname "$0")" || exit 1
exec "S:/Projects/concertino/skills/concertino/tools/claude-or.sh" handoff recover "$@"
