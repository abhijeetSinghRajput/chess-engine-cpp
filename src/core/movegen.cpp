#include "core/defs.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/utils.hpp"
#include "core/move.hpp"
#include "search/search.hpp"
#include "core/movegen.hpp"
#include <vector>

MoveList moveList;

// MvvLva = [victim][attacker]
int MvvLva[13][13] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 105, 102, 104, 103, 101, 100, 105, 102, 104, 103, 101, 100},
    {0, 405, 402, 404, 403, 401, 400, 405, 402, 404, 403, 401, 400},
    {0, 205, 202, 204, 203, 201, 200, 205, 202, 204, 203, 201, 200},
    {0, 305, 302, 304, 303, 301, 300, 305, 302, 304, 303, 301, 300},
    {0, 505, 502, 504, 503, 501, 500, 505, 502, 504, 503, 501, 500},
    {0, 605, 602, 604, 603, 601, 600, 605, 602, 604, 603, 601, 600},
    {0, 105, 102, 104, 103, 101, 100, 105, 102, 104, 103, 101, 100},
    {0, 405, 402, 404, 403, 401, 400, 405, 402, 404, 403, 401, 400},
    {0, 205, 202, 204, 203, 201, 200, 205, 202, 204, 203, 201, 200},
    {0, 305, 302, 304, 303, 301, 300, 305, 302, 304, 303, 301, 300},
    {0, 505, 502, 504, 503, 501, 500, 505, 502, 504, 503, 501, 500},
    {0, 605, 602, 604, 603, 601, 600, 605, 602, 604, 603, 601, 600},
};

inline void addCaptureMove(int move)
{
    int victim = moveCapturePiece(move);
    int attacker = board->pieces[moveFrom(move)];
    int score = MvvLva[victim][attacker] + 1000000;
    moveList.moves[moveList.count++] = {move, score};
}

inline void addQuiteMove(int move)
{
    int score = 0;
    if (searchController->killers[searchController->ply][0] == move)
        score = 900000;
    else if (searchController->killers[searchController->ply][1] == move)
        score = 800000;
    else
    {
        int piece = board->pieces[moveFrom(move)];
        int toSq = moveTo(move);
        score = searchController->history[piece][toSq];
        if (score == 0)
            score = 1 + (moveFrom(move) % 1000);
    }
    moveList.moves[moveList.count++] = {move, score};
}

inline void addEnPassantMove(int move)
{
    int score = MvvLva[PIECE_WP][PIECE_BP] + 1000000;
    moveList.moves[moveList.count++] = {move, score};
}

// Kept for header/ABI compatibility - no longer called from the hot pawn-gen
// path below (pawns are now generated in bulk via bitboard shifts), but left
// intact in case anything else links against them.
void addWhitePawnQuietMove(int from, int to)
{
    if (rankOf(to) == RANK_8)
    {
        addQuiteMove(buildMove(from, to, 0, PIECE_WQ, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_WR, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_WB, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_WN, 0));
    }
    else
    {
        addQuiteMove(buildMove(from, to, 0, 0, 0));
    }
}
void addBlackPawnQuietMove(int from, int to)
{
    if (rankOf(to) == RANK_1)
    {
        addQuiteMove(buildMove(from, to, 0, PIECE_BQ, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_BR, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_BB, 0));
        addQuiteMove(buildMove(from, to, 0, PIECE_BN, 0));
    }
    else
    {
        addQuiteMove(buildMove(from, to, 0, 0, 0));
    }
}

void addWhiteCaptureMove(int from, int to, int capture)
{
    if (rankOf(to) == RANK_8)
    {
        addCaptureMove(buildMove(from, to, capture, PIECE_WQ, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_WR, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_WB, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_WN, 0));
    }
    else
    {
        addCaptureMove(buildMove(from, to, capture, 0, 0));
    }
}
void addBlackCaptureMove(int from, int to, int capture)
{
    if (rankOf(to) == RANK_1)
    {
        addCaptureMove(buildMove(from, to, capture, PIECE_BQ, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_BR, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_BB, 0));
        addCaptureMove(buildMove(from, to, capture, PIECE_BN, 0));
    }
    else
    {
        addCaptureMove(buildMove(from, to, capture, 0, 0));
    }
}

// ==========================================================
// ===================== pawn move gen (bitboard) ===========
// ==========================================================
// Instead of walking every pawn on the 120-mailbox and probing
// sq+9/sq+10/sq+11 one square at a time, generate all pushes and
// all captures for the whole pawn bitboard in one shot with shifts,
// split promo/non-promo with a mask (no per-move rank branch), and
// only touch the 64->120 conversion table once per resulting move.

