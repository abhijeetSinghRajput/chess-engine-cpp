#include "core/move.hpp"
#include "core/utils.hpp"
#include "core/bitboard.hpp"

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

    printf("from : %s\n", SQUARE_CHAR[moveFrom(move)]);
    printf("to : %s\n", SQUARE_CHAR[moveTo(move)]);
    printf("capture : %c\n", PIECE_CHAR[moveCapturePiece(move)]);
    printf("promotion : %c\n", PIECE_CHAR[movePromotionPiece(move)]);

    if (move & EN_PASSANT_FLAG)
        printf("flag : enPassant\n");
    if (move & PAWN_START_FLAG)
        printf("flag : pawn start\n");
    if (move & CASTLE_FLAG)
        printf("flag : castle\n");
    printf("\n");
}

void movePiece(int from, int to)
{
    int piece = board->pieces[from];

    hashPiece(from, piece);
    board->pieces[from] = PIECE_EMPTY;

    hashPiece(to, piece);
    board->pieces[to] = piece;

    bitboard->movePiece(piece, sq120To64[from], sq120To64[to]);
}

void addPiece(int sq, int piece)
{
    if (board->pieces[sq] != PIECE_EMPTY)
    {
        // error
    }
    hashPiece(sq, piece);
    board->pieces[sq] = piece;
    board->material[PIECE_COLOR[piece]] += pieceValue[piece];
    board->pieceCount[piece]++;

    bitboard->setBit(piece, sq120To64[sq]);
}

void removePiece(int sq)
{
    int piece = board->pieces[sq];
    hashPiece(sq, piece);
    board->pieces[sq] = PIECE_EMPTY;
    board->material[PIECE_COLOR[piece]] -= pieceValue[piece];
    board->pieceCount[piece]--;

    bitboard->clearBit(piece, sq120To64[sq]);
}


int takeMove() {
    if (board->ply == 0) {
        return 0;
    }

    MoveInfo *moveInfo = board->popMoveFromHistory(); 
    if (board->enPassantSq != SQ_NONE) {
        hashEnPassant();
    }
    hashCastle();

    board->fiftyMove = moveInfo->fiftyMove;
    board->enPassantSq = moveInfo->enPassantSq;
    board->castlePermission = moveInfo->castlePermission;
    board->checkSq = moveInfo->checkSq;

    if (board->enPassantSq != SQ_NONE) {
        hashEnPassant();
    }
    hashCastle();
    board->side ^= 1;
    hashSide();

    const int move = moveInfo->move;
    const int from = moveFrom(move);
    const int to = moveTo(move);

    if (move & EN_PASSANT_FLAG) {
        if (board->side == WHITE) {
            addPiece(to - 10, PIECE_BP);
        }
        else {
            addPiece(to + 10, PIECE_WP);
        }
    }
    else if (move & CASTLE_FLAG) {
        switch (to) {
            case SQ_G1: movePiece(SQ_F1, SQ_H1); break;
            case SQ_C1: movePiece(SQ_D1, SQ_A1); break;

            case SQ_G8: movePiece(SQ_F8, SQ_H8); break;
            case SQ_C8: movePiece(SQ_D8, SQ_A8); break;

            default: break;
        }
    }


    movePiece(to, from);
    if (move & CAPTURE_FLAG) {
        addPiece(to, moveCapturePiece(move));
    }
    if (move & PROMOTION_FLAG) {
        removePiece(from);
        addPiece(from, PIECE_COLOR[movePromotionPiece(move)] == WHITE ? PIECE_WP : PIECE_BP);
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

    board->pushMoveToHistory(move);

    if (move & EN_PASSANT_FLAG)
    {
        if (side == WHITE)
        {
            removePiece(to - 10);
        }
        else
        {
            removePiece(to + 10);
        }
    }
    else if (move & CASTLE_FLAG)
    {
        switch (to)
        {
            case SQ_G1: movePiece(SQ_H1, SQ_F1); break;
            case SQ_C1: movePiece(SQ_A1, SQ_D1); break;

            case SQ_G8: movePiece(SQ_H8, SQ_F8); break;
            case SQ_C8: movePiece(SQ_A8, SQ_D8); break;

            default: break;
        }
    }
    // hash out
    if (board->enPassantSq != SQ_NONE)
    {
        hashEnPassant();
    }
    hashCastle();

    board->castlePermission &= CASTLE_PERMISSION[from];
    board->castlePermission &= CASTLE_PERMISSION[to];
    board->enPassantSq = SQ_NONE;

    // hash in
    hashCastle();

    board->fiftyMove++;

    if (piece == PIECE_WP || piece == PIECE_BP)
    {
        board->fiftyMove = 0;
        if (move & PAWN_START_FLAG)
        {
            if (side == WHITE)
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

    if (move & CAPTURE_FLAG)
    {
        board->fiftyMove = 0;
        removePiece(to);
    }
    movePiece(from, to);

    if (move & PROMOTION_FLAG)
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
        board->checkSq = sq64To120[enemyKingOnSq];
    }
    else
    {
        board->checkSq = SQ_NONE;
    }
    return true;
}


void makeNullMove() {
    if (board->checkSq != SQ_NONE)  return;

    board->pushMoveToHistory(0);

    if (board->enPassantSq != SQ_NONE) {
        hashEnPassant();
    }

    board->enPassantSq = SQ_NONE;
    board->side ^= 1;
    hashSide();
}

int takeNullMove(){
    if (board->ply == 0) {
        return 0;
    }

    MoveInfo *moveInfo = board->popMoveFromHistory(); 


    if(board->enPassantSq != SQ_NONE) {
        hashEnPassant();
    }

    board->castlePermission = moveInfo->castlePermission;
    board->fiftyMove = moveInfo->fiftyMove;
    board->enPassantSq = moveInfo->enPassantSq;
    board->checkSq = moveInfo->checkSq;

    if(board->enPassantSq != SQ_NONE) {
        hashEnPassant();
    }
    board->side ^= 1;
    hashSide();

    return 0;
}

