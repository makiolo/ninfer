#!/bin/sh
# Starts or hands off the director (coordinator) of this repo in a new terminal window (skill concertino,
# references/handoff.md). With docs/plans/current/HANDOFF.md ready and not accepted, opens the successor; with no
# HANDOFF (or with --first [--launch PATH]) opens the initial director, which runs the LAUNCH coordinator block; with no
# HANDOFF and no LAUNCH opens the on-call director. --dry-run only shows the command.
# Written by concertino install; identical in every repo.
cd "$(dirname "$0")" || exit 1
exec "S:/Projects/concertino/skills/concertino/tools/claude-or.sh" handoff launch "$@"
