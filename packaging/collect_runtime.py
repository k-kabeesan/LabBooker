"""Copy the Qt plugins and recursive UCRT64 DLL dependencies for LabBook."""
from pathlib import Path
import re
import shutil
import subprocess
import sys

package = Path(sys.argv[1]).resolve()
qt_bin = Path(sys.argv[2]).resolve()
qt_plugins = Path(sys.argv[3]).resolve()
for category, name in (("platforms", "qwindows.dll"), ("sqldrivers", "qsqlite.dll")):
    destination = package / category
    destination.mkdir(parents=True, exist_ok=True)
    shutil.copy2(qt_plugins / category / name, destination / name)

pending = list(package.rglob("*.dll")) + [package / "LabBook.exe"]
seen = set()
while pending:
    binary = pending.pop()
    if binary.resolve() in seen:
        continue
    seen.add(binary.resolve())
    result = subprocess.run([str(qt_bin / "objdump.exe"), "-p", str(binary)],
                            capture_output=True, text=True, check=True)
    for name in re.findall(r"DLL Name:\s*(\S+)", result.stdout):
        source = qt_bin / name
        target = package / name
        if source.is_file() and not target.exists():
            shutil.copy2(source, target)
            pending.append(target)

assert (package / "platforms" / "qwindows.dll").is_file()
assert (package / "sqldrivers" / "qsqlite.dll").is_file()
print(f"Runtime package: {len(list(package.rglob('*.*')))} files")
