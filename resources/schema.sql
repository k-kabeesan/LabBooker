CREATE TABLE IF NOT EXISTS roles (
  role_id INTEGER PRIMARY KEY,
  role_name TEXT NOT NULL UNIQUE CHECK(role_name IN ('Student','Lecturer','Administrator'))
);
INSERT OR IGNORE INTO roles(role_id, role_name) VALUES (1,'Student'),(2,'Lecturer'),(3,'Administrator');

CREATE TABLE IF NOT EXISTS users (
  user_id INTEGER PRIMARY KEY AUTOINCREMENT,
  eid TEXT NOT NULL COLLATE NOCASE UNIQUE,
  name TEXT NOT NULL,
  email TEXT NOT NULL DEFAULT '',
  faculty TEXT NOT NULL DEFAULT '',
  department TEXT NOT NULL DEFAULT '',
  contact TEXT NOT NULL DEFAULT '',
  role_id INTEGER NOT NULL REFERENCES roles(role_id),
  password_hash TEXT NOT NULL,
  password_salt TEXT NOT NULL,
  password_iterations INTEGER NOT NULL DEFAULT 210000 CHECK(password_iterations >= 100000),
  recovery_question TEXT NOT NULL DEFAULT '',
  recovery_hash TEXT NOT NULL DEFAULT '',
  recovery_salt TEXT NOT NULL DEFAULT '',
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1)),
  must_change_password INTEGER NOT NULL DEFAULT 0 CHECK(must_change_password IN (0,1)),
  approved INTEGER NOT NULL DEFAULT 1 CHECK(approved IN (0,1)),
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS student_profiles (
  user_id INTEGER PRIMARY KEY REFERENCES users(user_id) ON DELETE CASCADE,
  programme TEXT NOT NULL DEFAULT '',
  study_year INTEGER
);
CREATE TABLE IF NOT EXISTS lecturer_profiles (
  user_id INTEGER PRIMARY KEY REFERENCES users(user_id) ON DELETE CASCADE,
  title TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS laboratories (
  lab_id INTEGER PRIMARY KEY AUTOINCREMENT,
  lab_name TEXT NOT NULL,
  lab_number TEXT NOT NULL UNIQUE,
  location TEXT NOT NULL DEFAULT '',
  department TEXT NOT NULL DEFAULT '',
  capacity INTEGER NOT NULL CHECK(capacity > 0),
  description TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'Available' CHECK(status IN ('Available','Unavailable','Under Maintenance')),
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1)),
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS equipment (
  equipment_id INTEGER PRIMARY KEY AUTOINCREMENT,
  lab_id INTEGER NOT NULL REFERENCES laboratories(lab_id),
  equipment_name TEXT NOT NULL,
  total_qty INTEGER NOT NULL CHECK(total_qty >= 0),
  available_qty INTEGER NOT NULL CHECK(available_qty >= 0 AND available_qty <= total_qty),
  condition TEXT NOT NULL DEFAULT 'Good',
  status TEXT NOT NULL DEFAULT 'Available' CHECK(status IN ('Available','Unavailable','Maintenance')),
  notes TEXT NOT NULL DEFAULT '',
  active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0,1)),
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS bookings (
  booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
  reference TEXT NOT NULL UNIQUE,
  user_id INTEGER NOT NULL REFERENCES users(user_id),
  lab_id INTEGER NOT NULL REFERENCES laboratories(lab_id),
  booking_date TEXT NOT NULL,
  start_time TEXT NOT NULL,
  end_time TEXT NOT NULL,
  purpose TEXT NOT NULL,
  status TEXT NOT NULL DEFAULT 'Pending' CHECK(status IN ('Pending','Approved','Rejected','Cancelled','Completed')),
  reviewed_by INTEGER REFERENCES users(user_id),
  decision_note TEXT NOT NULL DEFAULT '',
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL,
  CHECK(start_time < end_time)
);
CREATE TABLE IF NOT EXISTS booking_approvals (
  approval_id INTEGER PRIMARY KEY AUTOINCREMENT,
  booking_id INTEGER NOT NULL REFERENCES bookings(booking_id) ON DELETE CASCADE,
  admin_id INTEGER NOT NULL REFERENCES users(user_id),
  action TEXT NOT NULL CHECK(action IN ('Approved','Rejected','Cancelled','Completed')),
  comment TEXT NOT NULL DEFAULT '',
  action_time TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS booking_equipment (
  booking_id INTEGER NOT NULL REFERENCES bookings(booking_id) ON DELETE CASCADE,
  equipment_id INTEGER NOT NULL REFERENCES equipment(equipment_id),
  qty_requested INTEGER NOT NULL CHECK(qty_requested > 0),
  PRIMARY KEY(booking_id,equipment_id)
);
CREATE TABLE IF NOT EXISTS audit_log (
  log_id INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id INTEGER REFERENCES users(user_id),
  action TEXT NOT NULL,
  entity_type TEXT NOT NULL DEFAULT '',
  entity_id INTEGER,
  timestamp TEXT NOT NULL,
  description TEXT NOT NULL DEFAULT ''
);
CREATE TABLE IF NOT EXISTS booking_schedule (
  id INTEGER PRIMARY KEY CHECK(id = 1),
  morning_start TEXT NOT NULL DEFAULT '08:30',
  afternoon_start TEXT NOT NULL DEFAULT '13:30'
);
INSERT OR IGNORE INTO booking_schedule(id,morning_start,afternoon_start) VALUES(1,'08:30','13:30');
CREATE TABLE IF NOT EXISTS closed_dates (closed_date TEXT PRIMARY KEY);

CREATE INDEX IF NOT EXISTS idx_users_eid ON users(eid);
CREATE INDEX IF NOT EXISTS idx_users_name ON users(name);
CREATE INDEX IF NOT EXISTS idx_bookings_lab_time ON bookings(lab_id,booking_date,start_time,end_time,status);
CREATE INDEX IF NOT EXISTS idx_bookings_user ON bookings(user_id,status,booking_date);
CREATE INDEX IF NOT EXISTS idx_booking_equipment ON booking_equipment(equipment_id,booking_id);
CREATE INDEX IF NOT EXISTS idx_booking_approvals_booking ON booking_approvals(booking_id,action_time);
