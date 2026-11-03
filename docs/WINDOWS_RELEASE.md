# Windows release report

## Build and contents

- Built with C++17, Qt 6 Widgets/Sql/PrintSupport, SQLite, and MinGW UCRT64 in CMake `Release` mode.
- `release/LabBook-Setup.exe` is an NSIS installer. It installs to `C:\Program Files\LabBook`, creates desktop and Start Menu shortcuts, registers an uninstaller, and uses the icon from the existing LabBook artwork.
- `release/LabBook-Portable.zip` contains `LabBook.exe`, Qt runtime DLLs, the Windows platform plugin, SQLite driver plugin, and recursive UCRT64 DLL dependencies. Assets and the database schema are embedded in the executable via Qt resources. The package has no test database or development tools.
- `release/LabBook.exe` is the application binary only. It requires the neighboring files from the portable package.

## User data and upgrades

The executable keeps the existing `LabBooker` application identity and `Team 13` organization identity. This preserves the database path `%APPDATA%\Team 13\LabBooker\labbooker.sqlite`. First launch creates the database and asks the user to create an administrator. The installer never ships, replaces, or removes the database. Uninstall leaves user data in place. Database migrations remain in the existing application code.

## Verification performed

- Fresh CMake Release build completed.
- Both Release test suites passed: `LabBookerTests` and `LabBookerUiTests`.
- Portable archive integrity and the Windows and SQLite plugin files were checked.
- The packaged application stayed running for six seconds with the development Qt directory removed from `PATH` and `QT_QPA_PLATFORM=offscreen`.
- The NSIS installer was run locally into Program Files. The installed executable, Windows platform plugin, SQLite driver, Start Menu shortcuts, desktop shortcut, and uninstall registry entry were present.
- Running the installer a second time left the existing database file timestamp unchanged. Silent uninstall removed the installed executable and desktop shortcut while retaining the database.

## Verification still needed on a separate clean Windows system

This workspace is a development machine, not a clean Windows VM. A separate-machine test is still required for the interactive installer wizard, first-run administrator setup, GUI login, export dialogs, and full module use. The local install/upgrade/uninstall and offscreen launch checks do not prove those workflows on a different PC. The installer is unsigned, so Windows may show a publisher warning.
