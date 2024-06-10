#pragma once

#include "defs.hpp"
#include "board.hpp"
#include <vector>

extern U64 bishopMagics[64];
extern U64 rookMagics[64];

U64 randomU64();
U64 random_64_fewbits();

class Bitboard
{
private:
    Board *board;
    void initMasks();
    void init_attackMasks();
    void init_pawnAttacks();
    void init_kingAttacks();
    void init_knighAttack();
    void init_rookAttacks();
    void init_bishopAttacks();
    void init_fileMasks();
    void init_rankMasks();
    void init_isolatedPawnMask();
    void init_passedPawnMask();
    void init_pieces();
    void generateBlockerBoards(U64 bitBoard, std::vector<U64> &blockerBoards);
    void generateBlockersFor(U64 (&attackMasks)[64], std::vector<U64> (&storage)[64]);
    void init_rookLookupTable();
    void init_bishopLookupTable();
    U64 legalMoveBitboardFromBlockers(int sq, U64 blockerBitboard, bool ortho);

public:
    //static variables
    static U64 rookLookupTable[64][4096];
    static U64 bishopLookupTable[64][4096];
    static U64 fileMasks[8];
    static U64 rankMasks[8];
    static U64 isolatedPawnMask[64];
    static U64 passedPawnMask[2][64];
    static std::vector<U64> rookBlockers[64];
    static std::vector<U64> bishopBlockers[64];

    //non static variables
    U64 pieces[13];
    U64 pawnAttacks[2][64] = {0ULL};
    U64 kingAttacks[64] = {0ULL};
    U64 knightAttacks[64] = {0ULL};
    U64 rookAttacks[64] = {0ULL};
    U64 bishopAttacks[64] = {0ULL};

    Bitboard();
    void initBoard(Board *board);
    void clearBit(int piece, int sq);
    void setBit(U64 &bitBoard, int sq);
    void setBit(int piece, int sq);
    int getPieceCount(int piece);
    void movePiece(int piece, int fromSq, int toSq);
    void print(int piece);
    void print(U64 bitBoard);
    U64 findMagic(int sq, const std::vector<U64> &blockerBitboards, bool ortho);
    U64 getPieces(int side);
};

extern Bitboard *bitboard;