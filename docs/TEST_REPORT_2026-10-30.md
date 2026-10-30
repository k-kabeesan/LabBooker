# LabBooker test report — 30 October 2026

## Build and test environment

- Windows x64, C++17, Qt 6 Widgets/Sql/PrintSupport/Test, SQLite, CMake/Ninja.
- Tested source: current workspace; executable: `build-redesign/LabBooker.exe` (SHA-256 matches `dist/LabBooker/LabBooker.exe`).
- `cmake --build build-redesign`: **passed**.
- `ctest --test-dir build-redesign --output-on-failure`: **2/2 suites passed**, 39.35 seconds. The suites contain 11 core test cases and one UI test case.
- All automated tests use temporary SQLite databases. The user's live database was only queried read-only.

## Verified areas

| Area | Evidence | Result |
| --- | --- | --- |
| Packaged startup | Launched `dist/LabBooker/LabBooker.exe`; observed its login window and later the administrator dashboard in Windows UI Automation | Pass |
| Authentication and roles | Correct/wrong password, password change, student/lecturer/admin permissions, registration approval | Pass in temporary database |
| Navigation | UI test visits every administrator page; student cannot open admin pages; separate My Bookings and New Booking views | Pass |
| Dashboard and Availability | Live administrator dashboard showed database-backed counts and upcoming table; live Availability page showed the selected laboratory, date, slot, calendar button and schedule | Pass for display/navigation |
| Calendar picker | UI test opens picker, chooses weekday, verifies date field; weekend confirmation is disabled by UI logic | Pass in UI test |
| Booking rules | Four fixed two-hour weekday slots, lunch break, operating hours, double-booking prevention, role consistency, lab/date isolation | Pass in temporary database |
| Booking lifecycle | Create, review, reject, cancel, approval history, released slots after cancellation/rejection | Pass in temporary database |
| Equipment reservations | Quantity checks, booking equipment persistence, transaction rollback | Pass in temporary database |
| Management | User, laboratory, equipment add/update/activation; updated records visible through a second database connection | Pass in temporary database |
| Reports | Counts/HTML summary and actual PDF generation with `%PDF` signature | Pass in temporary database |
| Migration | Version 1 database upgraded to version 3 with legacy rows retained | Pass in temporary database |
| Live database safety | `PRAGMA integrity_check=ok`; schema version 3; read-only counts: 1 user, 1 laboratory, 0 equipment, 1 booking, 2 approval records | Pass |

## Limits and observations

- The live desktop session was not used to create, edit, approve, reject, cancel, or export real records. Those actions were tested against temporary databases to avoid changing user data.
- Live UI inspection confirmed dashboard and Availability. Other pages were constructed and visited by the automated UI test, but their complete visual layouts were not manually inspected in the live signed-in session. The live window began receiving user input during inspection, so UI automation stopped.
- Login/logout through the live UI, first-run setup on a fresh installed machine, PDF appearance, dark-theme appearance, keyboard-only navigation, and resizing across multiple screen sizes remain unverified manually. Automated tests cover the underlying authentication, migration, PDF generation, and role-limited page wiring.
- No test failure was observed. Passing automated tests does not prove every possible desktop interaction or deployment environment.
