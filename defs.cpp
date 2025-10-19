#include "defs.hpp"
#include "move.hpp"
#include <chrono>

// Initialize global variables
std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
int sq120To64[120];
int sq64To120[64];

const char pieceChar[] = ".PRNBQKprnbqk";
const char pieceType[] = ".prnbqkprnbqk";
const char fileChar[] = "abcdefgh";
const int pieceColor[] = {
    2,
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1,
};
const char *squareChar[] = {
    ".", ".", ".", ".", ".", ".", ".", ".", ".", ".",
    ".", ".", ".", ".", ".", ".", ".", ".", ".", ".",
    ".", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", ".",
    ".", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", ".",
    ".", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", ".",
    ".", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", ".",
    ".", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", ".",
    ".", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", ".",
    ".", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", ".",
    ".", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", ".",
    ".", ".", ".", ".", ".", ".", ".", ".", ".", ".",
    ".", ".", ".", ".", ".", ".", ".", ".", ".", ".",
};
const char *pieceName[] = {
    ".",
    "wp", "wr", "wn", "wb", "wq", "wk",
    "bp", "br", "bn", "bb", "bq", "bk"
};

const int pieceValue[13] = {
    0,      // empty
    100,    // wp
    320,    // wn  (approx 3.2 pawns)
    330,    // wb  (approx 3.3 pawns)
    500,    // wr  (5 pawns)
    900,    // wq  (9 pawns)
    50000,  // wk  (king has no material value)
    
    100,    // bp
    320,    // bn
    330,    // bb
    500,    // br
    900,    // bq
    50000   // bk
};


const int Kings[] = {wk, bk};

const int maxDepth = 64;
const int Infinite = 30000;
const int Mate = Infinite - maxDepth;

const int CastlePermission[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 11, 15, 15, 15, 3, 15, 15, 7, 15,//white 1011 0011 0111
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 14, 15, 15, 15, 12, 15, 15, 13, 15,//black 1101 1100 1110
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

const int Mirror64[] = {
    56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
    48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
    40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
    32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
    24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
    16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
    8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
    0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};


const int rookDirections[] = {-10, 1, 10, -1};
const int knightDirections[] = {19, 21, 12, 8, -19, -21, -12, -8};
const int bishopDirections[] = {-9, -11, 9, 11};
const int kingDirections[] = {-10, 1, 10, -1, -9, -11, 9, 11};
const int queenDirections[] = {-10, 1, 10, -1, -9, -11, 9, 11};

const int slidingPieces[2][3] = {
    wr, wb, wq,
    br, bb, bq,
};
const int nonSlidingPieces[2][2] = {
    wn, wk,
    bn, bk,
};

void initSquareMappings() {
    for (int rank = rank1; rank <= rank8; ++rank) {
        for (int file = fileA; file <= fileH; ++file) {
            int sq120 = fileRank2Sq(file, rank);
            int sq64 = rank * 8 + file;
            sq120To64[sq120] = sq64;
            sq64To120[sq64] = sq120;
        }
    }
}

void initialize() {
    initSquareMappings();
}

std::string moveStr(int move) {
    if (!move) return "";

    char moveStr[5];
    int from = moveFrom(move);
    int to = moveTo(move);

    moveStr[0] = 'a' + fileOf(from);
    moveStr[1] = '1' + rankOf(from);
    moveStr[2] = 'a' + fileOf(to);
    moveStr[3] = '1' + rankOf(to);
    moveStr[4] = '\0';

    return std::string(moveStr);
}

long long getCurrTime() {
    // Get current time with milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return milliseconds;
}