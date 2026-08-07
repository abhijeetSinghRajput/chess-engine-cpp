#pragma once

struct ScoredMove
{
    int move;
    int score;
};

struct MoveList
{
    ScoredMove moves[218];
    int count = 0;

    inline void push(int move, int score)
    {
        moves[count].move = move;
        moves[count].score = score;
        ++count;
    }
};

void addCaptureMove(MoveList &list, int move);
void addQuiteMove(MoveList &list, int move);
void addEnPassantMove(MoveList &list, int move);
void addWhitePawnQuietMove(MoveList &list, int from, int to);
void addBlackPawnQuietMove(MoveList &list, int from, int to);
void addWhiteCaptureMove(MoveList &list, int from, int to, int capture);
void addBlackCaptureMove(MoveList &list, int from, int to, int capture);

void genNonSlidingMoves(MoveList &list, bool capturesOnly = false);
void genSlidingMoves(MoveList &list, bool capturesOnly = false);

void generateMoves(MoveList &list);
void generateCaptureMoves(MoveList &list);

// Legal-move-generation helpers (used by generateMoves only).
// King moves get their own function: they're never restricted by pins or
// check-blocking, but do need attacked-square filtering with the king's
// own square excluded from occupancy (see kingMoveAttacked in utils.cpp).
void genKingMoves(MoveList &list, int kingSq, int side, bool capturesOnly = false);
void genKnightMovesLegal(MoveList &list, int side, U64 pinned, U64 targetMask);
void genSlidingMovesLegal(MoveList &list, int side, U64 pinned, int kingSq, U64 targetMask);

// En passant has a rare discovered-check geometry (removing both pawns from
// the same rank can expose the king) that isn't worth encoding generally —
// verified directly via make/unmake instead, only for EP candidates.
void tryAddEnPassant(MoveList &list, int from, int to);