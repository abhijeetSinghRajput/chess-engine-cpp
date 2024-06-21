#include "evaluation.hpp"
#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "utils.hpp"
#include <iostream>

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
const int rookSupportSameFile = 10;
const int RookOpenFile = 20;
const int RookSemiOpenFile = 10;
const int QueenOpenFile = 10;
const int QueenSemiOpenFile = 5;
const int DoublePawnPenality = -10;
const int PawnSupport = 5;
const int brokenPawnShieldPenalty = -30;
const int exposedKingPenalty = -30;
const int kingZoneAttackPenalty = -10;
const int pawnStormPenalty[2][8] = {
    {0, 65, 35, 20, 10, 5, 0, 0},
    {0, 0, 5, 10, 20, 35, 65, 0},
};
const int EndGame_Material = (1 * pieceValue[wr]) + (2 * pieceValue[wn]) + (2 * pieceValue[wp]) + (pieceValue[wk]);

// MobilityBonus[phase][piece][mobility];
const int MobilityBonus[2][6][32] = {
    // midgame
    {
        {},                                                                                                                           // Empty
        {},                                                                                                                           // Pawns
        {-20, -14, -8, -2, 4, 10, 14, 19, 23, 26, 27, 28, 29, 30, 31, 32},                                                            // Rooks
        {-38, -25, -12, 0, 12, 25, 31, 38, 38},                                                                                       // Knights
        {-25, -11, 3, 17, 31, 45, 57, 65, 71, 74, 76, 78, 79, 80, 81, 81},                                                            // Bishops
        {-10, -8, -6, -3, -1, 1, 3, 5, 8, 10, 12, 15, 16, 17, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20} // Queens
    },
    // endgame
    {
        {},                                                                                                                              // Empty
        {},                                                                                                                              // Pawns
        {-36, -19, -3, 13, 29, 46, 62, 79, 95, 106, 111, 114, 116, 117, 118, 118},                                                       // Rooks
        {-33, -23, -13, -3, 7, 17, 22, 27, 27},                                                                                          // Knights
        {-30, -16, -2, 12, 26, 40, 52, 60, 65, 69, 71, 73, 74, 75, 76, 76},                                                              // Bishops
        {-18, -13, -7, -2, 3, 8, 13, 19, 23, 27, 32, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35} // Queens
    }};

int kingSafety(int kingSq, int color)
{
    int score = 0, sq;
    U64 pawnBitboard = (color == white) ? bitboard->pieces[wp] : bitboard->pieces[bp];
    U64 enemyPawnBitboard = (color == white) ? bitboard->pieces[bp] : bitboard->pieces[wp];

    U64 pawnShieldMask = 0ULL, pawnShield, pawnStorm;
    int missingPawn;

    switch (kingSq)
    {
    case b1:
        pawnShieldMask = (7ULL << 8);
        pawnStorm = enemyPawnBitboard & 0x70707070700ULL;
        break;
    case c1:
        pawnShieldMask = (7ULL << 9);
        pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e00ULL;
        break;
    case g1:
        pawnShieldMask = (7ULL << 13);
        pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e000ULL;
        break;
    case b8:
        pawnShieldMask = (7ULL << 48);
        pawnStorm = enemyPawnBitboard & 0x7070707070000ULL;
        break;
    case c8:
        pawnShieldMask = (7ULL << 49);
        pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e0000ULL;
        break;
    case g8:
        pawnShieldMask = (7ULL << 53);
        pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e00000ULL;
        break;

    default:
        break;
    }

    // non zero pawnShildMask indicating king is on reasonalbe sq
    if (pawnShieldMask)
    {
        pawnShield = pawnBitboard & pawnShieldMask;
        missingPawn = ~pawnShield & pawnShieldMask;

        // ============== pawn shield evaluation
        score += __builtin_popcountll(missingPawn) * brokenPawnShieldPenalty;
        while (missingPawn)
        {
            sq = __builtin_ctzll(missingPawn);
            if ((pawnBitboard & bitboard->fileMasks[sq % 8]) == 0)
            {
                score += exposedKingPenalty;
            }
            missingPawn &= missingPawn - 1;
        }

        // ============== pawn storm evaluation
        while (pawnStorm)
        {
            sq = __builtin_ctzll(pawnStorm);
            score += pawnStormPenalty[color][sq / 8];
            pawnStorm &= pawnStorm - 1;
        }
    }

    // minor piece attack on a king zone

    U64 knightBitboard = (color == white) ? bitboard->pieces[bn] : bitboard->pieces[wn];
    U64 bishopBitboard = (color == white) ? bitboard->pieces[bb] : bitboard->pieces[wb];
    U64 kingAttack = bitboard->kingAttacks[kingSq];

    while (knightBitboard)
    {
        sq = __builtin_ctzll(knightBitboard);
        U64 knightAttack = bitboard->knightAttacks[sq];
        score += kingZoneAttackPenalty * __builtin_popcountll(kingAttack & knightAttack);
        knightBitboard &= knightBitboard - 1;
    }

    while (bishopBitboard)
    {
        sq = __builtin_ctzll(bishopBitboard);
        U64 bishopAttack = bitboard->bishopAttacks[sq];
        if (kingAttack & bishopAttack)
            score += kingZoneAttackPenalty;
        bishopBitboard &= bishopBitboard - 1;
    }

    return score;
}

