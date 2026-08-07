
#include "core/defs.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/utils.hpp"
#include "core/move.hpp"
#include "search/search.hpp"
#include "core/movegen.hpp"

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

void addCaptureMove(MoveList &list, int move)
{
    int victim = moveCapturePiece(move);
    int attacker = board->pieces[moveFrom(move)];
    int score = MvvLva[victim][attacker] + 1000000;
    list.push(move, score);
}

void addQuiteMove(MoveList &list, int move)
{
    int score = 0;
    if (searchController->killers[searchController->ply][0] == move)
    {
        score = 900000;
    }
    else if (searchController->killers[searchController->ply][1] == move)
    {
        score = 800000;
    }
    else
    {
        int piece = board->pieces[moveFrom(move)];
        int toSq = moveTo(move);
        score = searchController->history[piece][toSq];
    }
    list.push(move, score);
}

void addEnPassantMove(MoveList &list, int move)
{
    int score = MvvLva[PIECE_WP][PIECE_BP] + 1000000;
    list.push(move, score);
}

void addWhitePawnQuietMove(MoveList &list, int from, int to)
{
    if (rankOf(to) == RANK_8)
    {
        addQuiteMove(list, buildMove(from, to, 0, PIECE_WQ, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_WR, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_WB, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_WN, 0));
    }
    else
    {
        addQuiteMove(list, buildMove(from, to, 0, 0, 0));
    }
}

void addBlackPawnQuietMove(MoveList &list, int from, int to)
{
    if (rankOf(to) == RANK_1)
    {
        addQuiteMove(list, buildMove(from, to, 0, PIECE_BQ, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_BR, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_BB, 0));
        addQuiteMove(list, buildMove(from, to, 0, PIECE_BN, 0));
    }
    else
    {
        addQuiteMove(list, buildMove(from, to, 0, 0, 0));
    }
}

void addWhiteCaptureMove(MoveList &list, int from, int to, int capture)
{
    if (rankOf(to) == RANK_8)
    {
        addCaptureMove(list, buildMove(from, to, capture, PIECE_WQ, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_WR, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_WB, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_WN, 0));
    }
    else
    {
        addCaptureMove(list, buildMove(from, to, capture, 0, 0));
    }
}

void addBlackCaptureMove(MoveList &list, int from, int to, int capture)
{
    if (rankOf(to) == RANK_1)
    {
        addCaptureMove(list, buildMove(from, to, capture, PIECE_BQ, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_BR, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_BB, 0));
        addCaptureMove(list, buildMove(from, to, capture, PIECE_BN, 0));
    }
    else
    {
        addCaptureMove(list, buildMove(from, to, capture, 0, 0));
    }
}

void genNonSlidingMoves(MoveList &list, bool capturesOnly)
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
                int targetSq = __builtin_ctzll(attacksPattern);
                if (enemyPiecesBitboard & (1ULL << targetSq))
                {
                    addCaptureMove(list, buildMove(sq, targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(list, buildMove(sq, targetSq, 0, 0, 0));
                }
                attacksPattern &= attacksPattern - 1;
            }
        }
    }
}

void genSlidingMoves(MoveList &list, bool capturesOnly)
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
            case 'r': attacksPattern = getRookAttacks(sq); break;
            case 'b': attacksPattern = getBishopAttacks(sq); break;
            case 'q': attacksPattern = getBishopAttacks(sq) | getRookAttacks(sq); break;
            default: break;
            }
            attacksPattern &= ~friendlyPiecesBitboard;

            while (attacksPattern)
            {
                int targetSq = __builtin_ctzll(attacksPattern);
                if (enemyPiecesBitboard & (1ULL << targetSq))
                {
                    addCaptureMove(list, buildMove(sq, targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(list, buildMove(sq, targetSq, 0, 0, 0));
                }
                attacksPattern &= attacksPattern - 1;
            }
        }
    }
}

