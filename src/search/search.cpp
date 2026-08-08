#include "search/search.hpp"
#include "core/board.hpp"
#include "core/defs.hpp"
#include "core/move.hpp"
#include "core/movegen.hpp"
#include "eval/evaluation.hpp"
#include "search/transpositionTable.hpp"
#include "core/utils.hpp"
#include <iostream>
#include <iomanip>
#include "core/polyglot.hpp"

SearchController *searchController = new SearchController;

SearchController::SearchController()
{
    this->clear();
}

void SearchController::clear()
{
    for (int i = 0; i < MAX_DEPTH; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            killers[i][j] = 0;
        }
    }
    for (int i = 0; i < 13; ++i)
    {
        for (int j = 0; j < 64; ++j)
        {
            history[i][j] = 0;
        }
    }
    ply = 0;
    nodes = 0;
    fh = 0;
    fhf = 0;
    stopped = false;
    useBook = false;
}

int searchPosition()
{
    int bestMove = 0;
    int bestScore = -INFINITE;
    int depth = 1;
    float ordering = 0;

    if (searchController->useBook)
    {
        bestMove = getRandBookMove();
        if (bestMove)
        {
            std::cout << "bestmove " << moveStr(bestMove) << std::endl;
            return bestMove;
        }
    }

    searchController->clear();
    for (depth = 1; depth <= searchController->depth; ++depth)
    {
        bestScore = alphaBeta(-INFINITE, INFINITE, depth, true);
        if (searchController->stopped)
            break;

        bestMove = transpositionTable->getMove();
        if (depth != 1 && searchController->fh)
        {
            ordering = (searchController->fhf / searchController->fh) * 100;
        }
        std::vector<int> line = transpositionTable->getLine(depth);
        std::string lineStr;
        for (auto move : line)
        {
            lineStr += moveStr(move) + ' ';
        }

        std::cout 
            << std::fixed << std::setprecision(2)
            << "info score "
            << " cp "       << bestScore 
            << " depth "    << depth
            << " ordering " << ordering
            << " nodes "    << searchController->nodes 
            << " time "     << getCurrTime() - searchController->startTime
            << " pv "      << lineStr
            << std::endl;
    }
    std::cout << "bestmove " << moveStr(bestMove) << std::endl;
    return bestMove;
}

