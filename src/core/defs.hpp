// defs.hpp
#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <unordered_map>


extern std::string START_FEN;

extern const char *PIECE_ICON[];
extern const char PIECE_CHAR[];
extern const char PIECE_TYPE[];
extern const char FILE_CHAR[];
extern const int PIECE_COLOR[];
extern const char *SQUARE_CHAR[];
extern const char *PIECE_NAME[];
extern const int pieceValue[];
extern const int CASTLE_PERMISSION[];
extern const int MIRROR_64[];

extern const int ROOK_DIRECTIONS[];
extern const int KNIGHT_DIRECTIONS[];
extern const int BISHOP_DIRECTIONS[];
extern const int KING_DIRECTIONS[];
extern const int QUEEN_DIRECTIONS[];
extern const int Kings[];

extern const int ROOK_DF  [];
extern const int ROOK_DR  [];
extern const int BISHOP_DF[];
extern const int BISHOP_DR[];
extern const int KNIGHT_DF[];
extern const int KNIGHT_DR[];
extern const int KING_DF  [];
extern const int KING_DR  [];

extern const int MAX_DEPTH;
extern const int INFINITE;
extern const int AB_BOUND;
extern const int MATE;

extern const int slidingPieces[2][3];
extern const int nonSlidingPieces[2][2];

constexpr int VALUE_ZERO   = 0;
constexpr int PAWN_VALUE   = 208;
constexpr int KNIGHT_VALUE = 781;
constexpr int BISHOP_VALUE = 825;
constexpr int ROOK_VALUE   = 1276;
constexpr int QUEEN_VALUE  = 2538;
constexpr int KING_VALUE   = 50000;

enum
{
    PIECE_EMPTY,

    PIECE_WP, PIECE_WR, PIECE_WN, PIECE_WB, PIECE_WQ, PIECE_WK,
    PIECE_BP, PIECE_BR, PIECE_BN, PIECE_BB, PIECE_BQ, PIECE_BK
};
enum
{
    WHITE,
    BLACK,
    BOTH
};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

enum MobilityPiece { MOB_ROOK = 2, MOB_KNIGHT = 3, MOB_BISHOP = 4, MOB_QUEEN = 5 };


typedef unsigned long long U64;

enum
{
    CASTLE_WK = 8,
    CASTLE_WQ = 4,
    CASTLE_BK = 2,
    CASTLE_BQ = 1
};

enum
{
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1, 
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2, 
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3, 
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4, 
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5, 
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6, 
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7, 
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8, 
    SQ_NONE = -1
};

struct ScoredMove {
    int move;
    int score;
};

struct MoveList {
    ScoredMove moves[256];
    uint16_t  count = 0;
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
    int age;
};

struct polyEntry{
    // U64 key;
    uint16_t move;   // 2 bytes
    uint16_t weight; // 2 bytes    
    uint32_t learn;  // 4 bytes
};                   // 8 bytes
extern std::unordered_map<U64, std::vector<polyEntry>> openingBook;

// functions
inline int fileRank2Sq(int file, int rank) { return rank * 8 + file; }
inline int fileOf(int sq) { return sq & 7; }
inline int rankOf(int sq) { return sq >> 3; }
std::string moveStr(int move);
long long getCurrTime();