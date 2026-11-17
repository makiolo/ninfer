"""Preserve an empty sync setting in the initial Windows environment block."""
import os
import subprocess
import sys

environment = dict(os.environ, NINFER_CUDA_SYNC='')
sys.exit(subprocess.run([sys.argv[1], '--invalid-sync'], env=environment).returncode)
