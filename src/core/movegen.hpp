// movegen.hpp
#pragma once

inline void addCaptureMove(MoveList &list, int move);
inline void addQuietMove(MoveList &list, int move);
inline void addEnPassantMove(MoveList &list, int move);

// move ordering score 
constexpr int SCORE_TT_MOVE       = 2'000'000;   // Highest priority
constexpr int SCORE_CAPTURE       = 1'000'000;   // MVV-LVA captures
constexpr int SCORE_KILLER_1      =   900'000;   // First killer
constexpr int SCORE_KILLER_2      =   800'000;   // Second killer
constexpr int SCORE_COUNTER       =   700'000;   // counter move score

void generateMoves(MoveList &list);
void generateCaptureMoves(MoveList &list);

void generateMoves(MoveList &list);
void generateCaptureMoves(MoveList &list);
