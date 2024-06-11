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
extern int enPassantFlag;
extern int castleFlag;
extern int pawnStartFlag;

extern int captureFlag;
extern int promotionFlag;

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