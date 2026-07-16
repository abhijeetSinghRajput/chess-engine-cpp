#include "core/bitboard.hpp"
#include "core/zobristKeys.hpp"
#include "core/movegen.hpp"
#include "core/move.hpp"

void hashPiece(int sq, int piece)
{
    board->positionKey ^= pieceKeys[piece][sq];
}
void hashEnPassant()
{
    board->positionKey ^= pieceKeys[PIECE_EMPTY][board->enPassantSq];
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
    U64 allPieces = bitboard->getPieces(BOTH);
    U64 blocker = allPieces & bitboard->rookAttacks[sq];

    int index = (blocker * rookMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->rookLookupTable[sq][index];
    return attacks;
}

U64 getBishopAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(BOTH);
    U64 blocker = allPieces & bitboard->bishopAttacks[sq];

    int index = (blocker * bishopMagics[sq]) >> (64 - 12);
    U64 attacks = bitboard->bishopLookupTable[sq][index];
    return attacks;
}

bool isUnderAttack(int sq, int attackingSide)
{
    if (attackingSide == BLACK)
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[PIECE_BP] & bitboard->pawnAttacks[WHITE][sq])
            return true;
        if (bitboard->pieces[PIECE_BN] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[PIECE_BK] & bitboard->kingAttacks[sq])
            return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[PIECE_BR] & rookAttacks)
            return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[PIECE_BB] & bishopAttacks)
            return true;

        // Queen attacks
        if (bitboard->pieces[PIECE_BQ] & (rookAttacks | bishopAttacks))
            return true;
    }
    else
    {
        // Pawn, knight, and king attacks
        if (bitboard->pieces[PIECE_WP] & bitboard->pawnAttacks[BLACK][sq])
            return true;
        if (bitboard->pieces[PIECE_WN] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[PIECE_WK] & bitboard->kingAttacks[sq])
            return true;

        // Rook attacks
        U64 rookAttacks = getRookAttacks(sq);
        if (bitboard->pieces[PIECE_WR] & rookAttacks)
            return true;

        // Bishop attacks
        U64 bishopAttacks = getBishopAttacks(sq);
        if (bitboard->pieces[PIECE_WB] & bishopAttacks)
            return true;

        // Queen attacks
        if (bitboard->pieces[PIECE_WQ] & (rookAttacks | bishopAttacks))
            return true;
    }
    return false;
}

int materialDraw()
{

    if (!board->pieceCount[PIECE_WR] && !board->pieceCount[PIECE_BR] && !board->pieceCount[PIECE_WQ] && !board->pieceCount[PIECE_BQ])
    {
        if (!board->pieceCount[PIECE_BB] && !board->pieceCount[PIECE_WB])
        {
            if (board->pieceCount[PIECE_WN] < 3 && board->pieceCount[PIECE_BN] < 3)
            {
                return true;
            }
        }
        else if (!board->pieceCount[PIECE_WN] && !board->pieceCount[PIECE_BN])
        {
            if (abs(board->pieceCount[PIECE_WB] - board->pieceCount[PIECE_BB]) < 2)
            {
                return true;
            }
        }
        else if ((board->pieceCount[PIECE_WN] < 3 && !board->pieceCount[PIECE_WB]) || (board->pieceCount[PIECE_WB] == 1 && !board->pieceCount[PIECE_WN]))
        {
            if ((board->pieceCount[PIECE_BN] < 3 && !board->pieceCount[PIECE_BB]) || (board->pieceCount[PIECE_BB] == 1 && !board->pieceCount[PIECE_BN]))
            {
                return true;
            }
        }
    }
    else if (!board->pieceCount[PIECE_WQ] && !board->pieceCount[PIECE_BQ])
    {
        if (board->pieceCount[PIECE_WR] == 1 && board->pieceCount[PIECE_BR] == 1)
        {
            if ((board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB]) < 2 && (board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB]) < 2)
            {
                return true;
            }
        }
        else if (board->pieceCount[PIECE_WR] == 1 && !board->pieceCount[PIECE_BR])
        {
            if ((board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB] == 0) && (((board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB]) == 1) || ((board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB]) == 2)))
            {
                return true;
            }
        }
        else if (board->pieceCount[PIECE_BR] == 1 && !board->pieceCount[PIECE_WR])
        {
            if ((board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB] == 0) && (((board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB]) == 1) || ((board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB]) == 2)))
            {
                return true;
            }
        }
    }
    return false;
}

void newGame()
{
    board->parseFen(START_FEN);
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
    int promotionPiece = PIECE_EMPTY;

    if(move_str.length() == 5){
        switch (move_str[4])
        {
            case 'q': promotionPiece = (move_str[3] == '8')? PIECE_WQ : PIECE_BQ; break;
            case 'r': promotionPiece = (move_str[3] == '8')? PIECE_WR : PIECE_BR; break;
            case 'b': promotionPiece = (move_str[3] == '8')? PIECE_WB : PIECE_BB; break;
            case 'n': promotionPiece = (move_str[3] == '8')? PIECE_WN : PIECE_BN; break;

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


int bigPieceCount(int side)
{
    if (side == WHITE)
        return board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB] + board->pieceCount[PIECE_WR] + board->pieceCount[PIECE_WQ];
    else
        return board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB] + board->pieceCount[PIECE_BR] + board->pieceCount[PIECE_BQ];
}