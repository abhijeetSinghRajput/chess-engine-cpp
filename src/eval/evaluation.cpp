// evaluation.cpp
#include "eval/evaluation.hpp"
#include "core/defs.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/utils.hpp"
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
const int rookSupportSameRank = 5;
const int RookOpenFile = 20;
const int RookSemiOpenFile = 10;
const int QueenOpenFile = 10;
const int QueenSemiOpenFile = 5;
const int DoublePawnPenality = -5;
const int PawnSupport = 5;
const int brokenPawnShieldPenalty = -20;
const int exposedKingPenalty = -30;
const int kingZoneAttackPenalty = -10;
const int pawnStormPenalty[2][8] = {
    {0, 65, 35, 20, 10, 5, 0, 0}, // [0] = white
    {0, 0, 5, 10, 20, 35, 65, 0}, // [1] = black
};
const int EndGame_Material = (1 * pieceValue[PIECE_WR]) + (2 * pieceValue[PIECE_WN]) + (2 * pieceValue[PIECE_WP]) + (pieceValue[PIECE_WK]);

// MobilityBonus[phase][piece][mobility];
const int MobilityBonus[2][6][32] = {
    // midgame
    {
        /* Empty  */  {0},                                                                                                                           
        /* Pawns  */  {0},                                                                                                                           
        /* Rooks  */  {-20, -14, -8, -2, 4, 10, 14, 19, 23, 26, 27, 28, 29, 30, 31, 32},                                                            
        /* Knights*/  {-38, -25, -12, 0, 12, 25, 31, 38, 38},                                                                                       
        /* Bishops*/  {-25, -11, 3, 17, 31, 45, 57, 65, 71, 74, 76, 78, 79, 80, 81, 81},                                                            
        /* Queens */  {-10, -8, -6, -3, -1, 1, 3, 5, 8, 10, 12, 15, 16, 17, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20} 
    },
    // endgame
    {
        /* Empty   */ {0},
        /* Pawns   */ {0},
        /* Rooks   */ {-36, -19, -3, 13, 29, 46, 62, 79, 95, 106, 111, 114, 116, 117, 118, 118},
        /* Knights */ {-33, -23, -13, -3, 7, 17, 22, 27, 27},
        /* Bishops */ {-30, -16, -2, 12, 26, 40, 52, 60, 65, 69, 71, 73, 74, 75, 76, 76},
        /* Queens  */ {-18, -13, -7, -2, 3, 8, 13, 19, 23, 27, 32, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35} 
    }
};

int kingSafety(int kingSq, int color)
{
    if (kingSq >= 64) return 0;

    int score = 0, sq;
    U64 pawnBitboard = (color == WHITE) ? bitboard->pieces[PIECE_WP] : bitboard->pieces[PIECE_BP];
    U64 enemyPawnBitboard = (color == WHITE) ? bitboard->pieces[PIECE_BP] : bitboard->pieces[PIECE_WP];

    U64 pawnShieldMask = 0ULL, missingPawn, pawnStorm;

    switch (kingSq)
    {
        case SQ_B1:
            pawnShieldMask = (7ULL << 8);
            pawnStorm = enemyPawnBitboard & 0x70707070700ULL;
            break;
        case SQ_C1:
            pawnShieldMask = (7ULL << 9);
            pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e00ULL;
            break;
        case SQ_G1:
            pawnShieldMask = (7ULL << 13);
            pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e000ULL;
            break;
        case SQ_B8:
            pawnShieldMask = (7ULL << 48);
            pawnStorm = enemyPawnBitboard & 0x7070707070000ULL;
            break;
        case SQ_C8:
            pawnShieldMask = (7ULL << 49);
            pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e0000ULL;
            break;
        case SQ_G8:
            pawnShieldMask = (7ULL << 53);
            pawnStorm = enemyPawnBitboard & 0xe0e0e0e0e00000ULL;
            break;

        default:
            break;
    }

    // non zero pawnShildMask indicating king is on reasonalbe sq
    if (pawnShieldMask)
    {
        missingPawn = (~pawnBitboard) & pawnShieldMask;

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

    U64 knightBitboard = (color == WHITE) ? bitboard->pieces[PIECE_BN] : bitboard->pieces[PIECE_WN];
    U64 bishopBitboard = (color == WHITE) ? bitboard->pieces[PIECE_BB] : bitboard->pieces[PIECE_WB];
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
        U64 bishopAttack = getBishopAttacks(sq);
        if (kingAttack & bishopAttack)
            score += kingZoneAttackPenalty;
        bishopBitboard &= bishopBitboard - 1;
    }

    return score;
}

