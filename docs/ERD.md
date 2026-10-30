# ER structure

```text
ROLES 1──* USERS 1──0..1 STUDENT_PROFILES
                  └──0..1 LECTURER_PROFILES
USERS 1──* BOOKINGS *──1 LABORATORIES 1──* EQUIPMENT
BOOKINGS 1──* BOOKING_EQUIPMENT *──1 EQUIPMENT
```

`Pending` and `Approved` bookings reserve their laboratory interval. Equipment availability is reserved by the same booking transaction. `Rejected` and `Cancelled` records remain historical and do not block future availability.