int alphaBeta(int alpha, int beta, int depth, bool doNull)
{
    if (depth <= 0)
    {
        return quiescence(alpha, beta);
    }

    // check every 2048th node
    if ((searchController->nodes & 2047) == 0)
    {
        checkTimeUp();
    }
    searchController->nodes++;
    if ((isRepetition() || board->fiftyMove >= 100) && searchController->ply != 0)
    {
        return 0;
    }

    if (searchController->ply >= MAX_DEPTH)
    {
        return evalPosition();
    }

    bool inCheck = board->checkSq != SQ_NONE;
    if (inCheck)
    {
        ++depth;
    }
    int score = -INFINITE;
    TableData *ttEntry = transpositionTable->get(board->positionKey);
    int pvMove = 0;
    if (ttEntry)
    {
        pvMove = extract_move(ttEntry->smp_data);
        if (extract_depth(ttEntry->smp_data) >= depth)
        {
            score = extract_score(ttEntry->smp_data);
            if (score > MATE)       score -= searchController->ply;
            else if (score < -MATE) score += searchController->ply;

            if (score > MATE)
                score -= searchController->ply;
            else if (score < -MATE)
                score += searchController->ply;
            if (extract_flag(ttEntry->smp_data) == AlphaFlag && score <= alpha)
                return alpha;
            if (extract_flag(ttEntry->smp_data) == BetaFlag && score >= beta)
                return beta;
            if (extract_flag(ttEntry->smp_data) == ExactFlag)
                return score;
        }
    }

    // NULL Move Pruning
    if (doNull && !inCheck && searchController->ply && depth >= 4)
    {
        makeNullMove();
        score = -alphaBeta(-beta, -beta + 1, depth - 4, false);
        takeNullMove();
        if (searchController->stopped)
            return 0;
        if (score >= beta)
        {
            return beta;
        }
    }

    MoveList list;
    generateMoves(list);
    int legalMoves = 0;
    int prevAlpha = alpha;
    int bestMove = 0;

    // MOVE ORDERING : Give the pv move highest priority
    if (pvMove)
    {
        for (int i = 0; i < list.count; ++i)
        {
            if (list.moves[i].move == pvMove)
            {
                list.moves[i].score = 2000000;
                break;
            }
        }
    }

    for (int i = 0; i < list.count; ++i)
    {
        swapWithBest(i, list);
        int move = list.moves[i].move;
        if (makeMove(move) == false)
            continue;
        legalMoves++;
        searchController->ply++;
        score = -alphaBeta(-beta, -alpha, depth - 1, true);

        takeMove();
        searchController->ply--;

        if (searchController->stopped)
            return 0;

        if (score > alpha)
        {
            bestMove = move;

            if (score >= beta)
            {
                if (legalMoves == 1)
                    searchController->fhf++;
                searchController->fh++;
                if (!(move & CAPTURE_FLAG))
                {
                    searchController->killers[searchController->ply][1] = searchController->killers[searchController->ply][0];
                    searchController->killers[searchController->ply][0] = move;
                }

                transpositionTable->add(board->positionKey, move, beta, BetaFlag, depth);
                return beta;
            }
            alpha = score;
            if (!(move & CAPTURE_FLAG))
            {
                int piece = board->pieces[moveFrom(move)];
                int toSq = moveTo(move);
                searchController->history[piece][toSq] += depth * depth;
            }
        }
    }

    if (legalMoves == 0)
    {
        if (inCheck)
        {
            return -INFINITE + searchController->ply;
        }
        else
        {
            return 0;
        }
    }

    if (alpha != prevAlpha)
    {
        transpositionTable->add(board->positionKey, bestMove, score, ExactFlag, depth);
    }
    else
    {
        transpositionTable->add(board->positionKey, bestMove, alpha, AlphaFlag, depth);
    }

    return alpha;
}

int quiescence(int alpha, int beta)
{

    // check every 2048th node
    if ((searchController->nodes & 2047) == 0)
    {
        checkTimeUp();
    }

    searchController->nodes++;

    if ((isRepetition() || board->fiftyMove >= 100) && searchController->ply != 0)
    {
        return 0;
    }

    if (searchController->ply >= MAX_DEPTH)
    {
        return evalPosition();
    }

    int score = evalPosition();

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    int legalMove = 0;
    MoveList list;
    generateMoves(list);

    for (int i = 0; i < list.count; ++i)
    {
        swapWithBest(i, list);
        const int move = list.moves[i].move;

        // quiescence: only search noisy moves
        bool isNoisy = (move & CAPTURE_FLAG) || (move & EN_PASSANT_FLAG) || (move & PROMOTION_FLAG);
        if (!isNoisy) continue;


        if (makeMove(move) == false)
            continue;
        legalMove++;
        searchController->ply++;

        score = -quiescence(-beta, -alpha);

        takeMove();
        searchController->ply--;

        if (searchController->stopped)
            return 0;

        if (score > alpha)
        {
            if (score >= beta)
            {
                if (legalMove == 1)
                {
                    searchController->fhf++;
                }
                searchController->fh++;
                return beta;
            }
            alpha = score;
        }
    }

    return alpha;
}

void checkTimeUp()
{
    if (searchController->timeSet && getCurrTime() > searchController->stopTime)
    {
        searchController->stopped = true;
    }
}

void swapWithBest(int i, MoveList &list)
{
    int bestIndex = i;
    for (int j = i + 1; j < list.count; ++j)
    {
        if (list.moves[j].score > list.moves[bestIndex].score)
        {
            bestIndex = j;
        }
    }
    if (bestIndex != i)
    {
        std::swap(list.moves[i], list.moves[bestIndex]);
    }
}

bool isRepetition()
{
    for (int i = board->ply - board->fiftyMove; i < board->ply - 1; ++i)
    {
        if (board->positionKey == board->history[i].positionKey)
        {
            return true;
        }
    }

    return false;
}