int evalPosition()
{

    int score = board->material[WHITE] - board->material[BLACK];
    int mobility = 0, sq;
    U64 allWhitePieces = bitboard->getPieces(WHITE);
    U64 allBlackPieces = bitboard->getPieces(BLACK);

    bool isEndgame = (board->material[BLACK] <= EndGame_Material && board->material[WHITE] <= EndGame_Material) ||
                     (board->material[BLACK] + board->material[WHITE] <= EndGame_Material * 2) ||
                     (board->pieceCount[PIECE_WP] <= 4 && board->pieceCount[PIECE_BP] <= 4);

    // printf("material: %d\n", score);
    // =================================================================
    // ============================= PAWN ==============================
    // =================================================================
    U64 pieceBitboard = bitboard->pieces[PIECE_WP];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += PawnTable[sq];

        // supporter pawn
        if (bitboard->pawnAttacks[WHITE][sq] & bitboard->pieces[PIECE_WP])
        {
            score += PawnSupport;
        }
        // isolated pawn penality
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[PIECE_WP]) == 0)
        {
            score += PawnIsolated;
        }
        // passed pawn
        if ((bitboard->passedPawnMask[WHITE][sq] & bitboard->pieces[PIECE_BP]) == 0)
        {
            score += PawnPassed[sq / 8];
        }
    }
    pieceBitboard = bitboard->pieces[PIECE_BP];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= PawnTable[MIRROR_64[sq]];

        // supporter pawn
        if (bitboard->pawnAttacks[BLACK][sq] & bitboard->pieces[PIECE_BP])
        {
            score -= PawnSupport;
        }
        // isolated pawn penality
        if ((bitboard->isolatedPawnMask[sq] & bitboard->pieces[PIECE_BP]) == 0)
        {
            score -= PawnIsolated;
        }
        // passed pawn
        if ((bitboard->passedPawnMask[BLACK][sq] & bitboard->pieces[PIECE_WP]) == 0)
        {
            score -= PawnPassed[7 - sq / 8];
        }
    }

    U64 whitePawns = bitboard->pieces[PIECE_WP];
    U64 blackPawns = bitboard->pieces[PIECE_BP];

    for (int file = 0; file < 8; file++)
    {
        int whiteCount = __builtin_popcountll(whitePawns & bitboard->fileMasks[file]);
        int blackCount = __builtin_popcountll(blackPawns & bitboard->fileMasks[file]);

        if (whiteCount > 1)
            score += whiteCount * DoublePawnPenality;

        if (blackCount > 1)
            score -= blackCount * DoublePawnPenality;
    }

    // =================================================================
    // ============================ KNIGHT =============================
    // =================================================================

    pieceBitboard = bitboard->pieces[PIECE_WN];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += KnightTable[sq];

        // mobility bonus
        mobility = __builtin_popcountll(bitboard->knightAttacks[sq] & ~allWhitePieces);
        score += MobilityBonus[isEndgame][MOB_KNIGHT][mobility];
    }

    pieceBitboard = bitboard->pieces[PIECE_BN];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= KnightTable[MIRROR_64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(bitboard->knightAttacks[sq] & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][MOB_KNIGHT][mobility];
    }
    // printf("Knight: %d\n", score);
    // =================================================================
    // ============================ BISHOP =============================
    // =================================================================
    pieceBitboard = bitboard->pieces[PIECE_WB];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score += BishopTable[sq];

        // mobility bonus
        mobility = __builtin_popcountll(getBishopAttacks(sq) & ~allWhitePieces);
        score += MobilityBonus[isEndgame][MOB_BISHOP][mobility];
    }

    pieceBitboard = bitboard->pieces[PIECE_BB];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= BishopTable[MIRROR_64[sq]];

        // mobility bonus
        mobility = __builtin_popcountll(getBishopAttacks(sq) & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][MOB_BISHOP][mobility];
    }
    // printf("Bishop: %d\n", score);

    // =================================================================
    // ============================= ROOK ==============================
    // =================================================================

    U64 rookAttack = 0ULL;
    U64 allWhiteRooks = bitboard->pieces[PIECE_WR];
    pieceBitboard = allWhiteRooks;
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        rookAttack = getRookAttacks(sq);
        score += RookTable[sq];

        // Build a mask of all OTHER white rooks (exclude current sq)
        U64 otherRooks = allWhiteRooks & ~(1ULL << sq);

        // Brothers
        if (rookAttack & bitboard->fileMasks[sq % 8] & otherRooks)
        {
            score += rookSupportSameFile;
        }
        if (rookAttack & bitboard->rankMasks[sq / 8] & otherRooks)
        {
            score += rookSupportSameRank;
        }

        // mobility bonus
        mobility = __builtin_popcountll(rookAttack & ~allWhitePieces);
        score += MobilityBonus[isEndgame][MOB_ROOK][mobility];

        // Taretary penetration
        if (rankOf(sq) == RANK_7) {
            // bonus if white rook attacks black pawns on 7th rank
            U64 attackedPawns = rookAttack & bitboard->pieces[PIECE_BP] & bitboard->rankMasks[RANK_7];
            if (attackedPawns) {
                score += 20 * __builtin_popcountll(attackedPawns);  // Per pawn attacked
            }
            // bonus if white rook is on same file/rank as black king
            int blackKingSq = __builtin_ctzll(bitboard->pieces[PIECE_BK]);
            int rookFile = fileOf(sq);
            int kingFile = fileOf(blackKingSq);
            if (abs(rookFile - kingFile) <= 1) {
                score += 40;  // Rook near king = dangerous!
            }
        }

        if (((bitboard->pieces[PIECE_WP] | bitboard->pieces[PIECE_BP]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += RookOpenFile;
        }
        else if ((bitboard->pieces[PIECE_WP] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += RookSemiOpenFile;
        }
    }

    U64 allBlackRooks = bitboard->pieces[PIECE_BR];
    pieceBitboard = allBlackRooks;

    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        score -= RookTable[MIRROR_64[sq]];
        rookAttack = getRookAttacks(sq);

        U64 otherRooks = allBlackRooks & ~(1ULL << sq);

        // Brothers
        if (rookAttack & bitboard->fileMasks[sq % 8] & otherRooks)
        {
            score -= rookSupportSameFile;
        }
        if (rookAttack & bitboard->rankMasks[sq / 8] & otherRooks)
        {
            score -= rookSupportSameRank;
        }

        // Mobility bonus
        mobility = __builtin_popcountll(rookAttack & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][MOB_ROOK][mobility];

        // Taretary penetration
        if (rankOf(sq) == RANK_2) {
            // bonus if black rook attacks white pawns on 2nd rank
            U64 attackedPawns = rookAttack & bitboard->pieces[PIECE_WP] & bitboard->rankMasks[RANK_2];
            if (attackedPawns) {
                score -= 20 * __builtin_popcountll(attackedPawns);  // Per pawn attacked
            }
            // bonus if black rook is on same file/rank as white king
            int whiteKingSq = __builtin_ctzll(bitboard->pieces[PIECE_WK]);
            int rookFile = fileOf(sq);
            int kingFile = fileOf(whiteKingSq);
            if (abs(rookFile - kingFile) <= 1) {
                score -= 40;  // Rook near king = dangerous!
            }
        }

        // Open file bonus
        if (((bitboard->pieces[PIECE_WP] | bitboard->pieces[PIECE_BP]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookOpenFile;
        }
        else if ((bitboard->pieces[PIECE_BP] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= RookSemiOpenFile;
        }
    }

    // printf("Rook: %d\n", score);
    // =================================================================
    // ============================= QUEEN =============================
    // =================================================================

    pieceBitboard = bitboard->pieces[PIECE_WQ];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        // an orthogonal support to rook
        if (getRookAttacks(sq) & bitboard->pieces[PIECE_WR])
        {
            score += rookSupportSameFile;
        }

        // mobility bonus
        mobility = __builtin_popcountll((getRookAttacks(sq) | getBishopAttacks(sq)) & ~allWhitePieces);
        score += MobilityBonus[isEndgame][MOB_QUEEN][mobility];

        // open file bonus
        if (((bitboard->pieces[PIECE_WP] | bitboard->pieces[PIECE_BP]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += QueenOpenFile;
        }
        else if ((bitboard->pieces[PIECE_WP] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score += QueenSemiOpenFile;
        }
    }

    pieceBitboard = bitboard->pieces[PIECE_BQ];
    while (pieceBitboard)
    {
        sq = __builtin_ctzll(pieceBitboard);
        pieceBitboard &= pieceBitboard - 1;
        // an orthogonal support from queen
        if (getRookAttacks(sq) & bitboard->pieces[PIECE_BR])
        {
            score -= rookSupportSameFile;
        }

        // mobility bonus
        mobility = __builtin_popcountll((getRookAttacks(sq) | getBishopAttacks(sq)) & ~allBlackPieces);
        score -= MobilityBonus[isEndgame][MOB_QUEEN][mobility];

        // open file bonus
        if (((bitboard->pieces[PIECE_WP] | bitboard->pieces[PIECE_BP]) & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= QueenOpenFile;
        }
        else if ((bitboard->pieces[PIECE_BP] & bitboard->fileMasks[sq % 8]) == 0)
        {
            score -= QueenSemiOpenFile;
        }
    }
    // printf("Queen: %d\n", score);
    // =================================================================
    // ======================= WHITE KING ==============================
    // =================================================================

    int kingSq = (bitboard->pieces[PIECE_WK]) ? __builtin_ctzll(bitboard->pieces[PIECE_WK]) : 64;

    if (kingSq < 64 && board->material[BLACK] <= EndGame_Material)
    {
        score += KingE[kingSq];
    }
    else if (kingSq < 64)
    {
        score += KingO[kingSq];
    }
    score += kingSafety(kingSq, WHITE);

    // =================================================================
    // ======================= BLACK KING ==============================
    // =================================================================

    kingSq = (bitboard->pieces[PIECE_BK]) ? __builtin_ctzll(bitboard->pieces[PIECE_BK]) : 64;

    if (kingSq < 64 && board->material[WHITE] <= EndGame_Material)
    {
        score -= KingE[MIRROR_64[kingSq]];
    }
    else if (kingSq < 64)
    {
        score -= KingO[MIRROR_64[kingSq]];
    }
    score -= kingSafety(kingSq, BLACK);

    // printf("king: %d\n", score);
    // =================================================================
    // ============================ ADD ON =============================
    // =================================================================

    if (board->pieceCount[PIECE_WB] >= 2)
    {
        score += BishopPair;
    }

    if (board->pieceCount[PIECE_BB] >= 2)
    {
        score -= BishopPair;
    }
    // printf("bonus: %d\n", score);
    return (board->side == WHITE) ? score : -score;
}
