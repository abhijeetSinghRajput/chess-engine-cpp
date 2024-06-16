#pragma once
#include "defs.hpp"

void hashPiece(int sq, int piece);
void hashEnPassant();
void hashCastle();
void hashSide();

bool isUnderAttack(int sq, int attackingSide);
U64 getRookAttacks(int sq);
U64 getBishopAttacks(int sq);
int materialDraw();
void newGame();
int parseMove(std::string &move_str);
