
# LabBooker

University laboratory booking and management desktop application built with C++17, Qt 6 Widgets, and SQLite.

## Download for Windows

Download [`release/LabBooker-Windows-x64.zip`](release/LabBooker-Windows-x64.zip), extract the entire ZIP, then run `LabBooker.exe` from the extracted folder. The ZIP includes the Qt runtime files required by the application. The standalone [`release/LabBooker.exe`](release/LabBooker.exe) is provided for users who already have the matching Qt runtime installed.

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

## Features

- Student and lecturer registration, administrator approval, authentication, profile and password management.
- Role-aware dashboard, laboratory availability, four fixed weekday booking slots, equipment requests, personal bookings, cancellation, and decision history.
- Administrator approval queue, all-booking management and search, laboratory/equipment/user management, filtered reports and PDF export.
- Light and dark themes, role-aware sidebar with separate New Booking and My Bookings pages, per-user booking count, collapsible navigation, and consistent data tables.

See [rebuild inventory](docs/REBUILD_AUDIT.md) for the complete feature map, schema, backup and regression checklist. `LabBookerTests` covers domain and database rules; `LabBookerUiTests` covers key navigation and UI wiring using a temporary database.
