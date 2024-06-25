#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <unordered_map>


extern std::string startFen;
extern int sq120To64[120];
extern int sq64To120[64];

extern const char pieceChar[];
extern const char pieceType[];
extern const char fileChar[];
extern const int pieceColor[];
extern const char *squareChar[];
extern const char *pieceName[];
extern const int pieceValue[];
extern const int CastlePermission[];
extern const int Mirror64[];

extern const int rookDirections[];
extern const int knightDirections[];
extern const int bishopDirections[];
extern const int kingDirections[];
extern const int queenDirections[];
extern const int Kings[];

extern const int maxDepth;
extern const int Infinite;
extern const int Mate;

extern const int slidingPieces[2][3];
extern const int nonSlidingPieces[2][2];

enum
{
    empty,
    wp,wr,wn,wb,wq,wk,
    bp,br,bn,bb,bq,bk
};
enum
{
    white,
    black,
    both
};
enum {fileA, fileB, fileC, fileD, fileE, fileF, fileG, fileH};
enum {rank1, rank2, rank3, rank4, rank5, rank6, rank7, rank8};

typedef unsigned long long U64;

enum
{
    castle_K = 8,
    castle_Q = 4,
    castle_k = 2,
    castle_q = 1
};

enum
{
    a1 = 21, b1, c1, d1, e1, f1, g1, h1, 
    a2 = 31, b2, c2, d2, e2, f2, g2, h2, 
    a3 = 41, b3, c3, d3, e3, f3, g3, h3, 
    a4 = 51, b4, c4, d4, e4, f4, g4, h4, 
    a5 = 61, b5, c5, d5, e5, f5, g5, h5, 
    a6 = 71, b6, c6, d6, e6, f6, g6, h6, 
    a7 = 81, b7, c7, d7, e7, f7, g7, h7, 
    a8 = 91, b8, c8, d8, e8, f8, g8, h8, 
    noSq = -1, offBoard = -2
};

struct MoveInfo
{
    int fiftyMove;
    U64 positionKey;
    int enPassantSq;
    int castlePermission;
    int checkSq;
    int move;
};

struct TableData
{
    U64 smp_data;
    U64 smp_key;
};

struct polyEntry{
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};
extern std::unordered_map<U64, std::vector<polyEntry>> openingBook;

// functions

void initialize();

inline int fileRank2Sq(int file, int rank) { return (rank * 10 + file) + 21; }
inline int fileOf(int sq) { return (sq % 10) - 1; }
inline int rankOf(int sq) { return (sq - 21) / 10; }
std::string moveStr(int move);
long long getCurrTime();