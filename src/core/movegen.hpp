// movegen.hpp
#pragma once

inline void addCaptureMove(MoveList &list, int move);
inline void addQuiteMove(MoveList &list, int move);
inline void addEnPassantMove(MoveList &list, int move);

void generateMoves(MoveList &list);
void generateCaptureMoves(MoveList &list);

void generateMoves(MoveList &list);
void generateCaptureMoves(MoveList &list);
