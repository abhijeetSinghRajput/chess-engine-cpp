<div align="center">

  <img src="/assets/logo-512.png" alt="Chanakya logo" width="128" height="128" />

  <h3>Chanakya (~2500 ELO)</h3>

  <p>
    A Open Source lightweight <strong>chess engine</strong> written in C++
  </p>

  <p align="center">
    <a href="https://github.com/abhijeetSinghRajput/chanakya">
      <img alt="made by" src="https://img.shields.io/badge/MADE_BY-ABHIJEET SINGH RAJPUT-4a4a4a?style=for-the-badge" />
    </a>
  </p>
  <p align="center">
    <img alt="license" src="https://img.shields.io/badge/LICENSE-GPL--3.0-38d430?style=for-the-badge" />
    <img alt="language" src="https://img.shields.io/badge/LANGUAGE-C%2B%2B-0078d4?style=for-the-badge&logo=cplusplus&logoColor=white" />
    <img alt="elo" src="https://img.shields.io/badge/ELO-2450±20-00a2ff?style=for-the-badge" />
    <img alt="uci" src="https://img.shields.io/badge/UCI-COMPATIBLE-38d430?style=for-the-badge" />
  </p>

  <br>
  <br>
  <br>

  <p align="center">
    <a href="https://marketplace.visualstudio.com/items?itemName=mrcodium.chess-engine">
      <img
        src="https://github.com/abhijeetSinghRajput/github-contributions-calendar/blob/main/media/screenshots/install-from-marketplace.png?raw=true"
        alt="Install from VS Code Marketplace"
        height="28"
        style="vertical-align: middle;"
      />
    </a>
  </p>

  <p align="center">
    Install the 
    <a href="https://marketplace.visualstudio.com/items?itemName=mrcodium.chess-engine">VS Code extension</a>
    and run it against the engines.
  </p>

  <img src="assets/chess-vscode-demo.gif" alt="GitHub Contributions Calendar logo" width="600"  />
</div>

## Table of Contents

