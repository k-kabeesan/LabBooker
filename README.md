
# LabBooker

University laboratory booking and management desktop application built with C++17, Qt 6 Widgets, and SQLite.

## Download for Windows

Use the [Windows installer](release/LabBook-Setup.exe) for a normal installation with desktop and Start Menu shortcuts. Windows may ask for administrator permission to install into Program Files. For a portable copy, download [LabBook-Portable.zip](release/LabBook-Portable.zip), extract the whole `LabBook` folder, and run `LabBook.exe` inside it. The [LabBook.exe](release/LabBook.exe) file is also provided separately, but it needs the DLL and plugin files in the portable folder; it is not a single-file program. See [installation instructions](packaging/INSTALL.txt) and the [release report](docs/WINDOWS_RELEASE.md).

## Build and run

Install Qt 6.5 or newer with Widgets, Sql, PrintSupport, and Test, plus a matching C++ compiler and CMake/Ninja. In a Qt-enabled terminal:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build build
ctest --test-dir build --output-on-failure
./build/LabBooker.exe
```

On this Windows workspace the Qt kit is also available at `C:/msys64/ucrt64`. If an existing `build/LabBooker.exe` is running, close it before rebuilding in that directory or use a separate build directory such as `build-verify`.

The application stores `labbooker.sqlite` under Qt's application data directory (on this Windows installation, `%APPDATA%\Team 13\LabBooker`). It prompts for the initial administrator on an empty database. No credentials are embedded in the source. Back up the database before replacing an installation; never substitute a demo database for existing user data.

To reproduce the Windows installer, install a matching Qt UCRT64 kit and NSIS, then run `powershell -ExecutionPolicy Bypass -File packaging/build_windows.ps1 -NsisCompiler "C:\path\to\makensis.exe"` from the project root. The script builds in Release mode, deploys Qt and SQLite dependencies, creates the portable ZIP, and compiles the installer.

The current build upgrades existing databases to schema version 4 on first launch to store booking session times and closure dates. Back up the database before launching the new build against existing data.

## Features

- Student and lecturer registration, administrator approval, authentication, profile and password management.
- Role-aware dashboard, weekly availability grid, four configurable two-hour weekday booking slots, equipment requests, personal bookings, cancellation, booking details, decision history, and persistent in-app notifications.
- Administrator approval queue and all-booking management with laboratory, department and date filters, booking status filtering, CSV export, laboratory/equipment/user management, filtered reports and PDF export.
- Administrator settings for booking session times, holiday/closure dates, and sign-in contact details. Closure and schedule changes are blocked while they conflict with upcoming active bookings.
- Light and dark themes, role-aware sidebar with separate New Booking and My Bookings pages, per-user booking count, collapsible navigation, and consistent data tables.

See [rebuild inventory](docs/REBUILD_AUDIT.md) for the complete feature map, schema, backup and regression checklist. `LabBookerTests` covers domain and database rules; `LabBookerUiTests` covers key navigation and UI wiring using a temporary database.
