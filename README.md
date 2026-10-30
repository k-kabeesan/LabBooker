
# LabBooker — Team 13

Qt 6 Widgets/C++17/SQLite desktop laboratory booking system.

## Build

Install a Qt 6.5+ MinGW 64-bit kit through Qt Creator, then open this directory as a CMake project. The required Qt components are Widgets, Sql, PrintSupport, and Test.

From a Qt-enabled terminal:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build build
ctest --test-dir build --output-on-failure
```

The application creates its SQLite database under the platform application-data directory and displays a first-run administrator setup when no users exist. No credentials are seeded in source code.

## Current implementation

The database schema, migration bootstrap, role-polymorphic domain model, PBKDF2-HMAC password handling, authentication, availability checks, transactional booking service, login window, first-run setup, dashboard, availability view, and My Bookings view are wired to SQLite. Administrator management/report pages are scaffolded for the next incremental slice; they intentionally do not use placeholder records as real data.
