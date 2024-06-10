#pragma once
#include "defs.hpp"

void hashPiece(int sq, int piece);
void hashEnPassant();
void hashCastle();
void hashSide();

bool isUnderAttack(int sq, int attackingSide);
U64 getRookAttacks(int sq);
U64 getBishopAttacks(int sq);
std::string &moveStr(int move);
