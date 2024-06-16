#include "utils.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "zobristKeys.hpp"
#include "transpositionTable.hpp"
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
        if (piece != empty)
        {
            hashKey ^= pieceKeys[piece][sq];
        }
    }

    hashKey ^= castleKeys[castlePermission];

    if (side == white)
        hashKey ^= sideKey;
    if (enPassantSq != noSq)
    {
        hashKey ^= pieceKeys[empty][enPassantSq];
    }
    return hashKey;
}

void Board::updateMaterial()
{
    for (int sq : sq64To120)
    {
        int piece = pieces[sq];
        if (piece != empty)
        {
            pieceCount[piece]++;
            material[pieceColor[piece]] += pieceValue[piece];
        }
    }
}

void Board::reset()
{
    transpositionTable->clear();
    
    for (int i = 0; i < 1024; ++i)
    {
        history[i].fiftyMove = 0;
        history[i].positionKey = 0ULL;
        history[i].enPassantSq = noSq;
        history[i].castlePermission = 0;
        history[i].checkSq = noSq;
        history[i].move = 0;
    }
    for (int i = 0; i < 120; ++i)
    {
        pieces[i] = offBoard;
    }
    for (auto sq : sq64To120)
    {
        pieces[sq] = empty;
    }
    for (int i = 0; i < 13; ++i)
    {
        pieceCount[i] = 0;
    }
    material[white] = 0;
    material[black] = 0;
    side = white;
    castlePermission = 0;
    fiftyMove = 0;
    checkSq = noSq;
    positionKey = 0;
    ply = 0;

}
void Board::print()
{
    int sq, piece;
    std::cout << std::endl;
    for (int rank = rank8; rank >= rank1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = fileA; file <= fileH; ++file)
        {
            sq = fileRank2Sq(file, rank);
            piece = pieces[sq];
            if (pieceColor[piece] == black)
            {
                printf("\033[2m%c  \033[0m", pieceChar[piece]);
            }
            else
            {
                printf("%c  ", pieceChar[piece]);
            }
        }
        std::cout << std::endl;
    }
    std::cout << "\n    ";
    for (int file = fileA; file <= fileH; ++file)
    {
        std::cout << fileChar[file] << "  ";
    }
    std::cout << std::endl
              << std::endl;
    std::cout << "side : " << side << std::endl;
    std::cout << "Key: 0x" << std::hex << positionKey << std::dec << std::endl;
    std::cout << "Fen: " << getFen() << std::endl;
    std::cout << "Castle: " << castlePermission << std::endl;
    if (board->checkSq != noSq)
    {
        printf("\033[31mcheck : %s\033[0m\n", squareChar[board->checkSq]);
    }
}

void Board::parseFen(std::string &fen)
{
    reset();

    int i = 0;
    int file = fileA;
    int rank = rank8;
    int sq;
    while (fen[i] == ' ') i++; // skip white space

    while (fen[i] != ' ')
    {
        sq = fileRank2Sq(file, rank);
        if (fen[i] == '/')
        {
            --rank;
            file = fileA;
        }
        else if (isalpha(fen[i]))
        {
            switch (fen[i])
            {
                case 'P': pieces[sq] = wp; break;
                case 'N': pieces[sq] = wn; break;
                case 'B': pieces[sq] = wb; break;
                case 'R': pieces[sq] = wr; break;
                case 'Q': pieces[sq] = wq; break;
                case 'K': pieces[sq] = wk; break;

                case 'p': pieces[sq] = bp; break;
                case 'n': pieces[sq] = bn; break;
                case 'b': pieces[sq] = bb; break;
                case 'r': pieces[sq] = br; break;
                case 'q': pieces[sq] = bq; break;
                case 'k': pieces[sq] = bk; break;

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
    side = (fen[i] == 'w') ? white : black;
    while (fen[++i] == ' '); // skip white space
    while (fen[i] != ' ')
    {
        switch (fen[i++])
        {
            case 'K': castlePermission |= castle_K; break;
            case 'Q': castlePermission |= castle_Q; break;
            case 'k': castlePermission |= castle_k; break;
            case 'q': castlePermission |= castle_q; break;
            default: break;
        }
    }

    while (fen[++i] == ' '); // skip white space
    if (fen[i] != '-')
    {
        int file = fen[i++] - 'a';
        int rank = fen[i] - '0';
        enPassantSq = fileRank2Sq(file, rank);
    }
    updateMaterial();
    bitboard->initBoard(this);

    // is in check
    int kingOnSq = __builtin_ctzll(bitboard->pieces[Kings[side]]);
    if (isUnderAttack(kingOnSq, board->side ^ 1))
    {
        board->checkSq = sq64To120[kingOnSq];
    }

    // generate a uniqe position key
    positionKey = generatePositionKey();
}

std::string Board::getFen()
{
    std::string fen;
    int emptySq = 0, sq, piece;

    for (int rank = rank8; rank >= rank1; --rank)
    {
        emptySq = 0;
        for (int file = fileA; file <= fileH; ++file)
        {
            sq = fileRank2Sq(file, rank);
            piece = pieces[sq];
            if (piece == empty)
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
                fen += pieceChar[piece];
            }
        }
        if (emptySq)
        {
            fen += std::to_string(emptySq);
        }
        if (rank != rank1)
        {
            fen += '/';
        }
    }

    fen += ' ';
    fen += (side == white) ? 'w' : 'b';
    fen += ' ';
    if (castlePermission)
    {
        if (castlePermission & castle_K)
            fen += 'K';
        if (castlePermission & castle_Q)
            fen += 'Q';
        if (castlePermission & castle_k)
            fen += 'k';
        if (castlePermission & castle_q)
            fen += 'q';
    }
    else
    {
        fen += '-';
    }

    if (enPassantSq != noSq)
    {
        fen += squareChar[enPassantSq];
    }
    else
    {
        fen += " - ";
    }
    fen += std::to_string(fiftyMove) + ' ';
    fen += std::to_string(ply + 1);
    return fen;
}
