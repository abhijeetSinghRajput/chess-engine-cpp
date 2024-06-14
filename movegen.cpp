#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "utils.hpp"
#include "move.hpp"
#include "search.hpp"
#include "movegen.hpp"
#include <vector>

std::vector<std::pair<int, int>> moves;
//MvvLva = [victim][attacker]
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
    moves.push_back({ move, score});
}

void addQuiteMove(int move) {
    int score = 0;
    if(searchController->ply < 0 && searchController->ply >= 64){
        printf("---- %d\n",searchController->ply);
    }
    if (searchController->killers[searchController->ply][0] == move) {
        score = 900000;
    }
    else if (searchController->killers[searchController->ply][1] == move) {
        score = 800000;
    }
    else {
        int piece = board->pieces[moveFrom(move)];
        int toSq = moveTo(move);
        score = searchController->history[piece][toSq];
    }

    moves.push_back({ move, score});
}

void addEnPassantMove(int move) {
    //MvvLva[Pieces.wp][Pieces.bp] == MvvLva[Pieces.bp][Pieces.wp]
    int score = MvvLva[wp][bp] + 1000000;
    moves.push_back({ move, score});
}

void addWhitePawnQuietMove(int from, int to)
{
    // handling promotion move
    if (rankOf(to) == rank8)
    {
        addQuiteMove(buildMove(from, to, 0, wq, 0));
        addQuiteMove(buildMove(from, to, 0, wr, 0));
        addQuiteMove(buildMove(from, to, 0, wb, 0));
        addQuiteMove(buildMove(from, to, 0, wn, 0));
    }
    else
    {
        addQuiteMove(buildMove(from, to, 0, 0, 0));
    }
}
void addBlackPawnQuietMove(int from, int to)
{
    // handling promotion move
    if (rankOf(to) == rank1)
    {
        addQuiteMove(buildMove(from, to, 0, bq, 0));
        addQuiteMove(buildMove(from, to, 0, br, 0));
        addQuiteMove(buildMove(from, to, 0, bb, 0));
        addQuiteMove(buildMove(from, to, 0, bn, 0));
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
        addCaptureMove(buildMove(from, to, capture, wq, 0));
        addCaptureMove(buildMove(from, to, capture, wr, 0));
        addCaptureMove(buildMove(from, to, capture, wb, 0));
        addCaptureMove(buildMove(from, to, capture, wn, 0));
    }
    else
    {
        addCaptureMove(buildMove(from, to, capture, 0, 0));
    }
}
void addBlackCaptureMove(int from, int to, int capture)
{
    // handling promotion move
    if (rankOf(to) == rank1)
    {
        addCaptureMove(buildMove(from, to, capture, bq, 0));
        addCaptureMove(buildMove(from, to, capture, br, 0));
        addCaptureMove(buildMove(from, to, capture, bb, 0));
        addCaptureMove(buildMove(from, to, capture, bn, 0));
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

            U64 attacksPattern = (pieceType[piece] == 'n') ? bitboard->knightAttacks[sq] : bitboard->kingAttacks[sq];
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

            switch (pieceType[piece])
            {
                case 'r': attacksPattern = getRookAttacks(sq); break;
                case 'b': attacksPattern = getBishopAttacks(sq); break;
                case 'q': attacksPattern = getBishopAttacks(sq) | getRookAttacks(sq); break;
                default: break;
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
    moves.clear();
    if (board->side == white)
    {
        U64 wpBitboard = bitboard->pieces[wp];
        // loop white pawn
        while (wpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(wpBitboard)];
            wpBitboard &= wpBitboard - 1;

            if (board->pieces[sq + 10] == empty)
            {
                addWhitePawnQuietMove(sq, sq + 10);
                if (board->pieces[sq + 20] == empty && rankOf(sq) == rank2)
                {
                    addQuiteMove(buildMove(sq, sq + 20, 0, 0, pawnStartFlag));
                }
            }

            // add capture move
            if (board->pieces[sq + 9] != offBoard && pieceColor[board->pieces[sq + 9]] == black)
            {
                addWhiteCaptureMove(sq, sq + 9, board->pieces[sq + 9]);
            }
            if (board->pieces[sq + 11] != offBoard && pieceColor[board->pieces[sq + 11]] == black)
            {
                addWhiteCaptureMove(sq, sq + 11, board->pieces[sq + 11]);
            }

            // add enpassant move
            if (sq + 9 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq + 9, 0, 0, enPassantFlag));
            }
            if (sq + 11 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq + 11, 0, 0, enPassantFlag));
            }

        } // end loop

        // CASTLING
        if (board->castlePermission & castle_K)
        {
            if (board->pieces[f1] == empty && board->pieces[g1] == empty)
            {
                if (board->checkSq == noSq && !isUnderAttack(sq120To64[f1], black))
                {
                    addQuiteMove(buildMove(e1, g1, 0, 0, castleFlag));
                }
            }
        }
        if (board->castlePermission & castle_Q)
        {
            if (board->pieces[d1] == empty && board->pieces[c1] == empty && board->pieces[b1] == empty)
            {
                if (board->checkSq == noSq && !isUnderAttack(sq120To64[d1], black))
                {
                    addQuiteMove(buildMove(e1, c1, 0, 0, castleFlag));
                }
            }
        }
    }
    else
    {
        U64 bpBitboard = bitboard->pieces[bp];
        // loop black pawn
        while (bpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(bpBitboard)];
            bpBitboard &= bpBitboard - 1;

            if (board->pieces[sq - 10] == empty)
            {
                addBlackPawnQuietMove(sq, sq - 10);

                if (board->pieces[sq - 20] == empty && rankOf(sq) == rank7)
                {
                    addQuiteMove(buildMove(sq, sq - 20, 0, 0, pawnStartFlag));
                }
            }

            // add capture move
            if (board->pieces[sq - 9] != offBoard && pieceColor[board->pieces[sq - 9]] == white)
            {
                addBlackCaptureMove(sq, sq - 9, board->pieces[sq - 9]);
            }
            if (board->pieces[sq - 11] != offBoard && pieceColor[board->pieces[sq - 11]] == white)
            {
                addBlackCaptureMove(sq, sq - 11, board->pieces[sq - 11]);
            }

            // add enpassant move
            if (sq - 9 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq - 9, 0, 0, enPassantFlag));
            }
            if (sq - 11 == board->enPassantSq)
            {
                addEnPassantMove(buildMove(sq, sq - 11, 0, 0, enPassantFlag));
            }
        }

        // castling
        if (board->castlePermission & castle_k)
        {
            if (board->pieces[f8] == empty && board->pieces[g8] == empty)
            {
                if (board->checkSq == noSq && !isUnderAttack(sq120To64[f8], white))
                {
                    addQuiteMove(buildMove(e8, g8, 0, 0, castleFlag));
                }
            }
        }
        if (board->castlePermission & castle_q)
        {
            if (board->pieces[d8] == empty && board->pieces[c8] == empty && board->pieces[b8] == empty)
            {
                if (board->checkSq == noSq && !isUnderAttack(sq120To64[d8], white))
                {
                    addQuiteMove(buildMove(e8, c8, 0, 0, castleFlag));
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
    moves.clear();
    if (board->side == white)
    {
        U64 wpBitboard = bitboard->pieces[wp];
        // loop white pawn
        while (wpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(wpBitboard)];
            wpBitboard &= wpBitboard - 1;

            // add capture move
            if (board->pieces[sq + 9] != offBoard && pieceColor[board->pieces[sq + 9]] == black)
            {
                addWhiteCaptureMove(sq, sq + 9, board->pieces[sq + 9]);
            }
            if (board->pieces[sq + 11] != offBoard && pieceColor[board->pieces[sq + 11]] == black)
            {
                addWhiteCaptureMove(sq, sq + 11, board->pieces[sq + 11]);
            }

        } // end loop
    }
    else
    {
        U64 bpBitboard = bitboard->pieces[bp];
        // loop black pawn
        while (bpBitboard)
        {
            int sq = sq64To120[__builtin_ctzll(bpBitboard)];
            bpBitboard &= bpBitboard - 1;

            // add capture move
            if (board->pieces[sq - 9] != offBoard && pieceColor[board->pieces[sq - 9]] == white)
            {
                addBlackCaptureMove(sq, sq - 9, board->pieces[sq - 9]);
            }
            if (board->pieces[sq - 11] != offBoard && pieceColor[board->pieces[sq - 11]] == white)
            {
                addBlackCaptureMove(sq, sq - 11, board->pieces[sq - 11]);
            }
        }
    }
    genSlidingMoves(true);
    genNonSlidingMoves(true);
    return moves;
}
