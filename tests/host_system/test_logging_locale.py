"""Give the console encoding check a private, hidden Windows console."""
import subprocess
import sys

startup = subprocess.STARTUPINFO()
startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = subprocess.SW_HIDE
result = subprocess.run([sys.argv[1]], capture_output=True, encoding='utf-8', timeout=30,
                        creationflags=subprocess.CREATE_NEW_CONSOLE, startupinfo=startup)
print(result.stdout, end='')
print(result.stderr, end='', file=sys.stderr)
sys.exit(result.returncode)