void genWhitePawnMoves(U64 empty, U64 enemy, bool capturesOnly)
{
    U64 whitePawns = bitboard->pieces[PIECE_WP];

    if (!capturesOnly)
    {
        U64 singlePush = (whitePawns << 8) & empty;
        U64 doublePush = (((whitePawns & Bitboard::rankMasks[RANK_2]) << 8) & empty) << 8 & empty;

        U64 pushPromo = singlePush & Bitboard::rankMasks[RANK_8];
        U64 pushQuiet = singlePush & ~Bitboard::rankMasks[RANK_8];

        while (pushQuiet)
        {
            int to64 = __builtin_ctzll(pushQuiet);
            pushQuiet &= pushQuiet - 1;
            addQuiteMove(buildMove(sq64To120[to64 - 8], sq64To120[to64], 0, 0, 0));
        }
        while (doublePush)
        {
            int to64 = __builtin_ctzll(doublePush);
            doublePush &= doublePush - 1;
            addQuiteMove(buildMove(sq64To120[to64 - 16], sq64To120[to64], 0, 0, PAWN_START_FLAG));
        }
        while (pushPromo)
        {
            int to64 = __builtin_ctzll(pushPromo);
            pushPromo &= pushPromo - 1;
            int from = sq64To120[to64 - 8];
            int to = sq64To120[to64];
            addQuiteMove(buildMove(from, to, 0, PIECE_WQ, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_WR, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_WB, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_WN, 0));
        }
    }

    U64 captNE = ((whitePawns & ~Bitboard::fileMasks[FILE_H]) << 9) & enemy;
    U64 captNW = ((whitePawns & ~Bitboard::fileMasks[FILE_A]) << 7) & enemy;

    U64 captNEPromo = captNE & Bitboard::rankMasks[RANK_8];
    U64 captNEQuiet = captNE & ~Bitboard::rankMasks[RANK_8];
    U64 captNWPromo = captNW & Bitboard::rankMasks[RANK_8];
    U64 captNWQuiet = captNW & ~Bitboard::rankMasks[RANK_8];

    while (captNEQuiet)
    {
        int to64 = __builtin_ctzll(captNEQuiet);
        captNEQuiet &= captNEQuiet - 1;
        int to = sq64To120[to64];
        addCaptureMove(buildMove(sq64To120[to64 - 9], to, board->pieces[to], 0, 0));
    }
    while (captNEPromo)
    {
        int to64 = __builtin_ctzll(captNEPromo);
        captNEPromo &= captNEPromo - 1;
        int from = sq64To120[to64 - 9];
        int to = sq64To120[to64];
        int cap = board->pieces[to];
        addCaptureMove(buildMove(from, to, cap, PIECE_WQ, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WR, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WB, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WN, 0));
    }
    while (captNWQuiet)
    {
        int to64 = __builtin_ctzll(captNWQuiet);
        captNWQuiet &= captNWQuiet - 1;
        int to = sq64To120[to64];
        addCaptureMove(buildMove(sq64To120[to64 - 7], to, board->pieces[to], 0, 0));
    }
    while (captNWPromo)
    {
        int to64 = __builtin_ctzll(captNWPromo);
        captNWPromo &= captNWPromo - 1;
        int from = sq64To120[to64 - 7];
        int to = sq64To120[to64];
        int cap = board->pieces[to];
        addCaptureMove(buildMove(from, to, cap, PIECE_WQ, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WR, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WB, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_WN, 0));
    }

    // En passant: rare, so a cheap per-pawn scan is fine. Guarded on
    // enPassantSq being truthy (0 == no ep square in the 120 mailbox,
    // matching the offBoard/empty convention already used elsewhere) -
    // remove the guard if that assumption doesn't hold in your defs.hpp.
    if (board->enPassantSq)
    {
        U64 epPawns = whitePawns;
        while (epPawns)
        {
            int sq64 = __builtin_ctzll(epPawns);
            epPawns &= epPawns - 1;
            int sq = sq64To120[sq64];
            if (sq + 9 == board->enPassantSq)
                addEnPassantMove(buildMove(sq, sq + 9, 0, 0, EN_PASSANT_FLAG));
            if (sq + 11 == board->enPassantSq)
                addEnPassantMove(buildMove(sq, sq + 11, 0, 0, EN_PASSANT_FLAG));
        }
    }
}

