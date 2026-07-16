#include "core/utils.hpp"
#include "core/board.hpp"
#include "core/bitboard.hpp"
#include "core/zobristKeys.hpp"
#include "search/transpositionTable.hpp"
#include <iostream>
#include <iomanip>

Board *board = new Board;
int Board::pieces[120];

Board::Board()
{
    reset();
}

void Board::pushMoveToHistory(int move)
{
    history[ply].fiftyMove = this->fiftyMove;
    history[ply].fiftyMove = this->fiftyMove;
    history[ply].positionKey = this->positionKey;
    history[ply].enPassantSq = this->enPassantSq;
    history[ply].castlePermission = this->castlePermission;
    history[ply].checkSq = this->checkSq;
    history[ply].move = move;
    ply++;
}
MoveInfo *Board::popMoveFromHistory()
{
    if (ply > 0)
    {
        MoveInfo *moveInfo = &history[--ply];
        return moveInfo;
    }
    else
    {
        std::cerr << "No moves to pop!" << std::endl;
        return nullptr;
    }
}

U64 Board::generatePositionKey()
{
    U64 hashKey = 0;
    for (auto sq : sq64To120)
    {
        int piece = pieces[sq];
        if (piece != PIECE_EMPTY)
        {
            hashKey ^= pieceKeys[piece][sq];
        }
    }

    hashKey ^= castleKeys[castlePermission];

    if (side == WHITE)
        hashKey ^= sideKey;
    if (enPassantSq != SQ_NONE)
    {
        hashKey ^= pieceKeys[PIECE_EMPTY][enPassantSq];
    }
    return hashKey;
}

void Board::updateMaterial()
{
    for (int sq : sq64To120)
    {
        int piece = pieces[sq];
        if (piece != PIECE_EMPTY)
        {
            pieceCount[piece]++;
            material[PIECE_COLOR[piece]] += pieceValue[piece];
        }
    }
}

void Board::reset()
{
    if (transpositionTable) transpositionTable->clear();

    for (int i = 0; i < 1024; ++i)
    {
        history[i].fiftyMove = 0;
        history[i].positionKey = 0ULL;
        history[i].enPassantSq = SQ_NONE;
        history[i].castlePermission = 0;
        history[i].checkSq = SQ_NONE;
        history[i].move = 0;
    }
    for (int i = 0; i < 120; ++i)
    {
        pieces[i] = offBoard;
    }
    for (auto sq : sq64To120)
    {
        pieces[sq] = PIECE_EMPTY;
    }
    for (int i = 0; i < 13; ++i)
    {
        pieceCount[i] = 0;
    }
    material[WHITE] = 0;
    material[BLACK] = 0;
    side = WHITE;
    castlePermission = 0;
    fiftyMove = 0;
    checkSq = SQ_NONE;
    positionKey = 0;
    ply = 0;

}
void Board::print()
{
    int sq, piece;
    std::cout << std::endl;
    for (int rank = rank8; rank >= RANK_1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            sq = fileRank2Sq(file, rank);
            piece = pieces[sq];
            if (PIECE_COLOR[piece] == BLACK)
            {
                printf("\033[2m%c  \033[0m", PIECE_CHAR[piece]);
            }
            else
            {
                printf("%c  ", PIECE_CHAR[piece]);
            }
        }
        std::cout << std::endl;
    }
    std::cout << "\n    ";
    for (int file = FILE_A; file <= FILE_H; ++file)
    {
        std::cout << FILE_CHAR[file] << "  ";
    }
    std::cout << std::endl
              << std::endl;
    std::cout << "side : " << side << std::endl;
    std::cout << "Key: 0x" << std::hex << positionKey << std::dec << std::endl;
    std::cout << "Fen: " << getFen() << std::endl;
    std::cout << "Castle: "
              << ((castlePermission & CASTLE_WK) ? "K" : "_")
              << ((castlePermission & CASTLE_WQ) ? "Q" : "_")
              << ((castlePermission & CASTLE_BK) ? "k" : "_")
              << ((castlePermission & CASTLE_BQ) ? "q" : "_")
              << std::endl;

    if (board->checkSq != SQ_NONE)
    {
        printf("\033[31mcheck: %s\033[0m\n", SQUARE_CHAR[board->checkSq]);
    }
    if (board->enPassantSq != SQ_NONE)
    {
        printf("\033[33mEP: %s\033[0m\n", SQUARE_CHAR[board->enPassantSq]);
    }
}

