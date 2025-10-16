# ♟️ Chanakya — UCI Chess Engine

A lightweight **UCI-compatible chess engine** written in C++.  
Supports **FEN**, **Polyglot opening book**, **time controls**, and standard UCI commands.  

---

## 🚀 Features

- Full **UCI protocol support**  
- Supports `startpos` and `FEN` positions  
- Adjustable **search depth** and **time controls**  
- Polyglot **opening book moves**  
- Board display (`d`) and manual moves (`move`) for testing  
- Undo moves (`undo`)  
- Threaded search for faster performance  

---

## 🕹️ UCI Commands

Below is a full list of commands supported by the engine with **examples**.

---

### 1️⃣ `uci`  
**Initialize the engine in UCI mode.**  

```bash
uci
id name MyChessEngine
id author Abhijeet Singh
uciok
```

### 2️⃣ `isready`  
**Check if the engine is ready.**  

```bash
isready
readyok
```

### 3️⃣ `setoption`  
**Set engine options**  

```bash
setoption name usebook value true
book move on
```

### 4️⃣ `ucinewgame`

**Start a new game.**
Resets internal search state and clears previous positions.
```bash
ucinewgame
```

### 5️⃣ `position`

**Set board position.**

- From starting position:
  ```bash
  position startpos
  position startpos moves e2e4 e7e5 g1f3
  ```
- From FEN:
  ```bash
  position fen position fen rn3rk1/pp1q1ppp/4pn2/1b1p4/2p2N2/P1B1P3/2PPBPPP/1R1Q1RK1 w - - 4 13
  ```

### 6️⃣ `go`

**Start searching for the best move.**

a) Search by depth
  ```bash
  go depth 10
  ```

b) Search by fixed time
  ```bash
  go movetime 2000
  ```
c) Search with remaining time
  ```bash
  go wtime 300000 btime 300000 winc 2000 binc 2000 movestogo 30
  ```

- `wtime` / `btime` → milliseconds left for white/black
- `winc` / `binc` → increment per move (ms)
- `movestogo` → moves until next time control

**Response:**
```base
info score cp 52 depth 1 ordering 0.00 nodes 48 time 1 pv d7d5 
info score cp -20 depth 2 ordering 83.61 nodes 205 time 5 pv d7d5 d2d4 
info score cp 39 depth 3 ordering 84.86 nodes 1880 time 11 pv d7d5 d2d4 f8b4 
info score cp -56 depth 4 ordering 79.62 nodes 14784 time 33 pv d7d5 f3e5 d5e4 d2d4 
info score cp 0 depth 5 ordering 82.11 nodes 72036 time 115 pv g8f6 d2d4 f6e4 f3e5 d7d5 
info score cp -36 depth 6 ordering 88.22 nodes 429954 time 624 pv d7d5 
info score cp 5 depth 7 ordering 86.80 nodes 1342069 time 1801 pv g8f6
```

### 7️⃣ stop

**Stop current search immediately.**
```base
stop
```
- Engine returns the bestmove found so far

### 8️⃣ quit

**Exit the engine.**
```bash
quit
```

### 9️⃣ Debug & Manual Commands
| Command | Description | Example |
|---------|-------------|---------|
| `d` | Print current board | `uci` |
| `move <move>` | Make a manual move (UCI format) | `move e2e4` |
| `undo` | Take back last move | `undo` |
| `book` | Load Polyglot book manually | `book` |

