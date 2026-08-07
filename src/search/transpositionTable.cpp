#include "search/transpositionTable.hpp"
#include "core/board.hpp"
#include "search/search.hpp"
#include "core/move.hpp"
#include "core/defs.hpp"
#include "core/movegen.hpp"
const int AlphaFlag = 0;
const int BetaFlag = 1;
const int ExactFlag = 2;


TranspositionTable *transpositionTable = new TranspositionTable;

TranspositionTable::TranspositionTable()
{
    resize(16); // default 16MB
}
TranspositionTable::~TranspositionTable()
{
    delete[] entries;
}

void TranspositionTable::resize(size_t mbSize)
{
    delete[] entries; // safe even if entries is nullptr

    size_t bytes = mbSize * 1024ULL * 1024ULL;
    size_t numEntries = bytes / sizeof(TableData);

    size_t pow2 = 1;
    while (pow2 * 2 <= numEntries)
        pow2 *= 2;
    if (pow2 == 0)
        pow2 = 1; // safety floor

    entries = new TableData[pow2];
    sizeMask = pow2 - 1;
    clear();
}

void TranspositionTable::clear()
{
    size_t numEntries = sizeMask + 1;
    for (size_t i = 0; i < numEntries; ++i)
    {
        entries[i].smp_data = 0ULL;
        entries[i].smp_key = 0ULL;
    }
}

void TranspositionTable::add(U64 positionKey, int move, int score, int flag, int depth)
{
    if (score > MATE)  score += searchController->ply;
    if (score < -MATE) score -= searchController->ply;

    size_t index = positionKey & sizeMask;

    U64 smp_data = pack_tableData(score, depth, flag, move);
    U64 smp_key = positionKey ^ smp_data;

    this->entries[index].smp_data = smp_data;
    this->entries[index].smp_key = smp_key;
}

TableData *TranspositionTable::get(U64 positionKey)
{
    size_t index = positionKey & sizeMask;
    TableData *data = &this->entries[index];

    if ((data->smp_key ^ data->smp_data) != positionKey)
        return nullptr;

    return data;
}

int TranspositionTable::getMove()
{
    size_t index = board->positionKey & sizeMask;
    TableData *data = &this->entries[index];

    if ((data->smp_key ^ data->smp_data) == board->positionKey)
        return extract_move(data->smp_data);

    return 0;
}

bool isMoveExists(int arg)
{
    MoveList list;
    generateMoves(list);
    for(int i = 0; i<list.count; ++i)
    {
        int move = list.moves[i].move;
        if (move == arg)
        {
            if (makeMove(move) == false)
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