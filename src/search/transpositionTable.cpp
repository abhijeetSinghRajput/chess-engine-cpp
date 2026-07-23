// transpositionTable.cpp
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
    currentAge = 0;
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
        entries[i].age = 0;
    }
}

void TranspositionTable::newSearch()
{
    currentAge++; // Increment age for each new search
    if (currentAge > 1000000)
    {
        // Prevent overflow, reset ages
        currentAge = 1;
        size_t numEntries = sizeMask + 1;
        for (size_t i = 0; i < numEntries; ++i)
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
    if (score > MATE)       score += searchController->ply;
    else if (score < -MATE) score -= searchController->ply;

    size_t index = positionKey & sizeMask;
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
std::vector<int> TranspositionTable::getLine(int depth)
{
    std::vector<int> pvLine;

    int move = this->getMove();
    int count = 0;

    std::vector<int> madeMoves;

    while (move && count < depth)
    {
        bool legal = false;

        MoveList generatedMoves;
        generateMoves(generatedMoves);

        for (int i = 0; i < generatedMoves.count; ++i)
        {
            if (generatedMoves.moves[i].move == move)
            {
                if (makeMove(move))
                {
                    legal = true;

                    madeMoves.push_back(move);
                    pvLine.push_back(move);

                    ++count;
                    break;
                }
            }
        }

        if (!legal)
            break;

        move = this->getMove();
    }

    while (!madeMoves.empty())
    {
        takeMove();
        madeMoves.pop_back();
    }

    return pvLine;
}