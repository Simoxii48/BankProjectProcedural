# 🏦 Bank Management System — Procedural C++

A fully-featured, console-based Bank Management System built in **C++ using a procedural programming paradigm**. The project simulates a real-world banking environment with a multi-user login system, granular role-based permissions, full transaction history, and persistent file-based storage.

> ⚠️ **Platform Note:** This project is built for **Windows** and uses Windows-specific APIs (`<windows.h>`, `<conio.h>`). It is not cross-platform.

---

## 📽️ Demo

> *Video demo coming soon — link will be added here.*

---

## ✨ Features

### 🔐 Authentication & Security
- Secure login with **password hashing** (Caesar cipher-based)
- **Account locking/unlocking** — locked users cannot log in
- Admin-only protection for sensitive operations
- **Action logs** for full audit trail of every user action

### 👥 User Management *(Admin Panel)*
- Add, update, delete, and search users
- **Granular 3-tier permission system:**
  - **Client Permissions** — control who can view, add, delete, update, or find clients
  - **Transaction Permissions** — control access to deposit, withdraw, transfer, and balance views
  - **Manage Users Permissions** — control who can manage the user panel itself
- Grant full permissions or configure them individually per user
- Revoke specific permissions without resetting all
- View **Action Logs** (paginated, 20 records per page)

### 🏦 Client Management
- Add, update, delete, and search bank clients
- Each client has: Account Number, PIN, Full Name, Phone Number, Balance
- Duplicate account number validation on insert

### 💸 Transactions
- **Deposit** — add funds to any account
- **Withdraw** — with balance validation (no overdraft)
- **Transfer** — between two accounts with balance checks
- **Total Balances** — view all accounts and their sum
- **Transaction Log** — full history of deposits and withdrawals
- **Transfer Log** — full history of all transfers

### 💾 Persistent File Storage
All data is saved to flat text files using a `#//# ` delimiter:

| File | Contents |
|------|----------|
| `clients.txt` | All client records |
| `users.txt` | All user accounts and permissions |
| `transactions.txt` | Deposit & withdrawal log |
| `transfers.txt` | Transfer history |
| `actionsLog.txt` | Full audit log of user actions |

---

## 🗂️ Project Structure

```
BankProjectProcedural/
├── Bank Project.cpp    # Main source file — all logic
├── myLib.h             # Helper library / utilities
```

---

## 🔧 How to Build & Run

### Requirements
- Windows OS
- A C++ compiler supporting C++11 or later (e.g. **MSVC** via Visual Studio, or **MinGW/g++**)

### Visual Studio
1. Clone the repository
2. Open `Bank Project.cpp` in Visual Studio
3. Build and run (`Ctrl+F5`)

### Command Line (MinGW/g++)
```bash
g++ "Bank Project.cpp" -o BankApp.exe
./BankApp.exe
```

---

## 🚀 Default Admin Credentials

On first run, ensure a `users.txt` file exists with an Admin entry. The default admin account is:

| Field | Value |
|-------|-------|
| Username | `Admin` |
| Password | *(set in your users file)* |

> The `Admin` user cannot be deleted or locked, and is the only account allowed to revoke permissions and manage locks.

---

## 🧠 Key Concepts Demonstrated

- **Procedural C++ design** — no classes, pure function-driven architecture
- **Bitwise permission flags** — permissions encoded as bitmasks for efficient storage and checks
- **File I/O** — custom serialization/deserialization with delimiter-based parsing
- **Input validation** — type checking, range validation, and loop-until-valid patterns
- **Password masking** — real-time `*` display using `_getch()`
- **GUID-like ID generation** — random numeric IDs for transactions and transfers
- **Audit logging** — every action is timestamped and written to a log file

---

## 📌 Permissions System (Bitmask Design)

Permissions are stored as integers and checked with bitwise AND:

```cpp
// Example permission flags
ShowClientsListPer  = 1 << 0   // 0000 0001
AddNewClientPer     = 1 << 1   // 0000 0010
DeleteClientPer     = 1 << 2   // 0000 0100
...

// Checking permission
bool hasPermission(int userPermission, int required) {
    return (userPermission & required) != 0;
}
```

This allows combining and revoking permissions efficiently without string comparisons or multiple boolean fields.

---

## 🔮 Potential Improvements

- [ ] Migrate to OOP (class-based) design
- [ ] Cross-platform support (remove Windows dependencies)
- [ ] Replace file storage with SQLite or similar
- [ ] Stronger password hashing (bcrypt / SHA-256)
- [ ] Add interest calculation and account types
- [ ] Export reports to CSV

---

## 👤 Author

**Mohammed Hejjam**
[LinkedIn](https://www.linkedin.com/in/mohammed-hejjam-755472173) · [GitHub](https://github.com/Simoxii48)

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
