#pragma once

// move structure                       set Mask
//  0000 0000 0000 0000 0000 0111 1111   0x7f          from
//  0000 0000 0000 0011 1111 1000 0000   0x7f          to
//  0000 0000 0011 1100 0000 0000 0000   0xf           capture piece
//  0000 0011 1100 0000 0000 0000 0000   0xf           promoted piece
//  0000 0100 0000 0000 0000 0000 0000   0x40000       enpassant
//  0000 1000 0000 0000 0000 0000 0000   0x80000       castle
//  0001 0000 0000 0000 0000 0000 0000   0x1000000     pawnstart

// flags 🚩
constexpr int EN_PASSANT_FLAG  = 0x400000;
constexpr int CASTLE_FLAG      = 0x800000;
constexpr int PAWN_START_FLAG  = 0x1000000;

constexpr int CAPTURE_FLAG     = 0x3c000;
constexpr int PROMOTION_FLAG   = 0x3c0000;

int buildMove(int from, int to, int capturedPiece, int promotedPiece, int flag);
int moveFrom(int move);
int moveTo(int move);
int moveCapturePiece(int move);
int movePromotionPiece(int move);
void moveDetail(int move);

void movePiece(int from, int to);
void addPiece(int sq,int piece);
void removePiece(int sq);

bool makeMove(int move);
int takeMove();
void makeNullMove();
int takeNullMove();