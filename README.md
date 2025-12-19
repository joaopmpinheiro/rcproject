# Event Reservation Platform

A C-based client-server event reservation system for the **Redes de Computadores (RC)** course at IST. Uses raw BSD sockets with UDP for user management and listing activities, and TCP for file transfers and event/reservation management.

**Group:** 32 **Deadline:** December 19, 2025

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Installation](#installation)
- [Build](#build)
- [Usage](#usage)
- [Protocol Documentation](#protocol-documentation)
- [License](#license)

## Features

- **User Management:** Register, login, logout, and unregister users (UDP)
- **Event Creation:** Create events with descriptions and seat limits (TCP)
- **Event Reservations:** Reserve seats for upcoming events (TCP)
- **Event Listing:** Browse all available events or personal event history (UDP/TCP)
- **Event Closure:** Event organizers can close events (TCP)
- **Password Management:** Change account passwords (TCP)
- **Multiplexed Server:** Single server handles both UDP and TCP using `select()`

## Project Structure

```
.
├── README.md                    # This file
├── Makefile                     # Top-level build configuration
│
├── common/                      # Shared code between client and server
│   ├── common.c/.h              # TCP/UDP utilities, message handling
│   ├── data.h                   # Enums (RequestType, ReplyStatus)
│   ├── parser.c/.h              # Common parsing utilities
│   ├── verifications.c/.h       # Input validation functions
│   └── Makefile                 # Build configuration for common lib
│
├── server/                      # Event-Reservation Server (ES)
│   ├── Makefile                 # Build configuration
│   ├── ES                       # Compiled executable
│   ├── include/
│   │   ├── globals.h            # Server settings and request structs
│   │   └── utils.h              # Server function declarations
│   ├── src/
│   │   ├── server.c             # Main entry point, select() loop
│   │   └── utils/               # Handler implementations
│   │       ├── command_handler.c    # UDP/TCP protocol handlers
│   │       ├── connection.c         # Connection setup, arg parsing
│   │       ├── error.c              # Error handling, logging
│   │       ├── socket_manager.c     # select_handler(), UDP/TCP setup
│   │       ├── file_manager.c       # File/directory operations
│   │       ├── users_manager.c      # User persistence
│   │       └── events_manager.c     # Event management
│   ├── USERS/                   # User data storage
│   │   └── <UID>/               # Per-user directory
│   │       ├── <UID>password.txt    # Stored password
│   │       ├── <UID>login.txt       # Login marker (exists = logged in)
│   │       ├── CREATED/             # Events created by user
│   │       └── RESERVED/            # User's reservations
│   └── EVENTS/                  # Event data storage
│       └── <EID>/               # Per-event directory (e.g., 001)
│           ├── START_<EID>.txt      # Event metadata
│           ├── END_<EID>.txt        # Closure marker
│           ├── RES_<EID>.txt        # Reserved seats count
│           └── DESCRIPTION/         # Event description files
│
└── user/                        # User Client Application
    ├── Makefile                 # Build configuration
    ├── user                     # Compiled executable
    ├── include/
    │   ├── client_data.h        # Global session state
    │   └── utils.h              # Client function declarations
    └── src/
        ├── user.c               # Main entry point, command loop
        └── utils/               # Command implementations
            ├── command_handler.c    # Command dispatcher
            ├── commands.c           # All user command handlers
            ├── messages.c           # User-facing output messages
            ├── read_from_server.c   # TCP response parsing
            ├── socket_manager.c     # UDP/TCP connection utilities
            └── user_parser.c        # Input parsing
```

## Installation

### Prerequisites

- **OS:** Linux/Unix/macOS
- **Compiler:** GCC or Clang with C99+ support
- **Build Tool:** GNU Make
- **C Libraries:** POSIX-compliant (BSD sockets, standard C library)

### Steps

1. **Clone or extract the project:**

   ```bash
   cd /path/to/project
   ```

2. **Verify structure:**

   ```bash
   ls -la
   # Should show: common/, server/, user/, Makefile
   ```

3. **Initialize required directories (for server, if somehow they don't exist):**

   ```bash
   cd server
   mkdir -p USERS EVENTS
   cd ..
   ```

   These directories are **essential** — the server stores all user data and event information in them.

## Build

### Build All Components

From the project root:

```bash
make
```

This compiles:

1. Common library (`common/`)
2. Server executable (`server/ES`)
3. User client executable (`user/user`)

### Build Individual Components

```bash
make -C common      # Build only common library
make -C server      # Build only server (requires common built first)
make -C user        # Build only user client (requires common built first)
```

### Clean Build Artifacts

```bash
make clean          # Remove all .o, .a, and executable files
```

After building successfully, you'll have:

- `server/ES` — Server executable
- `user/user` — Client executable

## Usage

### Start the Server

Navigate to the `server/` directory first:

```bash
cd server/

# Default configuration (port 58032, non-verbose)
./ES

# With custom port
./ES -p 59999

# Verbose mode (prints all requests with UID, command type, client IP:port)
./ES -v

# Custom port + verbose
./ES -p 59999 -v
```

The server will start a `select()` loop listening on the specified port for both UDP and TCP connections.

### Start the User Client

Navigate to the `user/` directory first:

```bash
cd user/

# Connect to default server (localhost:58032)
./user

# Connect to custom server
./user -n 192.168.1.100 -p 59999

# Options:
#   -n ESIP    Server IP address (default: localhost)
#   -p ESport  Server port (default: 58032)
```

### User Commands

| Command                            | Protocol | Description                                        |
| ---------------------------------- | -------- | -------------------------------------------------- |
| `login UID password`               | UDP      | Login or register new user                         |
| `logout`                           | UDP      | Logout current user                                |
| `unregister`                       | UDP      | Unregister and logout                              |
| `myevents` / `mye`                 | UDP      | List your created events                           |
| `myreservations` / `myr`           | UDP      | List your reservations (max 50 recent)             |
| `create name fname date attendees` | TCP      | Create event with file and date (DD-MM-YYYY HH:MM) |
| `close EID`                        | TCP      | Close event (stop reservations)                    |
| `list`                             | TCP      | List all available events                          |
| `show EID`                         | TCP      | Show event details and download file               |
| `reserve EID seats`                | TCP      | Reserve seats for event                            |
| `changePass oldPwd newPwd`         | TCP      | Change account password                            |
| `exit`                             | Local    | Exit application (must logout first)               |

### Example Session

```bash
# Terminal 1: Start server
cd /PATH/TO/PROJECT/server
./ES -v

# Terminal 2: Start user client
cd /PATH/TO/PROJECT/user
./user

> login 123456 password1
> create MyEvent event.txt 25-12-2025 14:30 100
> list
> reserve 001 5
> myevents
> logout
> exit
```

## Protocol Documentation

### Message Format

All protocol messages end with `\n`. Fields are space-separated.

### UDP Commands (User ↔ Server)

| Command         | Request            | Response                       | Status Codes      |
| --------------- | ------------------ | ------------------------------ | ----------------- |
| Login           | `LIN UID password` | `RLI status`                   | OK, NOK, REG      |
| Logout          | `LOU UID password` | `RLO status`                   | OK, NOK, UNR, WRP |
| Unregister      | `UNR UID password` | `RUR status`                   | OK, NOK, UNR, WRP |
| My Events       | `LME UID password` | `RME status [EID state]*`      | OK, NOK, NLG, WRP |
| My Reservations | `LMR UID password` | `RMR status [EID date value]*` | OK, NOK, NLG, WRP |

**Event States:** 0=past, 1=accepting, 2=sold out, 3=closed

### TCP Commands (User ↔ Server)

| Command     | Request                                       | Response                                                    | Status Codes                           |
| ----------- | --------------------------------------------- | ----------------------------------------------------------- | -------------------------------------- |
| Create      | `CRE UID pwd name date seats fname size data` | `RCE status [EID]`                                          | OK, NOK, NLG, WRP                      |
| Close       | `CLS UID pwd EID`                             | `RCL status`                                                | OK, NOK, NLG, NOE, EOW, SLD, PST, CLO  |
| List        | `LST`                                         | `RLS status [EID name state date]*`                         | OK, NOK                                |
| Show        | `SED EID`                                     | `RSE status [UID name date seats reserved fname size data]` | OK, NOK                                |
| Reserve     | `RID UID pwd EID seats`                       | `RRI status [n_seats]`                                      | ACC, REJ, CLS, SLD, PST, NOK, NLG, WRP |
| Change Pass | `CPS UID oldPwd newPwd`                       | `RCP status`                                                | OK, NOK, NLG, NID                      |

### Status Codes

**Note:** Status codes mean different things depending on context and command. The meanings below are common interpretations; refer to the protocol tables above for exact behavior in each command.

| Code | Meaning (in most cases)                 |
| ---- | --------------------------------------- |
| OK   | Operation successful                    |
| NOK  | Generic failure                         |
| REG  | New user registered                     |
| NLG  | User not logged in                      |
| WRP  | Wrong password                          |
| UNR  | User not registered                     |
| NID  | User does not exist                     |
| NOE  | Event does not exist                    |
| EOW  | Event not owned by user                 |
| SLD  | Event sold out                          |
| PST  | Event date has passed                   |
| CLO  | Event already closed                    |
| CLS  | Event closed (for reservations)         |
| ACC  | Reservation accepted                    |
| REJ  | Reservation rejected (not enough seats) |
| ERR  | Syntax or invalid parameter error       |

### Format Constraints

| Field       | Format                                                  |
| ----------- | ------------------------------------------------------- |
| UID         | 6 digits (student number)                               |
| EID         | 3 digits (001-999)                                      |
| Password    | 8 alphanumeric characters                               |
| Event Name  | Max 10 alphanumeric chars                               |
| Event Date  | DD-MM-YYYY HH:MM (or DD-MM-YYYY HH:MM:SS in some cases) |
| Filename    | Max 24 chars (alphanumeric + `-`, `_`, `.`)             |
| File Size   | Max 10MB                                                |
| Attendees   | 10-999                                                  |
| Reservation | 1-999 people                                            |

## Development Notes

- **Error Handling:** Never crash on invalid input; return appropriate error codes
- **Partial I/O:** `read()` and `write()` may transfer fewer bytes than requested—use loops
- **Verbose Mode:** Run server with `-v` to debug protocol interactions
- **Socket State:** Server uses `select()` for multiplexing; TCP connections stay open until client closes
- **Data Persistence:** All user data and event information is stored in the `USERS/` and `EVENTS/` directories on the server

## License

This project is for educational purposes in the RC course at Instituto Superior Técnico (IST).

## Authors

Group 32 - RC Course at IST

- **João Pinheiro** - 109333
- **Marta Braga** - 110034
