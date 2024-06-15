#include "transpositionTable.hpp"
#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include "defs.hpp"
#include "movegen.hpp"

const int AlphaFlag = 0;
const int BetaFlag = 1;
const int ExactFlag = 2;


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

    this->entries[index].smp_data = smp_data;
    this->entries[index].smp_key = smp_key;
}

TableData* TranspositionTable::get(U64 positionKey) {
    long long index = positionKey % this->maxEntries;
    TableData* data = &this->entries[index];
    if ((data->smp_key ^ data->smp_data) != positionKey) return nullptr;

    return data;
}

int TranspositionTable::getMove()
{
    long long index = board->positionKey % this->maxEntries;
    TableData *data = &this->entries[index];

    if ((data->smp_key ^ data->smp_data) == board->positionKey)
        return extract_move(data->smp_data);

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