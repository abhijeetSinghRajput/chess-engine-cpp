#include "core/defs.hpp"
#include "core/move.hpp"
#include <chrono>

// Initialize global variables
std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
int sq120To64[120];
int sq64To120[64];

const char* PIECE_ICON[] = {
    ".",
    u8"♙", u8"♖", u8"♘", u8"♗", u8"♕", u8"♔",
    u8"♟", u8"♜", u8"♞", u8"♝", u8"♛", u8"♚"
};
const char PIECE_CHAR[] = ".PRNBQKprnbqk";
const char PIECE_TYPE[] = ".prnbqkprnbqk";
const char FILE_CHAR[] = "abcdefgh";
const int  PIECE_COLOR[] = {
    2,
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1,
};
const char *SQUARE_CHAR[] = {
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
const char *PIECE_NAME[] = {
    ".",
    "wp", "wr", "wn", "wb", "wq", "wk",
    "bp", "br", "bn", "bb", "bq", "bk"
};

const int pieceValue[13] = {
    VALUE_ZERO,      // empty
    PAWN_VALUE,      // wp
    ROOK_VALUE,      // wr  (5 pawns)
    KNIGHT_VALUE,    // wn  (approx 3.2 pawns)
    BISHOP_VALUE,    // wb  (approx 3.3 pawns)
    QUEEN_VALUE,     // wq  (9 pawns)
    KING_VALUE,      // wk  (king has no material value)
    
    PAWN_VALUE,      // bp
    ROOK_VALUE,      // br
    KNIGHT_VALUE,    // bn
    BISHOP_VALUE,    // bb
    QUEEN_VALUE,     // bq
    KING_VALUE       // bk
};


const int Kings[] = {PIECE_WK, PIECE_BK};

const int MAX_DEPTH = 64;
const int INFINITE = 32001;
const int MATE = INFINITE - MAX_DEPTH;

const int CASTLE_PERMISSION[] = {
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

const int MIRROR_64[] = {
    56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
    48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
    40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
    32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
    24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
    16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
    8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
    0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};


const int ROOK_DIRECTIONS  [] = {-10, 1, 10, -1};
const int KNIGHT_DIRECTIONS[] = {19, 21, 12, 8, -19, -21, -12, -8};
const int BISHOP_DIRECTIONS[] = {-9, -11, 9, 11};
const int KING_DIRECTIONS  [] = {-10, 1, 10, -1, -9, -11, 9, 11};
const int QUEEN_DIRECTIONS [] = {-10, 1, 10, -1, -9, -11, 9, 11};

const int slidingPieces[2][3] = {
    PIECE_WR, PIECE_WB, PIECE_WQ,
    PIECE_BR, PIECE_BB, PIECE_BQ,
};
const int nonSlidingPieces[2][2] = {
    PIECE_WN, PIECE_WK,
    PIECE_BN, PIECE_BK,
};

void initSquareMappings() {
    for (int rank = RANK_1; rank <= rank8; ++rank) {
        for (int file = FILE_A; file <= FILE_H; ++file) {
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

std::string moveStr(int move)
{
    if (!move) return "";

    char moveStr[6];

    int from = moveFrom(move);
    int to = moveTo(move);

    moveStr[0] = 'a' + fileOf(from);
    moveStr[1] = '1' + rankOf(from);
    moveStr[2] = 'a' + fileOf(to);
    moveStr[3] = '1' + rankOf(to);

    switch (movePromotionPiece(move))
    {
        case PIECE_WQ: case PIECE_BQ:   moveStr[4] = 'q'; moveStr[5] = '\0'; break;
        case PIECE_WR: case PIECE_BR:   moveStr[4] = 'r'; moveStr[5] = '\0'; break;
        case PIECE_WB: case PIECE_BB:   moveStr[4] = 'b'; moveStr[5] = '\0'; break;
        case PIECE_WN: case PIECE_BN:   moveStr[4] = 'n'; moveStr[5] = '\0'; break;

        default:                              moveStr[4] = '\0'; break;
    }

    return std::string(moveStr);
}

long long getCurrTime() {
    // Get current time with milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return milliseconds;
}