void Board::parseFen(std::string &fen)
{
    reset();

    int i = 0;
    int file = FILE_A;
    int rank = rank8;
    int sq;
    while (fen[i] == ' ') i++; // skip white space

    while (fen[i] != ' ')
    {
        sq = fileRank2Sq(file, rank);
        if (fen[i] == '/')
        {
            --rank;
            file = FILE_A;
        }
        else if (isalpha(fen[i]))
        {
            switch (fen[i])
            {
                case 'P': pieces[sq] = PIECE_WP; break;
                case 'N': pieces[sq] = PIECE_WN; break;
                case 'B': pieces[sq] = PIECE_WB; break;
                case 'R': pieces[sq] = PIECE_WR; break;
                case 'Q': pieces[sq] = PIECE_WQ; break;
                case 'K': pieces[sq] = PIECE_WK; break;

                case 'p': pieces[sq] = PIECE_BP; break;
                case 'n': pieces[sq] = PIECE_BN; break;
                case 'b': pieces[sq] = PIECE_BB; break;
                case 'r': pieces[sq] = PIECE_BR; break;
                case 'q': pieces[sq] = PIECE_BQ; break;
                case 'k': pieces[sq] = PIECE_BK; break;

                default: return;
            }
            file++;
        }
        else
        {
            file += fen[i] - '0';
        }
        ++i;
    }
    while (fen[++i] == ' '); // skip white space
    side = (fen[i] == 'w') ? WHITE : BLACK;
    while (fen[++i] == ' '); // skip white space
    while (fen[i] != ' ')
    {
        switch (fen[i++])
        {
            case 'K': castlePermission |= CASTLE_WK; break;
            case 'Q': castlePermission |= CASTLE_WQ; break;
            case 'k': castlePermission |= CASTLE_BK; break;
            case 'q': castlePermission |= CASTLE_BQ; break;
            default: break;
        }
    }

    while (fen[++i] == ' ')
        ; // skip white space
    if (fen[i] != '-')
    {
        int file = fen[i++] - 'a';
        int rank = fen[i] - '1';
        enPassantSq = fileRank2Sq(file, rank);
    }
    else
    {
        enPassantSq = SQ_NONE;
    }
    updateMaterial();
    bitboard->initBoard(this);

    // is in check
    int kingOnSq = __builtin_ctzll(bitboard->pieces[Kings[side]]);
    board->checkSq = isUnderAttack(kingOnSq, board->side ^ 1) ? sq64To120[kingOnSq] : SQ_NONE;

    // generate a uniqe position key
    positionKey = generatePositionKey();
}

std::string Board::getFen()
{
    std::string fen;
    int emptySq = 0, sq;
    int piece;

    for (int rank = rank8; rank >= RANK_1; --rank)
    {
        emptySq = 0;
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            sq = fileRank2Sq(file, rank);
            piece = pieces[sq];
            if (piece == PIECE_EMPTY)
            {
                emptySq++;
            }
            else
            {
                if (emptySq)
                {
                    fen += std::to_string(emptySq);
                    emptySq = 0;
                }
                fen += PIECE_CHAR[piece];
            }
        }
        if (emptySq)
        {
            fen += std::to_string(emptySq);
        }
        if (rank != RANK_1)
        {
            fen += '/';
        }
    }

    fen += ' ';
    fen += (side == WHITE) ? 'w' : 'b';
    fen += ' ';

    if (castlePermission)
    {
        if (castlePermission & CASTLE_WK) fen += 'K';
        if (castlePermission & CASTLE_WQ) fen += 'Q';
        if (castlePermission & CASTLE_BK) fen += 'k';
        if (castlePermission & CASTLE_BQ) fen += 'q';
    }
    else
    {
        fen += '-';
    }

    fen += ' ';
    if (enPassantSq != SQ_NONE)
    {
        fen += SQUARE_CHAR[enPassantSq];
    }
    else
    {
        fen += '-';
    }

    fen += ' ';
    fen += std::to_string(fiftyMove);
    fen += ' ';
    fen += std::to_string((ply / 2) + 1);

    return fen;
}
