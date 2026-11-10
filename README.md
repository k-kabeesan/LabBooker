# LabBooker

**University Laboratory Booking & Management System**

<p align="center">
  <strong>A desktop laboratory booking and management solution</strong><br>
  Developed by Team 13 · Wayamba University of Sri Lanka
</p>

## Overview

LabBooker helps students and lecturers check laboratory availability, request reservations and equipment, and track booking decisions. Administrators manage accounts, laboratories, equipment, schedules, approvals, and reports. The desktop application is built with C++17, Qt 6 Widgets, and SQLite.

## Features

### Students and lecturers

- Register and sign in after administrator approval.
- Manage profiles and passwords.
- Check the weekly laboratory availability grid and request bookings with equipment.
- View booking details, decision history, notifications, and personal booking counts.
- Cancel personal bookings.

### Administrators

- Approve accounts and process booking requests.
- Manage users, laboratories, equipment, and reservations.
- Filter bookings by laboratory, department, date, and status; export booking data as CSV.
- Configure four weekday booking sessions, closure dates, and sign-in contact details.
- Generate filtered reports and export them as PDF.

The interface includes light and dark themes, role-aware navigation, a collapsible sidebar, and dedicated New Booking and My Bookings pages. Schedule or closure changes that conflict with upcoming active bookings are blocked.

## Download for Windows

Download the [Windows installer](release/LabBook-Setup.exe) or [portable ZIP](release/LabBook-Portable.zip). The installer creates Desktop and Start Menu shortcuts and may request administrator permission. For the portable version, extract the entire archive and run `LabBook.exe` inside the extracted `LabBook` folder. The separately provided [LabBook.exe](release/LabBook.exe) still requires the DLLs and plugins in that folder.

See the [installation instructions](packaging/INSTALL.txt) and [Windows release report](docs/WINDOWS_RELEASE.md). Packages may also be available on the [GitHub releases page](https://github.com/k-kabeesan/LabBooker/releases).

## Build and run

Install Qt 6.5 or newer with Widgets, Sql, PrintSupport, and Test, a compatible C++17 compiler, CMake, and Ninja. In a Qt-enabled PowerShell terminal, configure with the path to your Qt kit:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build build
ctest --test-dir build --output-on-failure
./build/LabBooker.exe
```

The MSYS2 UCRT64 kit at `C:/msys64/ucrt64` is another available `CMAKE_PREFIX_PATH` in this workspace. Close a running `build/LabBooker.exe` before rebuilding it, or use another build directory.

## Data and first launch

LabBooker stores `labbooker.sqlite` in Qt's application data directory. On Windows, the path is `%APPDATA%\Team 13\LabBooker`. On an empty database, the application prompts for the initial administrator account; credentials are not embedded in the source.

The first administrator also sets a recovery question and answer. To reset a forgotten administrator password, select **Forgot password?** on the sign-in screen, enter the administrator E-ID, answer the recovery question, and choose a new password. Existing administrators can set a recovery question under **Account security** after signing in. Other users should ask an administrator to reset their password. Recovery answers are stored as salted hashes.

Back up the database before replacing an installation or launching a newer build against existing data. The current build upgrades existing databases to schema version 4 on first launch to store session times and closure dates. Do not replace an existing database with a demo database.

## Windows packaging

With a matching Qt UCRT64 kit and NSIS installed, run this from the project root:

```powershell
powershell -ExecutionPolicy Bypass -File packaging/build_windows.ps1 -NsisCompiler "C:\path\to\makensis.exe"
```

The script builds in Release mode, deploys Qt and SQLite dependencies, creates the portable ZIP, and compiles the installer.

## Project structure

| Path | Contents |
| --- | --- |
| `src/` | C++ application, domain, services, and database code |
| `resources/` | Database schema, assets, and Qt resources |
| `tests/` | Core and UI tests |
| `packaging/` | Windows installer and portable packaging scripts |
| `release/` | Windows distribution files |
| `docs/` | Design, audit, test, and release documentation |

`LabBookerTests` covers domain and database rules; `LabBookerUiTests` covers navigation and UI wiring using a temporary database. Run both with `ctest --test-dir build --output-on-failure`. See the [rebuild inventory](docs/REBUILD_AUDIT.md) for the feature map and regression checklist, and the [ERD](docs/ERD.md) for the database model.

## Team and repository

Developed by Team 13, Bachelor of Engineering Technology (Honours) in Materials and Nano Science Technology, Faculty of Technology, Wayamba University of Sri Lanka.

Repository: [k-kabeesan/LabBooker](https://github.com/k-kabeesan/LabBooker).

## License

This academic project is subject to the terms specified by its authors. No separate license file is included in this repository.
