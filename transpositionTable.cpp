#include "transpositionTable.hpp"
#include "board.hpp"
#include "search.hpp"
#include "move.hpp"
#include "movegen.hpp"

const int AlphaFlag = 0;
const int BetaFlag = 1;
const int ExactFlag = 2;

TranspositionTable *transpositionTable = new TranspositionTable;


TranspositionTable::TranspositionTable()
{
    clear();
}

void TranspositionTable::clear()
{
    for(int i = 0; i<maxEntries; ++i){
        this->entries[i] = NULL;
    }
}

void TranspositionTable::add(U64 positionKey, int move, int score, int flag, int depth)
{
    if (score > Mate)
        score += searchController->ply;
    if (score < -Mate)
        score -= searchController->ply;
    int index = positionKey % this->maxEntries;

    this->entries[index++] = new TableData{
        positionKey,
        move,
        score,
        depth,
        flag
    };
}

TableData* TranspositionTable::get(U64 positionKey)
{
    int index = positionKey % this->maxEntries;
    TableData *data = this->entries[index];

    if(data && data->positionKey != positionKey) return NULL;
    return data;
}

int TranspositionTable::getMove()
{
    int index = board->positionKey % this->maxEntries;
    TableData *data = this->entries[index];

    if(data && data->positionKey != board->positionKey) return 0;
    return data->move;
}

bool isMoveExists(int arg){
    std::vector<int> moves = generateMoves();
    for(auto move : moves){
        if(move == arg){
            if(makeMove(move) == false) continue;
            takeMove();
            return true;
        }
    }
    return false;
}

std::vector<int> &TranspositionTable::getLine(int depth){
    int move = this->getMove();
    static std::vector<int> moveList;
    int count = 0;

    while (move && count < depth)
    {
        if(isMoveExists(move)){
            makeMove(move);
            moveList.push_back(move);
            count++;
        }
        else{
            break;
        }
        move = this->getMove();
    }
    while(count--) takeMove();
    return moveList;
}