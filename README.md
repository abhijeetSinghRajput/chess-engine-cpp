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

## 📊 Engine Strength

Chanakya's playing strength has been benchmarked at approximately
**2326 Elo (± 16)** using [cutechess-cli](https://github.com/cutechess/cutechess)
and [Ordo](https://github.com/michiguel/Ordo)'s Bayesian anchored rating
calculation, based on 827 valid games against Stockfish 18 (throttled to
2300 and 2400 via `UCI_LimitStrength`) at a 60s+0.6s time control.

| Metric | Value |
|---|---|
| Rating | 2325.8 Elo |
| Error margin (95% CI) | ± 16.1 |
| Games played | 827 |
| Opponents | Stockfish 18 @ UCI_Elo 2300 and 2400 |
| Time control | 60+0.6 |
| Rating method | Ordo, multi-anchor Bayesian estimation |

**Methodology notes:**
- This is a self-conducted benchmark relative to Stockfish's own strength
  throttling (`UCI_LimitStrength`), not an official CCRL/CEGT rating.
  Actual tournament or CCRL-list strength may differ.
- Games were played with Chanakya's internal opening book disabled
  (`usebook=false`) so both engines drew openings from a shared,
  randomized 8-move opening book, removing opening bias from the results.
- Games ending in engine time forfeits from an earlier, unpatched build
  were identified and excluded prior to the final rating calculation.