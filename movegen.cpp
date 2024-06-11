#include "defs.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "utils.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include <vector>

std::vector<int> moves;

void addCaptureMove(int move){
    moves.push_back(move);
}

void addQuiteMove(int move) {
    moves.push_back(move);
}

void addEnPassantMove(int move) {
    moves.push_back(move);
}

void addWhitePawnQuietMove(int from,int to) {
    //handling promotion move
    if (rankOf(to) == rank8) {
        addQuiteMove(buildMove(from, to, 0, wq, 0));
        addQuiteMove(buildMove(from, to, 0, wr, 0));
        addQuiteMove(buildMove(from, to, 0, wb, 0));
        addQuiteMove(buildMove(from, to, 0, wn, 0));
    }
    else {
        addQuiteMove(buildMove(from, to, 0, 0, 0));
    }
}
void addBlackPawnQuietMove(int from, int to) {
    //handling promotion move
    if (rankOf(to) == rank1) {
        addQuiteMove(buildMove(from, to, 0, bq, 0));
        addQuiteMove(buildMove(from, to, 0, br, 0));
        addQuiteMove(buildMove(from, to, 0, bb, 0));
        addQuiteMove(buildMove(from, to, 0, bn, 0));
    }
    else {
        addQuiteMove(buildMove(from, to, 0, 0, 0));
    }
}



void addWhiteCaptureMove(int from,int to,int capture) {
    //handling promotion move
    if (rankOf(to) == rank8) {
        addCaptureMove(buildMove(from, to, capture, wq, 0));
        addCaptureMove(buildMove(from, to, capture, wr, 0));
        addCaptureMove(buildMove(from, to, capture, wb, 0));
        addCaptureMove(buildMove(from, to, capture, wn, 0));
    }
    else {
        addCaptureMove(buildMove(from, to, capture, 0, 0));
    }
}
void addBlackCaptureMove(int from,int to,int capture) {
    //handling promotion move
    if (rankOf(to) == rank1) {
        addCaptureMove(buildMove(from, to, capture, bq, 0));
        addCaptureMove(buildMove(from, to, capture, br, 0));
        addCaptureMove(buildMove(from, to, capture, bb, 0));
        addCaptureMove(buildMove(from, to, capture, bn, 0));
    }
    else {
        addCaptureMove(buildMove(from, to, capture, 0, 0));
    }
}


void genKnightMoves()
{
    U64 knightBitboard = (board->side == white) ? bitboard->pieces[wn] : bitboard->pieces[bn];
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    while (knightBitboard)
    {
        int sq = __builtin_ctzll(knightBitboard);
        knightBitboard &= knightBitboard - 1;

        U64 attacksPattern = bitboard->knightAttacks[sq];
        attacksPattern &= ~friendlyPiecesBitboard;
        while (attacksPattern)
        {
            int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
            if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
            {
                addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
            }
            else
            {
                addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
            }

            attacksPattern &= attacksPattern - 1;
        }
    }
}

void genKingMoves()
{
    U64 kingBitboard = (board->side == white) ? bitboard->pieces[wk] : bitboard->pieces[bk];
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    while (kingBitboard)
    {
        int sq = __builtin_ctzll(kingBitboard);
        kingBitboard &= kingBitboard - 1;

        U64 attacksPattern = bitboard->kingAttacks[sq];
        attacksPattern &= ~friendlyPiecesBitboard;
        while (attacksPattern)
        {
            int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
            if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
            {
                addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
            }
            else
            {
                addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
            }

            attacksPattern &= attacksPattern - 1;
        }
    }
}

void genRookMoves()
{
    U64 rookBitboard = (board->side == white) ? bitboard->pieces[wr] : bitboard->pieces[br];
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    while (rookBitboard)
    {
        int sq = __builtin_ctzll(rookBitboard);
        rookBitboard &= rookBitboard - 1;
        U64 attacksPattern = getRookAttacks(sq);

        // remove friendly blockers
        attacksPattern &= ~friendlyPiecesBitboard;

        while (attacksPattern)
        {
            int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
            if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
            {
                addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
            }
            else
            {
                addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
            }

            attacksPattern &= attacksPattern - 1;
        }
    }
}

void genBishopMoves()
{
    U64 bishopBitboard = (board->side == white) ? bitboard->pieces[wb] : bitboard->pieces[bb];
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    while (bishopBitboard)
    {
        int sq = __builtin_ctzll(bishopBitboard);
        bishopBitboard &= bishopBitboard - 1;
        U64 attacksPattern = getBishopAttacks(sq);
        // remove friendly blockers
        attacksPattern &= ~friendlyPiecesBitboard;
        while (attacksPattern)
        {
            int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
            if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
            {
                addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
            }
            else
            {
                addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
            }

            attacksPattern &= attacksPattern - 1;
        }
    }
}

void genQueenMoves()
{
    U64 queenBitboard = (board->side == white) ? bitboard->pieces[wq] : bitboard->pieces[bq];
    U64 friendlyPiecesBitboard = bitboard->getPieces(board->side);
    U64 enemyPiecesBitboard = bitboard->getPieces(board->side ^ 1);

    while (queenBitboard)
    {
        int sq = __builtin_ctzll(queenBitboard);
        queenBitboard &= queenBitboard - 1;
        U64 attacksPattern = getBishopAttacks(sq) | getRookAttacks(sq);

        // remove friendly blockers
        attacksPattern &= ~friendlyPiecesBitboard;

        while (attacksPattern)
        {
            int targetSq = sq64To120[__builtin_ctzll(attacksPattern)];
            if (enemyPiecesBitboard & (1ULL << sq120To64[targetSq]))
            {
                addCaptureMove(buildMove(sq64To120[sq], targetSq, board->pieces[targetSq], 0, 0));
            }
            else
            {
                addQuiteMove(buildMove(sq64To120[sq], targetSq, 0, 0, 0));
            }

            attacksPattern &= attacksPattern - 1;
        }
    }
}


std::vector<int> &generateMoves()
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
    genRookMoves();
    genBishopMoves();
    genQueenMoves();
    genKingMoves();
    genKnightMoves();
    return moves;
}
