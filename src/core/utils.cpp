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
    // getPieces(BOTH) is now an O(1) load off Bitboard::occupied[] (see
    // bitboard.hpp/cpp) instead of OR-ing 12 piece bitboards together
    // on every call - this function runs on every sliding move
    // generated and every isUnderAttack() check.
    U64 allPieces = bitboard->getPieces(BOTH);
    U64 blocker = allPieces & bitboard->rookAttacks[sq];

    int index = (blocker * rookMagics[sq]) >> (64 - 12);
    return bitboard->rookLookupTable[sq][index];
}

U64 getBishopAttacks(int sq)
{
    U64 allPieces = bitboard->getPieces(BOTH);
    U64 blocker = allPieces & bitboard->bishopAttacks[sq];

    int index = (blocker * bishopMagics[sq]) >> (64 - 12);
    return bitboard->bishopLookupTable[sq][index];
}

bool isUnderAttack(int sq, int attackingSide)
{
    if (attackingSide == BLACK)
    {
        // Pawn, knight, and king attacks - cheap array lookups, checked first
        if (bitboard->pieces[PIECE_BP] & bitboard->pawnAttacks[WHITE][sq])
            return true;
        if (bitboard->pieces[PIECE_BN] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[PIECE_BK] & bitboard->kingAttacks[sq])
            return true;

        // Rook + queen share the rook attack pattern. Skip the magic
        // lookup entirely (multiply, shift, 32KB table probe) when
        // black has neither piece left on the board - common in
        // endgames and free when it doesn't apply.
        U64 rookLike = bitboard->pieces[PIECE_BR] | bitboard->pieces[PIECE_BQ];
        if (rookLike && (rookLike & getRookAttacks(sq)))
            return true;

        // Bishop + queen share the bishop attack pattern - same skip.
        U64 bishopLike = bitboard->pieces[PIECE_BB] | bitboard->pieces[PIECE_BQ];
        if (bishopLike && (bishopLike & getBishopAttacks(sq)))
            return true;
    }
    else
    {
        if (bitboard->pieces[PIECE_WP] & bitboard->pawnAttacks[BLACK][sq])
            return true;
        if (bitboard->pieces[PIECE_WN] & bitboard->knightAttacks[sq])
            return true;
        if (bitboard->pieces[PIECE_WK] & bitboard->kingAttacks[sq])
            return true;

        U64 rookLike = bitboard->pieces[PIECE_WR] | bitboard->pieces[PIECE_WQ];
        if (rookLike && (rookLike & getRookAttacks(sq)))
            return true;

        U64 bishopLike = bitboard->pieces[PIECE_WB] | bitboard->pieces[PIECE_WQ];
        if (bishopLike && (bishopLike & getBishopAttacks(sq)))
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

    if (move_str.length() == 5)
    {
        switch (move_str[4])
        {
            case 'q': promotionPiece = (move_str[3] == '8')? PIECE_WQ : PIECE_BQ; break;
            case 'r': promotionPiece = (move_str[3] == '8')? PIECE_WR : PIECE_BR; break;
            case 'b': promotionPiece = (move_str[3] == '8')? PIECE_WB : PIECE_BB; break;
            case 'n': promotionPiece = (move_str[3] == '8')? PIECE_WN : PIECE_BN; break;

            default: break;
        }
    }

    MoveList moves;
    generateMoves(moves);
    for (int i = 0; i < moves.count; ++i)
    {
        move = moves.moves[i].move;

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
    if (side == WHITE)
        return board->pieceCount[PIECE_WN] + board->pieceCount[PIECE_WB] + board->pieceCount[PIECE_WR] + board->pieceCount[PIECE_WQ];
    else
        return board->pieceCount[PIECE_BN] + board->pieceCount[PIECE_BB] + board->pieceCount[PIECE_BR] + board->pieceCount[PIECE_BQ];
}

bool isGameOver()
{
    MoveList moves;
    generateMoves(moves);
    // No move left to play
    if (moves.count == 0) return true;

    // Check Legal move
    for (int i = 0; i < moves.count; ++i)
    {
        int move = moves.moves[i].move;
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