void generateMoves(MoveList &list)
{
    list.count = 0;

    if (board->side == WHITE)
    {
        U64 wpBitboard = bitboard->pieces[PIECE_WP];
        while (wpBitboard)
        {
            int sq = __builtin_ctzll(wpBitboard);
            wpBitboard &= wpBitboard - 1;

            if (board->pieces[sq + 8] == PIECE_EMPTY)
            {
                addWhitePawnQuietMove(list, sq, sq + 8);
                if (board->pieces[sq + 16] == PIECE_EMPTY && rankOf(sq) == RANK_2)
                {
                    addQuiteMove(list, buildMove(sq, sq + 16, 0, 0, PAWN_START_FLAG));
                }
            }

            if (fileOf(sq) > FILE_A && PIECE_COLOR[board->pieces[sq + 7]] == BLACK)
            {
                addWhiteCaptureMove(list, sq, sq + 7, board->pieces[sq + 7]);
            }
            if (fileOf(sq) < FILE_H && PIECE_COLOR[board->pieces[sq + 9]] == BLACK)
            {
                addWhiteCaptureMove(list, sq, sq + 9, board->pieces[sq + 9]);
            }

            if (fileOf(sq) > FILE_A && sq + 7 == board->enPassantSq)
            {
                addEnPassantMove(list, buildMove(sq, sq + 7, 0, 0, EN_PASSANT_FLAG));
            }
            if (fileOf(sq) < FILE_H && sq + 9 == board->enPassantSq)
            {
                addEnPassantMove(list, buildMove(sq, sq + 9, 0, 0, EN_PASSANT_FLAG));
            }
        }

        if (board->castlePermission & castle_K)
        {
            if (board->pieces[SQ_F1] == PIECE_EMPTY && board->pieces[SQ_G1] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(SQ_F1, BLACK))
                {
                    addQuiteMove(list, buildMove(SQ_E1, SQ_G1, 0, 0, CASTLE_FLAG));
                }
            }
        }
        if (board->castlePermission & castle_Q)
        {
            if (board->pieces[SQ_D1] == PIECE_EMPTY && board->pieces[SQ_C1] == PIECE_EMPTY && board->pieces[SQ_B1] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(SQ_D1, BLACK))
                {
                    addQuiteMove(list, buildMove(SQ_E1, SQ_C1, 0, 0, CASTLE_FLAG));
                }
            }
        }
    }
    else
    {
        U64 bpBitboard = bitboard->pieces[PIECE_BP];
        while (bpBitboard)
        {
            int sq = __builtin_ctzll(bpBitboard);
            bpBitboard &= bpBitboard - 1;

            if (board->pieces[sq - 8] == PIECE_EMPTY)
            {
                addBlackPawnQuietMove(list, sq, sq - 8);
                if (board->pieces[sq - 16] == PIECE_EMPTY && rankOf(sq) == RANK_7)
                {
                    addQuiteMove(list, buildMove(sq, sq - 16, 0, 0, PAWN_START_FLAG));
                }
            }

            if (fileOf(sq) < FILE_H && PIECE_COLOR[board->pieces[sq - 7]] == WHITE)
            {
                addBlackCaptureMove(list, sq, sq - 7, board->pieces[sq - 7]);
            }
            if (fileOf(sq) > FILE_A && PIECE_COLOR[board->pieces[sq - 9]] == WHITE)
            {
                addBlackCaptureMove(list, sq, sq - 9, board->pieces[sq - 9]);
            }

            if (fileOf(sq) > FILE_A && sq - 9 == board->enPassantSq)
            {
                addEnPassantMove(list, buildMove(sq, sq - 9, 0, 0, EN_PASSANT_FLAG));
            }
            if (fileOf(sq) < FILE_H && sq - 7 == board->enPassantSq)
            {
                addEnPassantMove(list, buildMove(sq, sq - 7, 0, 0, EN_PASSANT_FLAG));
            }
        }

        if (board->castlePermission & castle_k)
        {
            if (board->pieces[SQ_F8] == PIECE_EMPTY && board->pieces[SQ_G8] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(SQ_F8, WHITE))
                {
                    addQuiteMove(list, buildMove(SQ_E8, SQ_G8, 0, 0, CASTLE_FLAG));
                }
            }
        }
        if (board->castlePermission & castle_q)
        {
            if (board->pieces[SQ_D8] == PIECE_EMPTY && board->pieces[SQ_C8] == PIECE_EMPTY && board->pieces[SQ_B8] == PIECE_EMPTY)
            {
                if (board->checkSq == SQ_NONE && !isUnderAttack(SQ_D8, WHITE))
                {
                    addQuiteMove(list, buildMove(SQ_E8, SQ_C8, 0, 0, CASTLE_FLAG));
                }
            }
        }
    }

    genSlidingMoves(list);
    genNonSlidingMoves(list);
}

void generateCaptureMoves(MoveList &list)
{
    list.count = 0;

    if (board->side == WHITE)
    {
        U64 wpBitboard = bitboard->pieces[PIECE_WP];
        while (wpBitboard)
        {
            int sq = __builtin_ctzll(wpBitboard);
            wpBitboard &= wpBitboard - 1;

            if (fileOf(sq) > FILE_A && PIECE_COLOR[board->pieces[sq + 7]] == BLACK)
            {
                addWhiteCaptureMove(list, sq, sq + 7, board->pieces[sq + 7]);
            }
            if (fileOf(sq) < FILE_H && PIECE_COLOR[board->pieces[sq + 9]] == BLACK)
            {
                addWhiteCaptureMove(list, sq, sq + 9, board->pieces[sq + 9]);
            }
        }
    }
    else
    {
        U64 bpBitboard = bitboard->pieces[PIECE_BP];
        while (bpBitboard)
        {
            int sq = __builtin_ctzll(bpBitboard);
            bpBitboard &= bpBitboard - 1;

            if (fileOf(sq) > FILE_A && PIECE_COLOR[board->pieces[sq - 9]] == WHITE)
            {
                addBlackCaptureMove(list, sq, sq - 9, board->pieces[sq - 9]);
            }
            if (fileOf(sq) < FILE_H && PIECE_COLOR[board->pieces[sq - 7]] == WHITE)
            {
                addBlackCaptureMove(list, sq, sq - 7, board->pieces[sq - 7]);
            }
        }
    }

    genSlidingMoves(list, true);
    genNonSlidingMoves(list, true);
}