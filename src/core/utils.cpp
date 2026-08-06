#include "core/bitboard.hpp"
#include "core/zobristKeys.hpp"
#include "core/movegen.hpp"
#include "core/move.hpp"
#include <iostream>

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
    // getPieces(both) is now an O(1) load off Bitboard::occupied[] (see
    // bitboard.hpp/cpp) instead of OR-ing 12 piece bitboards together
    // on every call - this function runs on every sliding move
    // generated and every isUnderAttack() check.
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->rookAttacks[sq];

    int index = (blocker * rookMagics[sq]) >> (64 - 12);
    return bitboard->rookLookupTable[sq][index];
}

U64 getBishopAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(both);
    U64 blocker = allPieces & bitboard->bishopAttacks[sq];

    int index = (blocker * bishopMagics[sq]) >> (64 - 12);
    return bitboard->bishopLookupTable[sq][index];
}

bool isUnderAttack(int sq, int attackingSide)
{
    if (attackingSide == black)
    {
        // Pawn, knight, and king attacks - cheap array lookups, checked first
        if (bitboard->pieces[bp] & bitboard->pawnAttacks[white][sq])
            return true;
        if (bitboard->pieces[bn] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[bk] & bitboard->kingAttacks[sq])
            return true;

        // Rook + queen share the rook attack pattern. Skip the magic
        // lookup entirely (multiply, shift, 32KB table probe) when
        // black has neither piece left on the board - common in
        // endgames and free when it doesn't apply.
        U64 rookLike = bitboard->pieces[br] | bitboard->pieces[bq];
        if (rookLike && (rookLike & getRookAttacks(sq)))
            return true;

        // Bishop + queen share the bishop attack pattern - same skip.
        U64 bishopLike = bitboard->pieces[bb] | bitboard->pieces[bq];
        if (bishopLike && (bishopLike & getBishopAttacks(sq)))
            return true;
    }
    else
    {
        if (bitboard->pieces[wp] & bitboard->pawnAttacks[black][sq])
            return true;
        if (bitboard->pieces[wn] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[wk] & bitboard->kingAttacks[sq])
            return true;

        U64 rookLike = bitboard->pieces[wr] | bitboard->pieces[wq];
        if (rookLike && (rookLike & getRookAttacks(sq)))
            return true;

        U64 bishopLike = bitboard->pieces[wb] | bitboard->pieces[wq];
        if (bishopLike && (bishopLike & getBishopAttacks(sq)))
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
    if (move_str[0] < 'a' || move_str[0] > 'h') return 0;
    if (move_str[1] < '1' || move_str[1] > '8') return 0;
    if (move_str[2] < 'a' || move_str[2] > 'h') return 0;
    if (move_str[3] < '1' || move_str[3] > '8') return 0;

    int from = fileRank2Sq(move_str[0] - 'a', move_str[1] - '1');
    int to = fileRank2Sq(move_str[2] - 'a', move_str[3] - '1');
    int promotionPiece = empty;

    if (move_str.length() == 5)
    {
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
    for (const auto& [move, score] : moves)
    {
        if (moveFrom(move) == from &&
            moveTo(move) == to &&
            movePromotionPiece(move) == promotionPiece)
        {
            return move;
        }
    }
    return 0;
}

int bigPieceCount(int side)
{
    if (side == white)
        return board->pieceCount[wn] + board->pieceCount[wb] + board->pieceCount[wr] + board->pieceCount[wq];
    else
        return board->pieceCount[bn] + board->pieceCount[bb] + board->pieceCount[br] + board->pieceCount[bq];
}

bool isGameOver()
{
    std::vector<std::pair<int, int>> moves = generateMoves();
    // No move left to play
    if (moves.size() == 0) return true;

    // Check Legal move
    for (const auto& [move, score] : moves)
    {
        if (makeMove(move))
        {
            takeMove();
            return false;
        }
    }

    return true;
}

void printHelp()
{
    std::cout
        << "\n"
        << "Chanakya Chess Engine - Command Reference\n"
        << "=========================================\n"
        << "\n"
        << "UCI Protocol Commands:\n"
        << "  uci                       Identify engine, list options, print 'uciok'\n"
        << "  isready                   Sync command, engine replies 'readyok'\n"
        << "  setoption name <n> value <v>\n"
        << "                            Set an engine option, e.g.:\n"
        << "                              setoption name usebook value true\n"
        << "                              setoption name Hash value 128\n"
        << "\n"
        << "  ucinewgame                Reset engine state for a new game\n"
        << "  position startpos [moves <m1> <m2> ...]\n"
        << "                            Set position to the standard start, optionally\n"
        << "                            followed by moves in long algebraic form\n"
        << "\n"
        << "  position fen <fen> [moves <m1> <m2> ...]\n"
        << "                            Set position from a FEN string\n"
        << "\n"
        << "  go [depth <d>] [movetime <ms>] [wtime <ms>] [btime <ms>]\n"
        << "     [winc <ms>] [binc <ms>] [movestogo <n>] [perft <d>]\n"
        << "                            Start a search with the given time/depth controls\n"
        << "                            'go perft <d>' runs a perft test instead of a search\n"
        << "\n"
        << "  stop                      Stop the current search early\n"
        << "  quit                      Exit the engine\n"
        << "\n\n"
        << "Debug / Manual Testing Commands (non-UCI, not sent by GUIs):\n"
        << "  d                         Print the current board position\n"
        << "  move <move>               Make a move (e.g. move e2e4)\n"
        << "  undo                      Undo the last move made\n"
        << "  book                      Show opening book moves for the current position\n"
        << "  eval                      Print the static evaluation of the current position\n"
        << "  help                      Show this message\n"
        << std::endl;
}