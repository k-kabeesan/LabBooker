# LabBooker rebuild inventory

## Architecture and data

- C++17, Qt 6 Widgets/Sql/PrintSupport, CMake, SQLite. `src/main.cpp` starts the application, creates the database connection, handles first-run administrator setup, and switches between login and the authenticated main window.
- `src/domain.h` defines users, roles, permissions, statuses, bookings, laboratories, and equipment. `src/services.cpp` owns authentication, permission checks, availability, booking, management, and reporting rules. `src/database.cpp` owns SQLite queries and schema migrations. `src/mainwindow.cpp` implements the screens and dialogs.
- The live database is `%APPDATA%\Team 13\LabBooker\labbooker.sqlite`. `resources/schema.sql` defines roles, users, student and lecturer profiles, laboratories, equipment, bookings, approval history, booking equipment, and audit log. Existing data is schema version 3. Appearance, remembered E-ID, and profile image paths are stored with `QSettings`; profile image copies are stored in the application data folder.
- The schema uses foreign keys, uniqueness and quantity checks, and indexes for booking and user lookups. Booking writes use transactions. SQL inputs in the repository/database services are bound parameters. The version 1 and 2 migration paths preserve rows; no schema change is needed for this UI rebuild.

## Feature preservation checklist

| Area | Existing workflow and responsible code | Verification |
| --- | --- | --- |
| Setup and authentication | First-run administrator setup, login, remember E-ID, password visibility, account registration and approval, logout; `main.cpp`, `LoginWindow`, `AuthService` | Core tests; UI smoke test |
| Roles | Student and lecturer self-service; administrator management and reporting; `User::permissions`, service permission checks, `MainWindow` navigation | Core tests; UI smoke test |
| Dashboard | Role-specific database counts, upcoming bookings, availability shortcut; `MainWindow::dashboard/refreshDashboard` | UI smoke test |
| Availability | Weekdays, four fixed two-hour slots, lunch closure, laboratory status and booking conflicts; `AvailabilityService`, availability page | Core tests |
| New Booking and My Bookings | Separate reservation form and personal history page; create, edit, cancel, equipment requests, status and decision notes; per-user total booking count; `BookingService`, booking pages | Core tests; UI smoke test |
| Administrator bookings | All bookings, complete, cancel, search; `BookingService`, admin bookings page | Core tests; UI smoke test |
| Approvals | Approve/reject pending bookings and store review history; `BookingService`, approvals page | Core tests |
| Laboratories | Search, add, edit, activate/deactivate; `LaboratoryManager`, laboratory page and dialog | Core tests |
| Equipment | Search, add, edit, activate/deactivate and reservation quantities; `EquipmentManager`, equipment page and dialog | Core tests |
| Users | Search, add, edit, approve, activate/deactivate; `UserManager`, users page and dialog | Core tests |
| Reports | Date-filtered booking, laboratory and equipment summaries; PDF export; `ReportService`, reports page | Core tests |
| Account | Profile details/photo, password change, light/dark theme; profile and settings pages, `QSettings` | UI smoke test |
| Persistence | SQLite schema version 3 and legacy migration; `DatabaseManager` | Core migration test; backup integrity check |

## Baseline and risks

- Existing `LabBookerTests` passed all 10 cases on 30 October 2026. The original build was blocked at link time because `build/LabBooker.exe` was in use; an isolated build directory avoids that lock.
- The existing sidebar, top bar, dashboard, cards, dark theme, fixed-slot booking controls, and management pages are functional. The administrator bookings search box was not wired to filtering. Repeated table setup and inlined styles make visual consistency costly to maintain.
- The reports page previously displayed an all-time summary while its PDF action still exported the date-filtered summary; the two actions now share the selected report period.
- A source archive and consistent SQLite backup were made in the ignored `backups/` directory before edits. No migration or write to the live database is part of this rebuild. To roll back source, restore the archive; to roll back data, restore the SQLite backup only after closing the application.