void genBlackPawnMoves(U64 empty, U64 enemy, bool capturesOnly)
{
    U64 blackPawns = bitboard->pieces[PIECE_BP];

    if (!capturesOnly)
    {
        U64 singlePush = (blackPawns >> 8) & empty;
        U64 doublePush = (((blackPawns & Bitboard::rankMasks[RANK_7]) >> 8) & empty) >> 8 & empty;

        U64 pushPromo = singlePush & Bitboard::rankMasks[RANK_1];
        U64 pushQuiet = singlePush & ~Bitboard::rankMasks[RANK_1];

        while (pushQuiet)
        {
            int to64 = __builtin_ctzll(pushQuiet);
            pushQuiet &= pushQuiet - 1;
            addQuiteMove(buildMove(sq64To120[to64 + 8], sq64To120[to64], 0, 0, 0));
        }
        while (doublePush)
        {
            int to64 = __builtin_ctzll(doublePush);
            doublePush &= doublePush - 1;
            addQuiteMove(buildMove(sq64To120[to64 + 16], sq64To120[to64], 0, 0, PAWN_START_FLAG));
        }
        while (pushPromo)
        {
            int to64 = __builtin_ctzll(pushPromo);
            pushPromo &= pushPromo - 1;
            int from = sq64To120[to64 + 8];
            int to = sq64To120[to64];
            addQuiteMove(buildMove(from, to, 0, PIECE_BQ, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_BR, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_BB, 0));
            addQuiteMove(buildMove(from, to, 0, PIECE_BN, 0));
        }
    }

    U64 captSW = ((blackPawns & ~Bitboard::fileMasks[FILE_A]) >> 9) & enemy;
    U64 captSE = ((blackPawns & ~Bitboard::fileMasks[FILE_H]) >> 7) & enemy;

    U64 captSWPromo = captSW & Bitboard::rankMasks[RANK_1];
    U64 captSWQuiet = captSW & ~Bitboard::rankMasks[RANK_1];
    U64 captSEPromo = captSE & Bitboard::rankMasks[RANK_1];
    U64 captSEQuiet = captSE & ~Bitboard::rankMasks[RANK_1];

    while (captSWQuiet)
    {
        int to64 = __builtin_ctzll(captSWQuiet);
        captSWQuiet &= captSWQuiet - 1;
        int to = sq64To120[to64];
        addCaptureMove(buildMove(sq64To120[to64 + 9], to, board->pieces[to], 0, 0));
    }
    while (captSWPromo)
    {
        int to64 = __builtin_ctzll(captSWPromo);
        captSWPromo &= captSWPromo - 1;
        int from = sq64To120[to64 + 9];
        int to = sq64To120[to64];
        int cap = board->pieces[to];
        addCaptureMove(buildMove(from, to, cap, PIECE_BQ, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BR, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BB, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BN, 0));
    }
    while (captSEQuiet)
    {
        int to64 = __builtin_ctzll(captSEQuiet);
        captSEQuiet &= captSEQuiet - 1;
        int to = sq64To120[to64];
        addCaptureMove(buildMove(sq64To120[to64 + 7], to, board->pieces[to], 0, 0));
    }
    while (captSEPromo)
    {
        int to64 = __builtin_ctzll(captSEPromo);
        captSEPromo &= captSEPromo - 1;
        int from = sq64To120[to64 + 7];
        int to = sq64To120[to64];
        int cap = board->pieces[to];
        addCaptureMove(buildMove(from, to, cap, PIECE_BQ, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BR, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BB, 0));
        addCaptureMove(buildMove(from, to, cap, PIECE_BN, 0));
    }

    if (board->enPassantSq)
    {
        U64 epPawns = blackPawns;
        while (epPawns)
        {
            int sq64 = __builtin_ctzll(epPawns);
            epPawns &= epPawns - 1;
            int sq = sq64To120[sq64];
            if (sq - 9 == board->enPassantSq)
                addEnPassantMove(buildMove(sq, sq - 9, 0, 0, EN_PASSANT_FLAG));
            if (sq - 11 == board->enPassantSq)
                addEnPassantMove(buildMove(sq, sq - 11, 0, 0, EN_PASSANT_FLAG));
        }
    }
}

// ==========================================================
// ================= knight/king/sliding move gen ============
// ==========================================================
// Same magic-bitboard/attack-table approach as before, but the
// enemy-piece test now happens directly on the 64-index bit that
// ctzll already gave us, instead of round-tripping it through
// sq64To120[] and then back through sq120To64[] (which always
// just returns the same index - a wasted double table lookup on
// every single generated move).

