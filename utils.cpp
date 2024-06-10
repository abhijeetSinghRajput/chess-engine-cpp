#include "bitboard.hpp"
#include "zobristKeys.hpp"

void hashPiece(int sq, int piece){
    board->positionKey ^= pieceKeys[piece][sq];
}
void hashEnPassant(){
    board->positionKey ^= pieceKeys[empty][board->enPassantSq];
}
void hashCastle(){
    board->positionKey ^= castleKeys[board->castlePermission];
}
void hashSide(){
    board->positionKey ^= sideKey;
}

U64 getRookAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->rookAttacks[sq];

    int index = (blocker * rookMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->rookLookupTable[sq][index];
    return attacks;
}

U64 getBishopAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->bishopAttacks[sq];

    int index = (blocker * bishopMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->bishopLookupTable[sq][index];
    return attacks;
}

bool isUnderAttack(int sq, int attackingSide)
{
    if (attackingSide == black)
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[bp] & bitboard->pawnAttacks[white][sq]) return true;
        if (bitboard->pieces[bn] & bitboard->knightAttacks[sq]) return true;
        if (bitboard->pieces[bk] & bitboard->kingAttacks[sq]) return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[br] & rookAttacks) return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[bb] & bishopAttacks) return true;

        // Queen attacks
        if (bitboard->pieces[bq] & (rookAttacks | bishopAttacks)) return true;
    }
    else
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[wp] & bitboard->pawnAttacks[black][sq]) return true;
        if (bitboard->pieces[wn] & bitboard->knightAttacks[sq]) return true;
        if (bitboard->pieces[wk] & bitboard->kingAttacks[sq]) return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[wr] & rookAttacks) return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[wb] & bishopAttacks) return true;

        // Queen attacks
        if (bitboard->pieces[wq] & (rookAttacks | bishopAttacks)) return true;
    }
    return false;
}


