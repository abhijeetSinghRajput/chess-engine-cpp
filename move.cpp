#include "move.hpp"
#include "utils.hpp"
#include "bitboard.hpp"

int enPassantFlag = 0x400000;
int castleFlag = 0x800000;
int pawnStartFlag = 0x1000000;

int captureFlag = 0x3c000;
int promotionFlag = 0x3c0000;

int buildMove(int from, int to, int capturedPiece, int promotedPiece, int flag)
{
    return from | (to << 7) | (capturedPiece << 14) | (promotedPiece << 18) | flag;
}

int moveFrom(int move)
{
    return (move & 0x7f);
}

int moveTo(int move)
{
    return (move >> 7) & 0x7f;
}

int moveCapturePiece(int move)
{
    return (move >> 14) & 0xf;
}
int movePromotionPiece(int move)
{
    return (move >> 18) & 0xf;
}

void moveDetail(int move)
{
    if (!move)
        return;

    printf("from : %s\n", squareChar[moveFrom(move)]);
    printf("to : %s\n", squareChar[moveTo(move)]);
    printf("capture : %c\n", pieceChar[moveCapturePiece(move)]);
    printf("promotion : %c\n", pieceChar[movePromotionPiece(move)]);

    if (move & enPassantFlag)
        printf("flag : enPassant\n");
    if (move & pawnStartFlag)
        printf("flag : pawn start\n");
    if (move & castleFlag)
        printf("flag : castle\n");
    printf("\n");
}

void movePiece(int from, int to)
{
    int piece = board->pieces[from];

    hashPiece(from, piece);
    board->pieces[from] = empty;

    hashPiece(to, piece);
    board->pieces[to] = piece;

    bitboard->movePiece(piece, sq120To64[from], sq120To64[to]);
}

void addPiece(int sq, int piece)
{
    if (board->pieces[sq] != empty)
    {
        // error
    }
    hashPiece(sq, piece);
    board->pieces[sq] = piece;
    board->material[pieceColor[piece]] += pieceValue[piece];
    board->pieceCount[piece]++;

    bitboard->setBit(piece, sq120To64[sq]);
}

void removePiece(int sq)
{
    int piece = board->pieces[sq];
    hashPiece(sq, piece);
    board->pieces[sq] = empty;
    board->material[pieceColor[piece]] -= pieceValue[piece];
    board->pieceCount[piece]--;

    bitboard->clearBit(piece, sq120To64[sq]);
}


int takeMove() {
    if (board->ply == 0) {
        return 0;
    }

    MoveInfo *moveInfo = board->popMoveFromHistory(); 
    if (board->enPassantSq != noSq) {
        hashEnPassant();
    }
    hashCastle();

    board->fiftyMove = moveInfo->fiftyMove;
    board->enPassantSq = moveInfo->enPassantSq;
    board->castlePermission = moveInfo->castlePermission;
    board->checkSq = moveInfo->checkSq;

    if (board->enPassantSq != noSq) {
        hashEnPassant();
    }
    hashCastle();
    board->side ^= 1;
    hashSide();

    const int move = moveInfo->move;
    const int from = moveFrom(move);
    const int to = moveTo(move);

    if (move & enPassantFlag) {
        if (board->side == white) {
            addPiece(to - 10, bp);
        }
        else {
            addPiece(to + 10, wp);
        }
    }
    else if (move & castleFlag) {
        switch (to) {
            case g1: movePiece(f1, h1); break;
            case c1: movePiece(d1, a1); break;

            case g8: movePiece(f8, h8); break;
            case c8: movePiece(d8, a8); break;

            default: break;
        }
    }


    movePiece(to, from);
    if (move & captureFlag) {
        addPiece(to, moveCapturePiece(move));
    }
    if (move & promotionFlag) {
        removePiece(from);
        addPiece(from, pieceColor[movePromotionPiece(move)] == white ? wp : bp);
    }

    return move;
}



bool makeMove(int move)
{
    if (!move)
    {
        // error
        return false;
    }
    const int from = moveFrom(move);
    const int to = moveTo(move);
    const int side = board->side;
    const int piece = board->pieces[from];

    struct MoveInfo
    {
        int fiftyMove;
        U64 positionKey;
        int enPassantSq;
        int castlePermission;
        int checkSq;
        int move;
    };

    board->pushMoveToHistory(move);

    if (move & enPassantFlag)
    {
        if (side == white)
        {
            removePiece(to - 10);
        }
        else
        {
            removePiece(to + 10);
        }
    }
    else if (move & castleFlag)
    {
        switch (to)
        {
            case g1: movePiece(h1, f1); break;
            case c1: movePiece(a1, d1); break;

            case g8: movePiece(h8, f8); break;
            case c8: movePiece(a8, d8); break;

            default: break;
        }
    }
    // hash out
    if (board->enPassantSq != noSq)
    {
        hashEnPassant();
    }
    hashCastle();

    board->castlePermission &= CastlePermission[from];
    board->castlePermission &= CastlePermission[to];
    board->enPassantSq = noSq;

    // hash in
    hashCastle();

    board->fiftyMove++;

    if (piece == wp || piece == bp)
    {
        board->fiftyMove = 0;
        if (move & pawnStartFlag)
        {
            if (side == white)
            {
                board->enPassantSq = from + 10;
            }
            else
            {
                board->enPassantSq = from - 10;
            }
            hashEnPassant();
        }
    }

    if (move & captureFlag)
    {
        board->fiftyMove = 0;
        removePiece(to);
    }
    movePiece(from, to);

    if (move & promotionFlag)
    {
        removePiece(to);
        addPiece(to, movePromotionPiece(move));
    }

    board->side ^= 1;
    hashSide();

    int kingOnSq = __builtin_ctzll(bitboard->pieces[Kings[side]]);
    int enemyKingOnSq = __builtin_ctzll(bitboard->pieces[Kings[board->side]]);

    if (isUnderAttack(kingOnSq, board->side))
    {
        takeMove();
        return false;
    }

    if (isUnderAttack(enemyKingOnSq, side))
    {
        board->checkSq = enemyKingOnSq;
    }
    else
    {
        board->checkSq = noSq;
    }
    return true;
}


void makeNullMove() {
    if(board->checkSq != noSq) return;

    board->pushMoveToHistory(0);
    board->enPassantSq = noSq;
    board->side ^= 1;
    hashSide();
}

int takeNullMove(){
    if (board->ply == 0) {
        return 0;
    }

    MoveInfo *moveInfo = board->popMoveFromHistory(); 


    if(board->enPassantSq != noSq) {
        hashEnPassant();
    }

    board->castlePermission = moveInfo->castlePermission;
    board->fiftyMove = moveInfo->fiftyMove;
    board->enPassantSq = moveInfo->enPassantSq;
    board->checkSq = moveInfo->checkSq;

    if(board->enPassantSq != noSq) {
        hashEnPassant();
    }
    board->side ^= 1;
    hashSide();

    return 0;
}

