<div align="center">

<img src="/assets/chanakya_logo.png" alt="Chanakya logo" width="96" height="96" />

<h1>Chanakya</h1>

<p>
  A Open Source lightweight <strong>UCI-compatible chess engine</strong> written in C++,
  featuring Polyglot opening books, configurable search, and a dedicated
  VS Code GUI.
</p>

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
  <a href="https://github.com/abhijeetSinghRajput/chanakya">
    <img alt="made by" src="https://img.shields.io/badge/made%20by-abhijeetsinghrajput-blueviolet?style=flat" />
  </a>
  <img alt="license" src="https://img.shields.io/badge/license-GPL--3.0-brightgreen?style=flat" />
  <img alt="language" src="https://img.shields.io/badge/language-C%2B%2B-00599C?style=flat&logo=cplusplus" />
  <img alt="elo" src="https://img.shields.io/badge/Elo-2326%20%C2%B1%2016-red?style=flat" />
  <img alt="uci" src="https://img.shields.io/badge/UCI-compatible-blue?style=flat" />
  <img alt="GitHub stars" src="https://img.shields.io/github/stars/abhijeetSinghRajput/chanakya?style=flat&color=blue" />
  <img alt="GitHub forks" src="https://img.shields.io/github/forks/abhijeetSinghRajput/chanakya?style=flat&color=blue" />
  <img alt="VS Code Marketplace Installs" src="https://vsmarketplacebadges.dev/installs/mrcodium.chess-engine.svg?color=blue" />
  <img alt="VS Code Marketplace Rating" src="https://vsmarketplacebadges.dev/rating/mrcodium.chess-engine.svg?color=blue" />
</p>

<img src="assets/chess-vscode-demo.gif" alt="GitHub Contributions Calendar logo" width="600"  />

</div>

<details>
<summary>Table of Contents</summary>

- [About](#-about)
- [Features](#-features)
- [VS Code GUI](#️-vs-code-gui)
- [Engine Strength](#-engine-strength)
- [Supported Commands](#-supported-commands)
- [Development](#-development)
- [Contributing](#-contributing)
- [License](#-license)

</details>

---

# 📖 About

**Chanakya** is a lightweight chess engine written in modern C++ that fully
implements the **UCI protocol**.

It supports custom **FEN positions**, **Polyglot opening books**, configurable
search depth and time controls, and ships with a dedicated **Visual Studio Code
extension** for playing and analyzing games directly inside your editor.

The engine is designed to be simple, fast, and easy to integrate into chess
GUIs and tooling.

---

# ✨ Features

- Full **UCI protocol** support
- Supports `startpos` and arbitrary **FEN** positions
- Adjustable **search depth** and **time controls**
- Polyglot **opening book** support
- Undo support (`undo`)
- Board visualization (`d`)
- Manual move testing (`move`)
- Dedicated **VS Code GUI**
- Lightweight modern C++ implementation

---

# 🖥️ VS Code GUI

Chanakya includes a Visual Studio Code extension that lets you:

- Play against the engine directly inside VS Code.
- Analyze positions and custom FENs.
- Visualize engine evaluations.
- Test moves and openings.
- Use Chanakya without installing a separate GUI.

### Install

**Marketplace**

https://marketplace.visualstudio.com/items?itemName=mrcodium.chess-engine

**Open directly in VS Code**

```text
vscode:extension/mrcodium.chess-engine
```

---

# 📊 Engine Strength

Chanakya has been benchmarked at approximately:

<div align="center">

# ⭐ 2326 Elo ± 16

**827 games · 60+0.6 time control**

</div>

using `cutechess-cli` and Ordo's Bayesian anchored rating calculation against
**Stockfish 18** throttled to **2300** and **2400 Elo**.

| Metric | Value |
| ------- | ------- |
| Rating | **2325.8 Elo** |
| Error margin (95% CI) | **± 16.1** |
| Games played | **827** |
| Opponents | **Stockfish 18 @ 2300 / 2400 Elo** |
| Time control | **60+0.6** |
| Rating method | **Ordo Bayesian estimation** |

### Methodology

- Ratings are relative to Stockfish's `UCI_LimitStrength`.
- Chanakya's internal opening book was disabled.
- Both engines used a shared randomized opening book.
- Time-forfeit games from an earlier build were excluded.
- This is **not** an official CCRL or CEGT rating.

---

# ⚙️ Supported Commands

| Command | Description |
|----------|----------|
| `uci` | Initialize UCI mode |
| `isready` | Check readiness |
| `position startpos` | Load starting position |
| `position fen ...` | Load custom FEN |
| `go depth N` | Search to depth |
| `go movetime N` | Search for milliseconds |
| `move e2e4` | Make a move |
| `undo` | Undo last move |
| `d` | Print board |

---

# 🔧 Development

```bash
make
./chanakya
```

Example:

```bash
uci
isready
position startpos
go depth 10
```

---

# 🤝 Contributing

Contributions are always welcome.

1. Fork the repository.
2. Create a feature branch.
3. Commit your changes.
4. Push the branch.
5. Open a pull request.

Please open an issue first if you'd like to discuss major changes.

---

# 📄 License

Released under the **GPL-3.0 License**.
