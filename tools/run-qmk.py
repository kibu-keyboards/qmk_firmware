"""Run the bundled QMK CLI, optionally fixing its generated build-date metadata."""
import datetime
import importlib
import os
import sys
from pathlib import Path

source = Path(__file__).resolve().parents[1] / "qmk_firmware"
os.chdir(source)
os.environ["ORIG_CWD"] = str(source)
sys.path.insert(0, str(source / "lib/python"))
sys.stdout.reconfigure(encoding="utf-8", newline="\n")

from qmk.cli import cli

fixed = os.environ.get("QMK_BUILD_DATE")
if fixed:
    datetime.datetime.strptime(fixed, "%Y-%m-%d-%H:%M:%S")
    module = importlib.import_module("qmk.cli.generate.version_h")
    original_strftime = module.strftime
    module.strftime = lambda fmt: fixed if fmt == module.TIME_FMT else original_strftime(fmt)

cli()
