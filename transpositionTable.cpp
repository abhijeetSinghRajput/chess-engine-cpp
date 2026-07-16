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
    currentAge = 0;
    clear();
}

TranspositionTable::~TranspositionTable()
{
    clear();
}

void TranspositionTable::clear()
{
    for (int i = 0; i < this->maxEntries; ++i)
    {
        this->entries[i].smp_data = 0ULL;
        this->entries[i].smp_key = 0ULL;
        this->entries[i].age = 0;
    }
}

void TranspositionTable::newSearch()
{
    currentAge++; // Increment age for each new search
    if (currentAge > 1000000)
    {
        // Prevent overflow, reset ages
        currentAge = 1;
        for (int i = 0; i < maxEntries; ++i)
        {
            if (entries[i].age > 0)
            {
                entries[i].age = 1;
            }
        }
    }
}

void TranspositionTable::add(U64 positionKey, int move, int score, int flag, int depth)
{
    if (score > MATE)       score -= searchController->ply;
    else if (score < -MATE) score += searchController->ply;

    size_t index = positionKey % (size_t)this->maxEntries;
    TableData *entry = &this->entries[index];

    // Depth-preferred + Aging replacement policy
    bool replace = false;

    // Case 1: Empty entry - always replace
    if (entry->smp_key == 0 && entry->smp_data == 0)
    {
        replace = true;
    }
    // Case 2: Same position - always replace (update)
    else if ((entry->smp_key ^ entry->smp_data) == positionKey)
    {
        replace = true;
    }
    // Case 3: New entry is deeper - depth-preferred
    else if (depth > extract_depth(entry->smp_data))
    {
        replace = true;
    }
    // Case 4: Entry is old (aged out) - aging policy
    else if (currentAge - entry->age > 2)
    {
        replace = true;
    }
    // Case 5: Same depth but newer - prefer newer
    else if (depth == extract_depth(entry->smp_data) &&
             currentAge > entry->age)
    {
        replace = true;
    }
    // Case 6: Always replace if position is old (even if deeper)
    else if (currentAge - entry->age > 5)
    {
        replace = true;
    }

    if (replace)
    {
        U64 smp_data = pack_tableData(score, depth, flag, move);
        U64 smp_key = positionKey ^ smp_data;

        this->entries[index].smp_data = smp_data;
        this->entries[index].smp_key = smp_key;
        this->entries[index].age = currentAge;
    }
}

TableData *TranspositionTable::get(U64 positionKey)
{
    size_t index = positionKey % (size_t)this->maxEntries;
    TableData *data = &this->entries[index];

    if ((data->smp_key ^ data->smp_data) != positionKey)
        return nullptr;

    return data;
}

int TranspositionTable::getMove()
{
    size_t index = board->positionKey % (size_t)this->maxEntries;
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