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
        for (int j = 0; j < 120; ++j)
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
    int bestScore = -AB_BOUND;
    int depth = 1;
    float ordering = 0;

    transpositionTable->newSearch();

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
        bestScore = alphaBeta(-AB_BOUND, AB_BOUND, depth, true);
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
            << "info "
            << "score cp " << bestScore
            << " depth " << depth
            << " ordering " << ordering
            << " nodes " << searchController->nodes
            << " time " << (getCurrTime() - searchController->startTime)
            << " pv " << lineStr
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

    if ((searchController->nodes % 2048) == 0)
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
    int score = -AB_BOUND;
    int bestScore = -AB_BOUND;
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

            if (extract_flag(ttEntry->smp_data) == AlphaFlag && score <= alpha)
                return alpha;
            if (extract_flag(ttEntry->smp_data) == BetaFlag && score >= beta)
                return beta;
            if (extract_flag(ttEntry->smp_data) == ExactFlag)
                return score;
        }
    }

    // NULL Move Pruning
    if (
        doNull && 
        !inCheck && 
        searchController->ply && 
        depth >= 4 && 
        bigPieceCount(board->side) > 1
    )
    {
        makeNullMove();
        score = -alphaBeta(-beta, -beta + 1, depth - 4, false);
        takeNullMove();
        if (searchController->stopped)
            return 0;
        if (score >= beta && abs(score) < MATE)
        {
            return beta;
        }
    }

    std::vector<std::pair<int, int>> moves = generateMoves();
    int legalMoves = 0;
    int prevAlpha = alpha;
    int bestMove = 0;

    if (pvMove)
    {
        for (auto &pair : moves)
        {
            if (pair.first == pvMove)
            {
                pair.second = 2000000;
                break;
            }
        }
    }

    for (auto i = 0u; i < moves.size(); ++i)
    {
        swapWithBest(i, moves);
        int move = moves[i].first;
        if (makeMove(move) == false)
            continue;
        legalMoves++;
        searchController->ply++;

        // ===== LMR =====
        int reduction = 0;
        if (
            depth >= 3 && i >= 4 && 
            !(move & CAPTURE_FLAG) && 
            !inCheck && !(move & PROMOTION_FLAG)
        )
        {
            reduction = 1;
            if (i >= 8)
                reduction = 2;
            if (i >= 16)
                reduction = 3;
            // Don't reduce killer moves
            if (move == searchController->killers[searchController->ply][0] ||
                move == searchController->killers[searchController->ply][1])
            {
                reduction = 0;
            }
        }

        score = -alphaBeta(-beta, -alpha, depth - 1 - reduction, true);

        // Re-search if reduction failed
        if (reduction && score > alpha)
        {
            score = -alphaBeta(-beta, -alpha, depth - 1, true);
        }

        // ===== End LMR =====

        takeMove();
        searchController->ply--;

        if (searchController->stopped)
            return 0;

        bestScore = std::max(bestScore, score);
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
            return -AB_BOUND + searchController->ply;
        }
        else
        {
            return 0;
        }
    }

    if (alpha != prevAlpha)
    {
        transpositionTable->add(board->positionKey, bestMove, bestScore, ExactFlag, depth);
    }
    else
    {
        transpositionTable->add(board->positionKey, bestMove, alpha, AlphaFlag, depth);
    }

    return alpha;
}

int quiescence(int alpha, int beta, int checkPly)
{
    if ((searchController->nodes % 2048) == 0)
        checkTimeUp();

    searchController->nodes++;

    if ((isRepetition() || board->fiftyMove >= 100) && searchController->ply != 0)
        return 0;

    if (searchController->ply >= MAX_DEPTH)
        return evalPosition();

    const int MAX_QCHECK_PLY = 8;
    bool inCheck = board->checkSq != SQ_NONE;
    bool expandFull = inCheck && checkPly < MAX_QCHECK_PLY;

    int score = -AB_BOUND;

    if (!expandFull)
    {
        score = evalPosition();
        if (score >= beta) return beta;

        // Delta pruning: even winning a queen can't save this node, skip captures
        const int DELTA_MARGIN = 200;
        if (!inCheck && score + QUEEN_VALUE + DELTA_MARGIN < alpha)
            return alpha;

        if (score > alpha) alpha = score;
    }

    int legalMove = 0;
    std::vector<std::pair<int, int>> moves =
        expandFull ? generateMoves() : generateCaptureMoves();

    for (auto i = 0u; i < moves.size(); ++i)
    {
        swapWithBest(i, moves);
        const int move = moves[i].first;

        if (makeMove(move) == false)
            continue;
        legalMove++;
        searchController->ply++;

        score = -quiescence(-beta, -alpha, expandFull ? checkPly + 1 : 0);

        takeMove();
        searchController->ply--;

        if (searchController->stopped)
            return 0;

        if (score > alpha)
        {
            if (score >= beta)
            {
                if (legalMove == 1) searchController->fhf++;
                searchController->fh++;
                return beta;
            }
            alpha = score;
        }
    }

    if (legalMove == 0 && expandFull)
        return -AB_BOUND + searchController->ply;

    return alpha;
}

void checkTimeUp()
{
    if (searchController->timeSet && getCurrTime() > searchController->stopTime)
    {
        searchController->stopped = true;
    }
}

void swapWithBest(int i, std::vector<std::pair<int, int>> &moves)
{
    int bestIndex = i;
    for (auto j = unsigned(i + 1); j < moves.size(); ++j)
    {
        if (moves[j].second > moves[bestIndex].second)
        {
            bestIndex = j;
        }
    }
    if (bestIndex != i)
    {
        std::swap(moves[i], moves[bestIndex]);
    }
}

bool isRepetition()
{
    int start = std::max(0, board->ply - board->fiftyMove);

    for (int i = start; i < board->ply; ++i)
    {
        if (board->positionKey == board->history[i].positionKey)
        {
            return true;
        }
    }

    return false;
}