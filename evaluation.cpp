#include "evaluation.hpp"
#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"

const int PawnTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    10, 10, 0, -10, -10, 0, 10, 10,
    5, 0, 0, 5, 5, 0, 0, 5,
    0, 0, 10, 20, 20, 10, 0, 0,
    5, 5, 5, 10, 10, 5, 5, 5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0, 0, 0, 0, 0, 0, 0, 0};

const int KnightTable[] = {
    0, -10, 0, 0, 0, 0, -10, 0,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 10, 10, 10, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 5, 0,
    5, 10, 15, 20, 20, 15, 10, 5,
    5, 10, 10, 20, 20, 10, 10, 5,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

const int BishopTable[] = {
    0, 0, -10, 0, 0, -10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};
    
const int RookTable[] = {
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0, 0, 5, 10, 10, 5, 0, 0};
    
const int KingO[] = {
    0, 5, 5, -10, -10, 0, 10, 5,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70};
    
const int KingE[] = {
    -50, -10, 0, 0, 0, 0, -10, -50,
    -10, 0, 10, 10, 10, 10, 0, -10,
    0, 10, 20, 20, 20, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 20, 20, 20, 10, 0,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -50, -10, 0, 0, 0, 0, -10, -50};
    

const int BishopPair = 40;
const int PawnIsolated = -10;
const int PawnPassed[] = {0, 5, 10, 20, 35, 60, 100, 200};
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int EndGame_Material = (1 * pieceValue[wr]) + (2 * pieceValue[wn]) + (2 * pieceValue[wp]) + (pieceValue[wk]);

int evalPosition()
{

    int score = board->material[white] - board->material[black];
    // pawn
    U64 pieceBitboard = bitboard->pieces[wp];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += PawnTable[sq];
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[wp]) == 0)
        {
            score += PawnIsolated;
        }
        if ((bitboard->passedPawnMask[white][sq] & bitboard->pieces[bp]) == 0)
        {
            score += PawnPassed[sq / 8];
        }
    }
    pieceBitboard = bitboard->pieces[bp];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= PawnTable[Mirror64[sq]];
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[bp]) == 0)
        {
            score -= PawnIsolated;
        }
        if ((bitboard->passedPawnMask[black][sq] & bitboard->pieces[wp]) == 0)
        {
            score -= PawnPassed[7 - sq / 8];
        }
    }

    // KNIGHT
    pieceBitboard = bitboard->pieces[wn];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += KnightTable[sq];
    }
    pieceBitboard = bitboard->pieces[bn];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= KnightTable[Mirror64[sq]];
    }


    // BISHOP
    pieceBitboard = bitboard->pieces[wb];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += BishopTable[sq];
    }
    pieceBitboard = bitboard->pieces[bb];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= BishopTable[Mirror64[sq]];
    }



    // ROOK
    pieceBitboard = bitboard->pieces[wr];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += RookTable[Mirror64[sq]];
        if (((bitboard->pieces[wp] | bitboard->pieces[bp]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += RookOpenFile;
        }
        else if ((bitboard->pieces[wp] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += RookSemiOpenFile;
        }
    }
    pieceBitboard = bitboard->pieces[br];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= RookTable[Mirror64[sq]];
        if (((bitboard->pieces[wp] | bitboard->pieces[bp]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookOpenFile;
        }
        else if ((bitboard->pieces[bp] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookSemiOpenFile;
        }
    }



    // QUEEN
    pieceBitboard = bitboard->pieces[wq];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        // open file bonus
        if (((bitboard->pieces[wp] | bitboard->pieces[bp]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += QueenOpenFile;
        }
        else if ((bitboard->pieces[wp] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += QueenSemiOpenFile;
        }
    }
    pieceBitboard = bitboard->pieces[bq];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        // open file bonus
        if (((bitboard->pieces[wp] | bitboard->pieces[bp]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= QueenOpenFile;
        }
        else if ((bitboard->pieces[bp] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= QueenSemiOpenFile;
        }
    }


    // white king
    int kingSq = __builtin_ctzll(bitboard->pieces[wk]);
    if (board->material[black] <= EndGame_Material)
    {
        score += KingE[kingSq];
    }
    else
    {
        score += KingO[kingSq];
    }

    // black king
    kingSq = __builtin_ctzll(bitboard->pieces[bk]);
    if (board->material[white] <= EndGame_Material)
    {
        score -= KingE[Mirror64[kingSq]];
    }
    else
    {
        score -= KingO[Mirror64[kingSq]];
    }



    // bonus
    if (board->pieceCount[wb] >= 2)
    {
        score += BishopPair;
    }
    if (board->pieceCount[bb] >= 2)
    {
        score -= BishopPair;
    }

    return (board->side == white) ? score : -score;
}