#include "search/see.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/move.hpp"
#include <algorithm>

// ---- occupancy-parameterized slider attacks (SEE-local, doesn't touch bitboard.hpp) ----

static inline U64 seeRookAttacks(int sq, U64 occ)
{
    U64 blockers = occ & bitboard->rookAttacks[sq];
    int index = (blockers * rookMagics[sq]) >> (64 - 12);
    return Bitboard::rookLookupTable[sq][index];
}

static inline U64 seeBishopAttacks(int sq, U64 occ)
{
    U64 blockers = occ & bitboard->bishopAttacks[sq];
    int index = (blockers * bishopMagics[sq]) >> (64 - 12);
    return Bitboard::bishopLookupTable[sq][index];
}

// Pawn/knight/king attackers to `sq` never change as pieces are removed
// during the swap loop — their attack pattern doesn't depend on what's
// between the attacker and the target. Compute once, mask by shrinking
// `occ` on every iteration instead of recomputing from scratch.
static U64 nonSliderAttackersTo(int sq)
{
    U64 attackers = 0ULL;
    attackers |= bitboard->pawnAttacks[BLACK][sq] & bitboard->pieces[PIECE_WP];
    attackers |= bitboard->pawnAttacks[WHITE][sq] & bitboard->pieces[PIECE_BP];
    attackers |= bitboard->knightAttacks[sq] &
                 (bitboard->pieces[PIECE_WN] | bitboard->pieces[PIECE_BN]);
    attackers |= bitboard->kingAttacks[sq] &
                 (bitboard->pieces[PIECE_WK] | bitboard->pieces[PIECE_BK]);
    return attackers;
}

// Only sliders can reveal a *new* attacker when a piece is removed
// (x-ray) — so only these need recomputing against the shrinking `occ`.
static U64 sliderAttackersTo(int sq, U64 occ)
{
    U64 attackers = 0ULL;

    U64 rookLike = (bitboard->pieces[PIECE_WR] | bitboard->pieces[PIECE_WQ] |
                     bitboard->pieces[PIECE_BR] | bitboard->pieces[PIECE_BQ]) & occ;
    if (rookLike)
        attackers |= seeRookAttacks(sq, occ) & rookLike;

    U64 bishopLike = (bitboard->pieces[PIECE_WB] | bitboard->pieces[PIECE_WQ] |
                       bitboard->pieces[PIECE_BB] | bitboard->pieces[PIECE_BQ]) & occ;
    if (bishopLike)
        attackers |= seeBishopAttacks(sq, occ) & bishopLike;

    return attackers;
}

// Cheapest attacker for `attackingSide` among `attackers` (still-present per `occ`).
// Returns square of that attacker, or SQ_NONE if none.
static int leastValuableAttacker(U64 attackers, U64 occ, int attackingSide)
{
    static const int order[2][6] = {
        {PIECE_WP, PIECE_WN, PIECE_WB, PIECE_WR, PIECE_WQ, PIECE_WK},
        {PIECE_BP, PIECE_BN, PIECE_BB, PIECE_BR, PIECE_BQ, PIECE_BK}
    };

    for (int p : order[attackingSide])
    {
        U64 bb = attackers & occ & bitboard->pieces[p];
        if (bb)
            return __builtin_ctzll(bb);
    }
    return SQ_NONE;
}

// Classic "swap" algorithm (Fabien Letouzey). Non-destructive: only
// tracks a local `occ` bitmask, never touches the real board.
int see(int move)
{
    int from = moveFrom(move);
    int to   = moveTo(move);

    int attackerPiece = Board::pieces[from];
    int side           = PIECE_COLOR[attackerPiece];

    U64 occ = bitboard->getPieces(BOTH);

    // Handle en passant: captured pawn isn't on `to`.
    int capturedPiece;
    if (move & EN_PASSANT_FLAG)
    {
        int capSq = (side == WHITE) ? to - 8 : to + 8;
        capturedPiece = Board::pieces[capSq];
        occ &= ~(1ULL << capSq);
    }
    else
    {
        capturedPiece = Board::pieces[to];
    }

    int gain[32];
    int d = 0;
    gain[0] = pieceValue[capturedPiece];

    occ &= ~(1ULL << from);

    // Computed once — pawn/knight/king attackers never change mid-exchange.
    U64 nonSliders = nonSliderAttackersTo(to);
    U64 attackers = (nonSliders & occ) | sliderAttackersTo(to, occ);

    int sideToMove = side ^ 1;

    while (true)
    {
        int sq = leastValuableAttacker(attackers, occ, sideToMove);
        if (sq == SQ_NONE)
            break;

        d++;
        gain[d] = pieceValue[attackerPiece] - gain[d - 1];

        // Early cutoff: if even a "free" recapture can't make this
        // worthwhile, the exchange is settled.
        if (std::max(-gain[d - 1], gain[d]) < 0)
            break;

        attackerPiece = Board::pieces[sq];
        occ &= ~(1ULL << sq);

        // Only sliders need recomputing after a removal — non-sliders
        // are re-masked by the shrunk occ, not recomputed.
        attackers = (nonSliders & occ) | sliderAttackersTo(to, occ);

        sideToMove ^= 1;
    }

    while (--d >= 0)
        gain[d] = -std::max(-gain[d], gain[d + 1]);

    return gain[0];
}

bool seeGe(int move, int threshold)
{
    return see(move) >= threshold;
}