int evalPosition()
{

    int score = board->material[white] - board->material[black];
    int doublePawnCount = 0;
    int mobility = 0;
    U64 allWhitePieces = bitboard->getPieces(white);
    U64 allBlackPieces = bitboard->getPieces(black);

    bool isEndgame = (board->material[black] <= EndGame_Material && board->material[white] <= EndGame_Material) ||
                     (board->material[black] + board->material[white] <= EndGame_Material * 2) ||
                     (board->pieceCount[wp] <= 4 && board->pieceCount[bp] <= 4);

    // =================================================================
    // ============================= PAWN ==============================
    // =================================================================

    U64 pieceBitboard = bitboard->pieces[wp];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += PawnTable[sq];
        // double pawn penality
        doublePawnCount = __builtin_popcountll(bitboard->pieces[wp] & bitboard->fileMasks[sq % 8]);
        if (doublePawnCount > 1)
        {
            score += DoublePawnPenality;
        }
        // supporter pawn
        if (bitboard->pawnAttacks[white][sq] & bitboard->pieces[wp])
        {
            score += PawnSupport;
        }
        // isolated pawn penality
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[wp]) == 0)
        {
            score += PawnIsolated;
        }
        // passed pawn
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

        // double pawn penality
        doublePawnCount = __builtin_popcountll(bitboard->pieces[bp] & bitboard->fileMasks[sq % 8]);
        if (doublePawnCount > 1)
        {
            score -= DoublePawnPenality;
        }
        // supporter pawn
        if (bitboard->pawnAttacks[black][sq] & bitboard->pieces[bp])
        {
            score -= PawnSupport;
        }
        // isolated pawn penality
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[bp]) == 0)
        {
            score -= PawnIsolated;
        }
        // passed pawn
        if ((bitboard->passedPawnMask[black][sq] & bitboard->pieces[wp]) == 0)
        {
            score -= PawnPassed[7 - sq / 8];
        }
    }

    // =================================================================
    // ============================ KNIGHT =============================
    // =================================================================

    pieceBitboard = bitboard->pieces[wn];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += KnightTable[sq];

        // mobility bonus
        mobility = __builtin_popcountll(bitboard->knightAttacks[wn] & ~allWhitePieces);
        score += MobilityBonus[isEndgame][wn][mobility];
    }

    pieceBitboard = bitboard->pieces[bn];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= KnightTable[Mirror64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(bitboard->knightAttacks[wn] & ~allBlackPieces);
        score += MobilityBonus[isEndgame][wn][mobility];
    }

    // =================================================================
    // ============================ BISHOP =============================
    // =================================================================
    pieceBitboard = bitboard->pieces[wb];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += BishopTable[sq];

        // mobility bonus
        mobility = __builtin_popcountll(getBishopAttacks(sq) & ~allWhitePieces);
        score += MobilityBonus[isEndgame][wb][mobility];
    }

    pieceBitboard = bitboard->pieces[bb];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= BishopTable[Mirror64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(getBishopAttacks(sq) & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][wb][mobility];
    }

    // =================================================================
    // ============================= ROOK ==============================
    // =================================================================

    pieceBitboard = bitboard->pieces[wr];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        if (getRookAttacks(sq) & bitboard->fileMasks[sq % 8] & pieceBitboard)
        {
            score += rookSupportSameFile;
        }
        score += RookTable[Mirror64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(getRookAttacks(sq) & ~allWhitePieces);
        score += MobilityBonus[isEndgame][wr][mobility];

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
        if (getRookAttacks(sq) & bitboard->fileMasks[sq % 8] & pieceBitboard)
        {
            score -= rookSupportSameFile;
        }
        score -= RookTable[Mirror64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(getRookAttacks(sq) & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][wr][mobility];

        if (((bitboard->pieces[wp] | bitboard->pieces[bp]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookOpenFile;
        }
        else if ((bitboard->pieces[bp] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookSemiOpenFile;
        }
    }

    // =================================================================
    // ============================= QUEEN =============================
    // =================================================================

    pieceBitboard = bitboard->pieces[wq];
    while (pieceBitboard)
    {
        int sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;

        // mobility bonus
        mobility = __builtin_popcountll((getRookAttacks(sq) | getBishopAttacks(sq)) & ~allWhitePieces);
        score += MobilityBonus[isEndgame][wq][mobility];

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

        // mobility bonus
        mobility = __builtin_popcountll((getRookAttacks(sq) | getBishopAttacks(sq)) & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][wq][mobility];

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

    // =================================================================
    // ======================= WHITE KING ==============================
    // =================================================================

    int kingSq = (bitboard->pieces[wk]) ? __builtin_ctzll(bitboard->pieces[wk]) : 64;

    if (kingSq < 64 && board->material[black] <= EndGame_Material)
    {
        score += KingE[kingSq];
    }
    else if (kingSq < 64)
    {
        score += KingO[kingSq];
    }
    score += kingSafety(kingSq, white);

    // =================================================================
    // ======================= BLACK KING ==============================
    // =================================================================

    kingSq = (bitboard->pieces[wk]) ? __builtin_ctzll(bitboard->pieces[bk]) : 64;

    if (kingSq < 64 && board->material[white] <= EndGame_Material)
    {
        score -= KingE[Mirror64[kingSq]];
    }
    else if (kingSq < 64)
    {
        score -= KingO[Mirror64[kingSq]];
    }
    score -= kingSafety(kingSq, black);

    // =================================================================
    // ============================ ADD ON =============================
    // =================================================================

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
