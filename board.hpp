#pragma once

#include <string>
#include "defs.hpp"

class Board
{
    U64 generatePositionKey();
    void updateMaterial();

public:
    static int pieces[120];
    int side = white;
    int castlePermission = 0;
    int fiftyMove = 0;
    int enPassantSq = noSq;
    int checkSq = noSq;
    int ply = 0;

    int pieceCount[13];
    int material[2];
    U64 positionKey = 0;
    MoveInfo history[1024];

    Board();
    void pushMoveToHistory(int move);
    MoveInfo *popMoveFromHistory();

    void parseFen(std::string &fen);
    std::string getFen();
    void print();
    void reset();
};

extern Board* board;
