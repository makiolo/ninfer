@echo off
rem Starts or hands off the director (coordinator) of this repo in a new terminal tab (skill concertino,
rem references\handoff.md). With docs\plans\current\HANDOFF.md ready and not accepted, opens the successor, which
rem follows its "Boot". With no HANDOFF (or with --first [--launch PATH]) opens the initial director, which runs the
rem coordinator block of the LAUNCH. With no HANDOFF and no LAUNCH opens the on-call director (references\on-call.md).
rem --dry-run only shows the command. No venv needed: the skill's tools\claude-or.cmd picks the interpreter.
rem Written by concertino install; identical in every repo.
cd /d "%~dp0"
call "S:\Projects\concertino\skills\concertino\tools\claude-or.cmd" handoff launch %*
exit /b %ERRORLEVEL%
