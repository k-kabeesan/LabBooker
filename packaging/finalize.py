"""Generate the uninstall file list and portable archive from the staged app."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import sys

root = Path(__file__).resolve().parent.parent
package = root / "build-release" / "package" / "LabBook"
release = root / "release"
files = sorted(path for path in package.rglob("*") if path.is_file())
directories = sorted((path for path in package.rglob("*") if path.is_dir()),
                     key=lambda path: len(path.parts), reverse=True)
assert (package / "LabBook.exe").is_file()
assert (package / "platforms" / "qwindows.dll").is_file()
assert (package / "sqldrivers" / "qsqlite.dll").is_file()
lines = [f'Delete "$INSTDIR\\{path.relative_to(package)}"' for path in files]
lines += [f'RMDir "$INSTDIR\\{path.relative_to(package)}"' for path in directories]
(root / "packaging" / "uninstall-files.nsh").write_text("\n".join(lines) + "\n", encoding="utf-8")
release.mkdir(exist_ok=True)
archive = release / "LabBook-Portable.zip"
with ZipFile(archive, "w", ZIP_DEFLATED, compresslevel=9) as out:
    for path in files:
        out.write(path, Path("LabBook") / path.relative_to(package))
with ZipFile(archive) as checked:
    assert checked.testzip() is None
(release / "LabBook.exe").write_bytes((package / "LabBook.exe").read_bytes())
print(f"Packaged {len(files)} files in {archive}")
