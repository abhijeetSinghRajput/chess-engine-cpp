#include "core/defs.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/utils.hpp"
#include "core/move.hpp"
#include "search/search.hpp"
#include "core/movegen.hpp"
#include <vector>

std::vector<std::pair<int, int>> moves;

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

void addCaptureMove(int move)
{
    int victim = moveCapturePiece(move);
    int attacker = board->pieces[moveFrom(move)];
    int score = MvvLva[victim][attacker] + 1000000;
    moves.push_back({move, score});
}

void addQuiteMove(int move)
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
        // Give a small bonus to all moves based on history
        score = searchController->history[piece][toSq];
        if (score == 0) {
            // Give random small scores to avoid ties
            score = 1 + (moveFrom(move) % 1000);
        }
    }
    moves.push_back({move, score});
}

void addEnPassantMove(int move)
{
    // MvvLva[Pieces.wp][Pieces.bp] == MvvLva[Pieces.bp][Pieces.wp]
    int score = MvvLva[PIECE_WP][PIECE_BP] + 1000000;
    moves.push_back({move, score});
}

void addWhitePawnQuietMove(int from, int to)
{
    // handling promotion move
    if (rankOf(to) == rank8)
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
    // handling promotion move
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
    // handling promotion move
    if (rankOf(to) == rank8)
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
    // handling promotion move
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

void genNonSlidingMoves(bool capturesOnly)
{
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    for (int piece : nonSlidingPieces[board->side])
    {
        U64 pieceBitboard = bitboard->pieces[piece];

        // traverse each square where these pieces are
        while (pieceBitboard)
        {
            int sq = __builtin_ctzll(pieceBitboard);
            pieceBitboard &= pieceBitboard - 1;

            U64 attacksPattern = (PIECE_TYPE[piece] == 'n') ? bitboard->knightAttacks[sq] : bitboard->kingAttacks[sq];
            attacksPattern &= ~friendlyPiecesBitboard;

            while (attacksPattern)
            {
                int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
                if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
                {
                    addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
                }

                attacksPattern &= attacksPattern - 1;
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
        // traverse each square
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
            // remove friendly blockers
            attacksPattern &= ~friendlyPiecesBitboard;

            // traverse the squares where these pieces can move
            while (attacksPattern)
            {
                int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
                if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
                {
                    addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
                }
                else if (!capturesOnly)
                {
                    addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
                }

                attacksPattern &= attacksPattern - 1;
            }
        }
    }
}

std::vector<std::pair<int, int>> &generateMoves()
{
    if (moves.capacity() < 218)
    {
        moves.reserve(218);
    }
    moves.resize(0);

    if (board->side == WHITE)
    {
        U64 wpBitboard = bitboard->pieces[PIECE_WP];
        // loop white pawn
        while (wpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(wpBitboard)];
            wpBitboard &= wpBitboard - 1;

            if (board->pieces[sq + 10] == PIECE_EMPTY)
            {
                addWhitePawnQuietMove(sq, sq + 10);
                if (board->pieces[sq + 20] == PIECE_EMPTY && rankOf(sq) == RANK_2)
                {
                    addQuiteMove(buildMove(sq, sq + 20, 0, 0, PAWN_START_FLAG));
                }
            }

            // add capture move
            if (board->pieces[sq + 9] != offBoard && PIECE_COLOR[board->pieces[sq + 9]] == BLACK)
            {
                addWhiteCaptureMove(sq, sq + 9, board->pieces[sq + 9]);
            }
            if (board->pieces[sq + 11] != offBoard && PIECE_COLOR[board->pieces[sq + 11]] == BLACK)
            {
                addWhiteCaptureMove(sq, sq + 11, board->pieces[sq + 11]);
            }

            // add enpassant move
            if (sq + 9 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq + 9, 0, 0, EN_PASSANT_FLAG));
            }
            if (sq + 11 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq + 11, 0, 0, EN_PASSANT_FLAG));
            }

        } // end loop

        // CASTLING
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
        U64 bpBitboard = bitboard->pieces[PIECE_BP];
        // loop black pawn
        while (bpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(bpBitboard)];
            bpBitboard &= bpBitboard - 1;

            if (board->pieces[sq - 10] == PIECE_EMPTY)
            {
                addBlackPawnQuietMove(sq, sq - 10);

                if (board->pieces[sq - 20] == PIECE_EMPTY && rankOf(sq) == RANK_7)
                {
                    addQuiteMove(buildMove(sq, sq - 20, 0, 0, PAWN_START_FLAG));
                }
            }

            // add capture move
            if (board->pieces[sq - 9] != offBoard && PIECE_COLOR[board->pieces[sq - 9]] == WHITE)
            {
                addBlackCaptureMove(sq, sq - 9, board->pieces[sq - 9]);
            }
            if (board->pieces[sq - 11] != offBoard && PIECE_COLOR[board->pieces[sq - 11]] == WHITE)
            {
                addBlackCaptureMove(sq, sq - 11, board->pieces[sq - 11]);
            }

            // add enpassant move
            if (sq - 9 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq - 9, 0, 0, EN_PASSANT_FLAG));
            }
            if (sq - 11 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq - 11, 0, 0, EN_PASSANT_FLAG));
            }
        }

        // castling
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
    genSlidingMoves();
    genNonSlidingMoves();
    return moves;
}

std::vector<std::pair<int, int>> &generateCaptureMoves()
{
    if (moves.capacity() < 218)
    {
        moves.reserve(218);
    }
    moves.resize(0);

    if (board->side == WHITE)
    {
        U64 wpBitboard = bitboard->pieces[PIECE_WP];
        // loop white pawn
        while (wpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(wpBitboard)];
            wpBitboard &= wpBitboard - 1;

            // add capture move
            if (board->pieces[sq + 9] != offBoard && PIECE_COLOR[board->pieces[sq + 9]] == BLACK)
            {
                addWhiteCaptureMove(sq, sq + 9, board->pieces[sq + 9]);
            }
            if (board->pieces[sq + 11] != offBoard && PIECE_COLOR[board->pieces[sq + 11]] == BLACK)
            {
                addWhiteCaptureMove(sq, sq + 11, board->pieces[sq + 11]);
            }

        } // end loop
    }
    else
    {
        U64 bpBitboard = bitboard->pieces[PIECE_BP];
        // loop black pawn
        while (bpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(bpBitboard)];
            bpBitboard &= bpBitboard - 1;

            // add capture move
            if (board->pieces[sq - 9] != offBoard && PIECE_COLOR[board->pieces[sq - 9]] == WHITE)
            {
                addBlackCaptureMove(sq, sq - 9, board->pieces[sq - 9]);
            }
            if (board->pieces[sq - 11] != offBoard && PIECE_COLOR[board->pieces[sq - 11]] == WHITE)
            {
                addBlackCaptureMove(sq, sq - 11, board->pieces[sq - 11]);
            }
        }
    }
    genSlidingMoves(true);
    genNonSlidingMoves(true);
    return moves;
}