- [About](#about)
- [Features](#features)
- [Installation](#installation)
- [Engine Strength](#engine-strength)
- [Supported Commands](#supported-commands)
- [Development](#development)
- [Contributing](#contributing)
  - [What's Already Implemented](#whats-already-implemented-confirmed)
  - [What You Can Contribute](#-what-you-can-contribute)
  - [PR Merge Policy](#pr-merge-policy)
- [License](#license)

## About

Chanakya is a lightweight chess engine written in modern C++ that fully
implements the UCI protocol. It supports custom FEN positions, Polyglot
opening books, configurable search depth and time controls, and ships with a
dedicated Visual Studio Code extension for playing and analyzing games
directly inside your editor.

The engine is designed to be simple, fast, and easy to integrate into chess
GUIs and tooling — and to be a friendly, well-documented codebase for anyone
who wants to learn how a real alpha-beta engine is put together, or to
contribute a search/eval improvement of their own. See
[`ARCHITECTURE.md`](ARCHITECTURE.md) for a full technical map of the engine.

## Features

- Full UCI protocol support
- Supports `startpos` and arbitrary FEN positions
- Adjustable search depth and time controls
- Polyglot opening book support
- Undo support (`undo`)
- Board visualization (`d`)
- Manual move testing (`move`)
- Dedicated VS Code GUI
- Lightweight, modern C++ implementation

## Installation 

Chanakya can be used in two ways:

1. Through the VS Code extension (recommended)
2. Directly from the command line

### Option 1: VS Code Extension

Install the official VS Code extension to play and analyze games directly
inside your editor.

[Install from VS Code Marketplace](#)

**Steps**
1. Open VS Code.
2. Open the Extensions panel (`Ctrl+Shift+X`).
3. Search for **Chanakya Chess Engine**.
4. Click **Install**.
5. Open the extension and start playing.

### Option 2: Build from Source (CLI)

```bash
git clone https://github.com/abhijeetSinghRajput/chess-engine-cpp.git
cd chess-engine-cpp
make
```

Run the engine:

```bash
# Linux / macOS
./chanakya

# Windows
chanakya.exe
```

Test the engine:

```
uci
isready
position startpos
go depth 10
```

## Engine Strength

Chanakya has been benchmarked at approximately:

**⭐ ~2499 Elo (± 20.6)** — 1000 games, 60+0.6 time control, using
cutechess-cli and Ordo's Bayesian anchored rating calculation against
Stockfish 18 throttled to 2500 Elo.

| Player   | Rating | Error | Points | Played | Score % |
|----------|-------:|------:|-------:|-------:|--------:|
| SF-2500  | 2500.0 |   —   |  501.5 |   1000 |   50%   |
| Chanakya | 2498.9 |  20.6 |  498.5 |   1000 |   50%   |

White advantage = 41.20 ± 10.68 &nbsp;·&nbsp; Draw rate (equal opponents) = 4.94% ± 0.70

<details>
<summary>Previous benchmark (for reference)</summary>

| Metric                | Value                       |
|------------------------|------------------------------|
| Rating                 | 2450 Elo                    |
| Error margin (95% CI)  | ± 20.4                      |
| Games played           | 1,000                       |
| Opponents               | Stockfish 18 @ 2400 Elo      |
| Time control            | 60+0.6                      |
| Rating method            | Ordo Bayesian estimation     |

</details>

**Methodology**
- Ratings are relative to Stockfish's `UCI_LimitStrength`.
- Chanakya's internal opening book was disabled.
- Both engines used a shared randomized opening book.
- Time-forfeit games from an earlier build were excluded.
- This is not an official CCRL or CEGT rating.

## Supported Commands

| Command              | Description               |
|-----------------------|----------------------------|
| `uci`                 | Initialize UCI mode       |
| `isready`             | Check readiness           |
| `position startpos`   | Load starting position    |
| `position fen ...`    | Load custom FEN           |
| `go depth N`          | Search to depth           |
| `go movetime N`       | Search for milliseconds   |
| `move e2e4`           | Make a move                |
| `undo`                | Undo last move             |
| `d`                   | Print board                |

## Development

```bash
make
./chanakya
```

```
uci
isready
position startpos
go depth 10
```

For an in-depth technical map of the engine (board representation, move
encoding, search, eval, TT) before you dive into a change, read
[`ARCHITECTURE.md`](ARCHITECTURE.md) first — it also lists the specific
things you should **not** assume about the codebase.

## Contributing

Contributions are always welcome — from a one-line eval tweak to a full
search feature. This section is a living map of what's done and what's
genuinely useful to work on next. Please open an issue first if you'd like
to discuss a major change before writing code.

**Workflow:** fork → feature branch → commit → push → open a PR. See
[PR Merge Policy](#pr-merge-policy) below for how changes get accepted.

### What's Already Implemented (Confirmed)

**Core Engine**
- Bitboard magic generation — fully functional rook/bishop attack lookups
- Zobrist hashing — position key with piece, en passant, castle, side keys
- Transposition table — with depth, flag, and score extraction
- Complete move generation — all pieces including pawns with promotions, en
  passant, castling
- Move validation — legal move checking with make/take move
- UCI protocol — core commands implemented

**Search**
- Alpha-beta with iterative deepening
- Reverse futility (static null-move) pruning
- Null move pruning
- Late move reduction (LMR) with re-search
- Quiescence search with delta pruning
- Killer moves (2 per ply)
- Countermove heuristic
- History heuristic with aging (20% decay per search)
- Repetition detection and the fifty-move rule
- SEE (Static Exchange Evaluation)

**Evaluation**
- Piece-square tables for all pieces
- Mobility (binary midgame/endgame switch, not tapered — see
  [`ARCHITECTURE.md` §8](ARCHITECTURE.md#8-evaluation-evalevaluationcpp))
- Pawn structure (isolated, passed, doubled, support)
- Rook-specific bonuses (open files, support, 7th rank)
- King safety (pawn shield, storm, zone attacks)
- Bishop pair
- Material-based draw detection

### What You Can Contribute

Everything below is confirmed **not yet implemented** — no partial/half-done
code to work around, just a clean slot to fill.

**1. Search**

| Feature | Expected Impact |
|---|---|
| Principal Variation Search (PVS) | 15–20% speedup |
| Aspiration Windows | 10–15% speedup |
| Internal Iterative Deepening (IID) | Better move ordering at depth |
| Razor Pruning | 5–10% speedup |
| Futility Pruning (forward) | 5–10% speedup |
| Singular Extensions | Position-specific improvements |
| Mate Distance Pruning | Faster mate detection |
| Multi-Cut Pruning | Aggressive pruning at higher depths |

**2. Transposition Table**

| Feature | Expected Impact |
|---|---|
| TT Prefetching | 5–10% NPS increase |
| TT Probing in Quiescence | Reduce quiescence work |
| TT Hit Statistics | Tuning/debugging visibility |
| 2-Entry TT Slots | Better replacement behavior |

**3. Evaluation**

| Feature | Expected Impact |
|---|---|
| Texel / SPSA Tuning Framework | Optimize all PSTs and constants at once |
| Tapered (phase-interpolated) Eval | Smoother midgame→endgame transition |
| Connected Passed Pawns | Stronger endgame play |
| King Tropism | Better king safety |
| Outpost Squares | Better piece placement |
| Pawn Chains / Backward Pawns | Pawn-structure refinement |
| Trapped Piece Penalty | Avoid bad trades |
| Space Advantage | Positional control |
| King Activity (endgame) | Better king usage in the endgame |
| Bishop-Pair Opposite-Color Check | More accurate bishop-pair bonus |
| Endgame Opposition | Pawn endgame precision |

**4. Move Ordering & Generation**

| Feature | Expected Impact |
|---|---|
| SEE in Move Ordering | Better capture ordering |
| Killer / History / Countermove Tuning | More effective move ordering |
| Pawn Push Ordering | Minor improvement |

**5. UCI & Engine Features**

| Feature | Expected Impact |
|---|---|
| Fuller UCI Compliance | Wider GUI compatibility |
| Better Time Management | Fewer flags, better clock usage |
| Multi-Threading (Lazy SMP) | 2–3x speedup on multi-core |
| Syzygy Tablebase Support | Perfect endgame play |
| Opening Book Selection | Better/varied opening play |
| Benchmark Mode | Standardized performance testing |

**Suggested priority order**, if you're not sure where to start:
1. PVS + Aspiration Windows
2. SEE
3. Tapered evaluation
4. Lazy SMP (4 cores)
5. TT improvements (prefetch, quiescence probing)

### PR Merge Policy

**A PR is only merged if it's measurably better than the current version.**
"Better" is decided by self-play testing on the same hardware and the same
engine options, not by code review alone — a plausible-sounding idea that
doesn't hold up in an SPRT gets rejected just as fast as an unplausible one.

**Testing setup:** [cutechess-cli](https://github.com/cutechess/cutechess),
same machine, same `Hash`/thread settings for both sides, at all three of:

```
60+0.6
30+0.3
15+0.15
```

**Draw/resign adjudication** (used in every test match so games don't drag
on pointlessly):

```powershell
"-draw", "movenumber=80", "movecount=8", "score=5",
"-resign", "movecount=3", "score=600"
```

**Example overnight run** (candidate build vs. a Stockfish anchor — adapt
paths/engine names for a build-vs-build test):

```powershell
# =========================================================
# Chanakya Selfplay Test
# =========================================================

$CUTECHESS = "C:\path\to\cutechess-cli.exe"
$CHANAKYA  = "C:\path\to\chanakya.exe"
$CHANAKYA_new = "C:\path\to\chanakya_new.exe"
$BOOK      = "C:\path\to\8moves_v3.pgn"

$TC = "60+0.6"
$CONCURRENCY = 4

$matchArgs = @(
    "-engine", "cmd=$CHANAKYA", "name=Chanakya", "option.Hash=128", "option.usebook=false",
    "-engine", "cmd=$CHANAKYA_new", "name=Chanakya-New", "option.Hash=128", "option.usebook=false",
    "-each", "proto=uci", "tc=$TC",
    "-openings", "file=$BOOK", "format=pgn", "order=random", "plies=16",
    "-rounds", "500",
    "-repeat",
    "-games", "2",
    "-concurrency", "$CONCURRENCY",
    "-pgnout", "chanakya_selfplay.pgn",
    "-ratinginterval", "10",
    "-recover",
    "-maxmoves", "200",
    "-draw", "movenumber=80", "movecount=8", "score=5",
    "-resign", "movecount=3", "score=600"
)

& $CUTECHESS $matchArgs 2>&1 | Tee-Object -FilePath "chanakya_selfplay.txt"

# Rate the result:
# .\ordo.exe -a 2500 -A "SF-2500" -p chanakya_selfplay.pgn -o chanakya_selfplay_rating.txt -W -s 1000 -D
```

If you're proposing a search or eval change, run your build against the
current `main` at all three time controls above (build-vs-build, not vs.
Stockfish) and include the SPRT/Ordo result in your PR description. Small,
isolated changes are much easier to validate — and much easier to merge —
than a PR that bundles five ideas at once.

## Thanks and Acknowledgements
 
Chanakya wouldn't exist without the chess programming community's tradition
of teaching in public. Particular thanks to:
 
- **[Sebastian Lague](https://www.youtube.com/watch?v=U4ogK0MIzqk)** — the video that started it all. This
  is where the interest in building a chess engine actually began.
- **[Harvard CS50's AI with Python](https://www.youtube.com/watch?v=5NgNicANyqM)** — the adversarial-search
  (minimax) lecture was my first real introduction to minimax, via a
  tic-tac-toe AI. Tic-tac-toe tops out at roughly 5,000 possible game states;
  chess has something like 10^123 — learning that gap is what made chess
  search genuinely interesting instead of just a class exercise, and led
  straight back to Sebastian Lague's video above.
- **[VICE](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)** (Bluefever Software) — Bluefever's video series on building a chess
  engine from scratch is the reason a lot of this codebase's structure looks
  the way it does; an essential starting point for understanding board
  representation, move generation, and search in a real engine.
- **[BBC](https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs)** (Bit-Board Chess, by Code Monkey King) — the bitboard/magic-bitboard
  tutorial series that shaped how Chanakya's attack generation and move
  encoding work.
- **[cutechess](https://github.com/cutechess/cutechess)** — the engine-vs-engine testing tool behind every
  self-play match and SPRT run referenced in this repo's [PR merge policy](#pr-merge-policy).
- **[Ordo](https://github.com/michiguel/Ordo)** — the Bayesian rating tool used to turn cutechess PGN
  output into the Elo numbers you see in [Engine Strength](#engine-strength).
If you're new to engine programming, working through VICE and BBC before
your first Chanakya PR will save you a lot of time.
 

## License

Released under the [GPL-3.0 License](LICENSE).