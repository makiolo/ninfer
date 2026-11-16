@echo off
rem Recovers the director of this repo after a shutdown or a director that died without a handoff (skill concertino,
rem references\handoff.md, "Cold recovery"): rebuilds the state from the transcript (report in
rem .git\concertino\handoff\RECOVERY-<date>.md) and opens the successor. --dry-run writes the report but opens no tab.
rem No venv needed: the skill's tools\claude-or.cmd picks the interpreter. Written by concertino install.
cd /d "%~dp0"
call "S:\Projects\concertino\skills\concertino\tools\claude-or.cmd" handoff recover %*
exit /b %ERRORLEVEL%
