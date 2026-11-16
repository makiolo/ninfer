"""Exercise real process handlers in a private hidden console on Windows."""
import os
import subprocess
import sys
from pathlib import Path

options = {}
if os.name == 'nt':
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = subprocess.SW_HIDE
    options.update(creationflags=subprocess.CREATE_NEW_CONSOLE, startupinfo=startup)
for argument in ('--ctrl-c', '--break'):
    result = subprocess.run([sys.argv[1], argument], capture_output=True, text=True,
                            timeout=20, **options)
    print(argument, 'exit', result.returncode, result.stdout, result.stderr, flush=True)
    if result.returncode or result.stdout.count('cleanup complete') != 2:
        sys.exit(1)
    if result.stdout.count('resources released') != 2:
        sys.exit(1)

if os.name == 'nt':
    result = subprocess.run([sys.argv[1], '--close'], capture_output=True, text=True,
                            timeout=20, **options)
    print('--close', 'exit', result.returncode, result.stdout, result.stderr, flush=True)
    if (result.returncode & 0xffffffff) not in (0, 0xc000013a):
        sys.exit(1)
    if 'cleanup complete\nresources released\n' not in result.stdout:
        sys.exit(1)

# This path is in the caller's build tree, which the gate places under .agent-tmp/R97-06.
log_path = Path.cwd() / 'redirected-log.txt'
try:
    with log_path.open('wb') as output:
        result = subprocess.run([sys.argv[1], '--redirected-log'], stdout=subprocess.PIPE,
                                stderr=output, timeout=20, **options)
    output = log_path.read_bytes()
    print('file redirection', 'exit', result.returncode, repr(output), flush=True)
    if (result.returncode or output.count(b'\n') != 2 or b'\r' in output or b'\x1b' in output
            or b'starting engine' not in output or b'redirected record' not in output
            or b'transient' in output):
        sys.exit(1)
finally:
    log_path.unlink(missing_ok=True)
