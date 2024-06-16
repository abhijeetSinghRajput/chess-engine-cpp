#include "bitboard.hpp"
#include "zobristKeys.hpp"
#include "movegen.hpp"
#include "move.hpp"

void hashPiece(int sq, int piece)
{
    board->positionKey ^= pieceKeys[piece][sq];
}
void hashEnPassant()
{
    board->positionKey ^= pieceKeys[empty][board->enPassantSq];
}
void hashCastle()
{
    board->positionKey ^= castleKeys[board->castlePermission];
}
void hashSide()
{
    board->positionKey ^= sideKey;
}

U64 getRookAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->rookAttacks[sq];

    int index = (blocker * rookMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->rookLookupTable[sq][index];
    return attacks;
}

U64 getBishopAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->bishopAttacks[sq];

    int index = (blocker * bishopMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->bishopLookupTable[sq][index];
    return attacks;
}

bool isUnderAttack(int sq, int attackingSide)
{
    if (attackingSide == black)
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[bp] & bitboard->pawnAttacks[white][sq])
            return true;
        if (bitboard->pieces[bn] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[bk] & bitboard->kingAttacks[sq])
            return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[br] & rookAttacks)
            return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[bb] & bishopAttacks)
            return true;

        // Queen attacks
        if (bitboard->pieces[bq] & (rookAttacks | bishopAttacks))
            return true;
    }
    else
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[wp] & bitboard->pawnAttacks[black][sq])
            return true;
        if (bitboard->pieces[wn] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[wk] & bitboard->kingAttacks[sq])
            return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[wr] & rookAttacks)
            return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[wb] & bishopAttacks)
            return true;

        // Queen attacks
        if (bitboard->pieces[wq] & (rookAttacks | bishopAttacks))
            return true;
    }
    return false;
}

int materialDraw()
{

    if (!board->pieceCount[wr] && !board->pieceCount[br] && !board->pieceCount[wq] && !board->pieceCount[bq])
    {
        if (!board->pieceCount[bb] && !board->pieceCount[wb])
        {
            if (board->pieceCount[wn] < 3 && board->pieceCount[bn] < 3)
            {
                return true;
            }
        }
        else if (!board->pieceCount[wn] && !board->pieceCount[bn])
        {
            if (abs(board->pieceCount[wb] - board->pieceCount[bb]) < 2)
            {
                return true;
            }
        }
        else if ((board->pieceCount[wn] < 3 && !board->pieceCount[wb]) || (board->pieceCount[wb] == 1 && !board->pieceCount[wn]))
        {
            if ((board->pieceCount[bn] < 3 && !board->pieceCount[bb]) || (board->pieceCount[bb] == 1 && !board->pieceCount[bn]))
            {
                return true;
            }
        }
    }
    else if (!board->pieceCount[wq] && !board->pieceCount[bq])
    {
        if (board->pieceCount[wr] == 1 && board->pieceCount[br] == 1)
        {
            if ((board->pieceCount[wn] + board->pieceCount[wb]) < 2 && (board->pieceCount[bn] + board->pieceCount[bb]) < 2)
            {
                return true;
            }
        }
        else if (board->pieceCount[wr] == 1 && !board->pieceCount[br])
        {
            if ((board->pieceCount[wn] + board->pieceCount[wb] == 0) && (((board->pieceCount[bn] + board->pieceCount[bb]) == 1) || ((board->pieceCount[bn] + board->pieceCount[bb]) == 2)))
            {
                return true;
            }
        }
        else if (board->pieceCount[br] == 1 && !board->pieceCount[wr])
        {
            if ((board->pieceCount[bn] + board->pieceCount[bb] == 0) && (((board->pieceCount[wn] + board->pieceCount[wb]) == 1) || ((board->pieceCount[wn] + board->pieceCount[wb]) == 2)))
            {
                return true;
            }
        }
    }
    return false;
}

void newGame()
{
    board->parseFen(startFen);
    // more
}

int parseMove(std::string &move_str)
{
    if (move_str.length() < 4) return 0;
    if (move_str[0] < 'a' && move_str[0] > 'h') return 0;
    if (move_str[1] < '1' && move_str[1] > '8') return 0;
    if (move_str[2] < 'a' && move_str[2] > 'h') return 0;
    if (move_str[3] < '1' && move_str[3] > '8') return 0;

    int from = fileRank2Sq(move_str[0] - 'a', move_str[1] - '1');
    int to = fileRank2Sq(move_str[2] - 'a', move_str[3] - '1');
    int move;
    int promotionPiece = empty;

    if(move_str.length() == 5){
        switch (move_str[4])
        {
            case 'q': promotionPiece = (move_str[3] == '8')? wq : bq; break;
            case 'r': promotionPiece = (move_str[3] == '8')? wr : br; break;
            case 'b': promotionPiece = (move_str[3] == '8')? wb : bb; break;
            case 'n': promotionPiece = (move_str[3] == '8')? wn : bn; break;

            default: break;
        }
    }
   
    std::vector<std::pair<int, int>> moves = generateMoves();
    for (auto &pair : moves)
    {
        move = pair.first;
        if(moveFrom(move) == from && moveTo(move) == to && movePromotionPiece(move) == promotionPiece){
            return move;
        }
    }
    return 0;
}
