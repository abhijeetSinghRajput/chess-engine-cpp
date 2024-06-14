#include "transpositionTable.hpp"
#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include "defs.hpp"
#include "movegen.hpp"

const int AlphaFlag = 0;
const int BetaFlag = 1;
const int ExactFlag = 2;

inline U64 pack_tableData(int score, int depth, int flag, U64 move)
{
    return (score + 32000) | (depth << 16) | (flag << 22) | (move << 24);
}

inline int extract_score(U64 data) { return (data & 0xffff) - 32000; }
inline int extract_move(U64 data) { return data >> 24; }
inline int extract_flag(U64 data) { return (data >> 22) & 3; }
inline int extract_depth(U64 data) { return (data >> 16) & 0x3f; }

// void checkData(int move)
// {
//     int depth = rand() % maxDepth;
//     int flag = rand() % 3;
//     int score = rand() % Infinite;

//     U64 data = pack_tableData(score, depth, flag, move);
//     printf("\033[%dm", (depth == extract_depth(data) && flag == extract_flag(data) && score == extract_score(data)? 32 : 31));

//     printf("move : %s depth : %d flag : %d score : %d data %llx\n", moveStr(move).c_str(), depth, flag, score, data);
//     printf("move : %s depth : %d flag : %d score : %d data %llx\n\n",
//            moveStr(move).c_str(),
//            extract_depth(data),
//            extract_flag(data),
//            extract_score(data),
//            data);
//     printf("\033[0m");
// }

void verifyEntrySMP(TableData &entry){
    U64 data = pack_tableData(entry.score, entry.depth, entry.flag, entry.move);
    U64 key = entry.positionKey ^ data;

    if(data != entry.smp_data){ printf("data error"); exit(1);}
    if(key != entry.smp_key){ printf("smp_key error"); exit(1);}

    int move = extract_move(data);
    int flag = extract_flag(data);
    int score = extract_score(data);
    int depth = extract_depth(data);

    if(move != entry.move) {
        printf("\033[31mmove error\n");
        printf("data: %20lld\n", data);
        printf("ext_move: %20d  entry.move %20d\n", move, entry.move);
        printf("ext_flag: %20d  entry.flag %20d\n", flag, entry.flag);
        printf("ext_score: %20d  entry.score %20d\n", score, entry.score);
        printf("ext_depth: %20d  entry.depth %20d\n", depth, entry.depth);
        printf("\033[0m");
        exit(1);
    }
    if(flag != entry.flag) {printf("flag error"); exit(1);}
    if(score != entry.score) {printf("score error"); exit(1);}
    if(depth != entry.depth) {printf("depth error"); exit(1);}
}


TranspositionTable *transpositionTable = new TranspositionTable;

TranspositionTable::TranspositionTable()
{
    clear();
}
TranspositionTable::~TranspositionTable()
{
    clear();
}

void TranspositionTable::clear()
{
    for(int i = 0; i<this->maxEntries; ++i){
        this->entries[i].positionKey = 0ULL;
        this->entries[i].move = 0;
        this->entries[i].depth = 0;
        this->entries[i].score = 0;
        this->entries[i].flag = 0;
        this->entries[i].smp_data = 0ULL;
        this->entries[i].smp_key = 0ULL;
    }
}

void TranspositionTable::add(U64 positionKey, int move, int score, int flag, int depth)
{
    if (score > Mate)
        score += searchController->ply;
    if (score < -Mate)
        score -= searchController->ply;
    long long index = positionKey % this->maxEntries;

    U64 smp_data = pack_tableData(score, depth, flag, move);
    U64 smp_key = positionKey ^ smp_data;

    this->entries[index].positionKey = positionKey;
    this->entries[index].move = move;
    this->entries[index].score = score;
    this->entries[index].depth = depth;
    this->entries[index].flag = flag;
    this->entries[index].smp_data = smp_data;
    this->entries[index].smp_key = smp_key;

    verifyEntrySMP(this->entries[index]);
}

TableData* TranspositionTable::get(U64 positionKey) {
    long long index = positionKey % this->maxEntries;
    TableData* data = &this->entries[index];
    if (data->positionKey != positionKey) return nullptr;
    return data;
}

int TranspositionTable::getMove()
{
    long long index = board->positionKey % this->maxEntries;
    TableData *data = &this->entries[index];

    if (data->positionKey == board->positionKey)
        return data->move;
    return 0;
}

bool isMoveExists(int arg)
{
    std::vector<std::pair<int, int>> moves = generateMoves();
    for (auto &pair : moves)
    {
        if (pair.first == arg)
        {
            if (makeMove(pair.first) == false)
                continue;
            takeMove();
            return true;
        }
    }
    return false;
}

std::vector<int> TranspositionTable::getLine(int depth)
{
    std::vector<int> moveList;
    int move = this->getMove();
    int count = 0;

    while (move && count < depth)
    {
        if (isMoveExists(move))
        {
            makeMove(move);
            moveList.push_back(move);
            count++;
        }
        else
        {
            break;
        }
        move = this->getMove();
    }
    while (count--)
        takeMove();
    return moveList;
}