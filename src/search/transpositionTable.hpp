#pragma once

#include "core/defs.hpp"

extern const int AlphaFlag;
extern const int BetaFlag;
extern const int ExactFlag;

class TranspositionTable
{
private:
    TableData* entries = nullptr;
    size_t sizeMask = 0; // numEntries - 1 (numEntries is power of 2)

public:
    TranspositionTable();
    ~TranspositionTable();
    void resize(size_t mbSize);
    void clear();
    void add(U64 positionKey, int move, int score, int flag, int depth);
    TableData *get(U64 positionKey);
    std::vector<int> getLine(int depth);
    int getMove();
};


inline U64 pack_tableData(int score, int depth, int flag, U64 move)
{
    return (score + 32000) | (depth << 16) | (flag << 22) | (move << 24);
}

inline int extract_score(U64 data) { return (data & 0xffff) - 32000; }
inline int extract_move(U64 data) { return data >> 24; }
inline int extract_flag(U64 data) { return (data >> 22) & 3; }
inline int extract_depth(U64 data) { return (data >> 16) & 0x3f; }

extern TranspositionTable* transpositionTable;