void genNonSlidingMoves(bool capturesOnly)
{
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    for (int piece : nonSlidingPieces[board->side])
    {
        U64 pieceBitboard = bitboard->pieces[piece];

        while (pieceBitboard)
        {
            int sq = __builtin_ctzll(pieceBitboard);
            pieceBitboard &= pieceBitboard - 1;

            U64 attacksPattern = (PIECE_TYPE[piece] == 'n') ? bitboard->knightAttacks[sq] : bitboard->kingAttacks[sq];
            attacksPattern &= ~friendlyPiecesBitboard;

            while (attacksPattern)
            {
                int targetSq64 = __builtin_ctzll(attacksPattern);
                attacksPattern &= attacksPattern - 1;
                int targetSq = sq64To120[targetSq64];

                if (enemyPiecesBitboard & (1ULL << targetSq64))
                {
                    addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
                }
            }
        }
    }
}

void genSlidingMoves(bool capturesOnly)
{
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    for (int piece : slidingPieces[board->side])
    {
        U64 pieceBitboard = bitboard->pieces[piece];
        while (pieceBitboard)
        {
            int sq = __builtin_ctzll(pieceBitboard);
            pieceBitboard &= pieceBitboard - 1;
            U64 attacksPattern = 0ULL;

            switch (PIECE_TYPE[piece])
            {
            case 'r':
                attacksPattern = getRookAttacks(sq);
                break;
            case 'b':
                attacksPattern = getBishopAttacks(sq);
                break;
            case 'q':
                attacksPattern = getBishopAttacks(sq) | getRookAttacks(sq);
                break;
            default:
                break;
            }
            attacksPattern &= ~friendlyPiecesBitboard;

            while (attacksPattern)
            {
                int targetSq64 = __builtin_ctzll(attacksPattern);
                attacksPattern &= attacksPattern - 1;
                int targetSq = sq64To120[targetSq64];

                if (enemyPiecesBitboard & (1ULL << targetSq64))
                {
                    addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
                }
            }
        }
    }
}

MoveList generateMoves()
{
    moveList.count = 0;
    U64 friendly = bitboard->getPieces(board->side);
    U64 enemy = bitboard->getPieces(board->side ^ 1);
    U64 empty = ~(friendly | enemy);

    if (board->side == WHITE)
    {
        genWhitePawnMoves(empty, enemy, false);

        if (board->castlePermission & CASTLE_WK)
        {
            if (board->pieces[SQ_F1] == PIECE_EMPTY && board->pieces[SQ_G1] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(sq120To64[SQ_F1], BLACK))
                {
                    addQuiteMove(buildMove(SQ_E1, SQ_G1, 0, 0, CASTLE_FLAG));
                }
            }
        }
        if (board->castlePermission & CASTLE_WQ)
        {
            if (board->pieces[SQ_D1] == PIECE_EMPTY && board->pieces[SQ_C1] == PIECE_EMPTY && board->pieces[SQ_B1] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(sq120To64[SQ_D1], BLACK))
                {
                    addQuiteMove(buildMove(SQ_E1, SQ_C1, 0, 0, CASTLE_FLAG));
                }
            }
        }
    }
    else
    {
        genBlackPawnMoves(empty, enemy, false);

        if (board->castlePermission & CASTLE_BK)
        {
            if (board->pieces[SQ_F8] == PIECE_EMPTY && board->pieces[SQ_G8] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(sq120To64[SQ_F8], WHITE))
                {
                    addQuiteMove(buildMove(SQ_E8, SQ_G8, 0, 0, CASTLE_FLAG));
                }
            }
        }
        if (board->castlePermission & CASTLE_BQ)
        {
            if (board->pieces[SQ_D8] == PIECE_EMPTY && board->pieces[SQ_C8] == PIECE_EMPTY && board->pieces[SQ_B8] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(sq120To64[SQ_D8], WHITE))
                {
                    addQuiteMove(buildMove(SQ_E8, SQ_C8, 0, 0, CASTLE_FLAG));
                }
            }
        }
    }
    genSlidingMoves(false);
    genNonSlidingMoves(false);
    return moveList;
}

MoveList generateCaptureMoves()
{
    moveList.count = 0;
    U64 friendly = bitboard->getPieces(board->side);
    U64 enemy = bitboard->getPieces(board->side ^ 1);
    U64 empty = ~(friendly | enemy);

    if (board->side == WHITE)
        genWhitePawnMoves(empty, enemy, true);
    else
        genBlackPawnMoves(empty, enemy, true);

    genSlidingMoves(true);
    genNonSlidingMoves(true);
    return moveList;
}