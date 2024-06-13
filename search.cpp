#include "search.hpp"
#include "board.hpp"
#include "defs.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "evaluation.hpp"
#include "transpositionTable.hpp"
#include "utils.hpp"

SearchController *searchController = new SearchController;

void SearchController::clear()
{
    for (int i = 0; i < maxDepth; ++i)
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
    depthReached = 0;
    nodes = 0;
    fh = 0;
    fhf = 0;
    start = 0; // current date;
    stop = false;
}

int searchPosition(int thinkingTime)
{
    int bestMove = 0;
    int bestScore = -Infinite;
    int depth = 1;
    float ordering = 0;

    searchController->depth = maxDepth;
    searchController->start = getCurrTime();
    searchController->time = thinkingTime;

    for (depth = 1; depth <= searchController->depth; ++depth)
    {
        bestScore = alphaBeta(-Infinite, Infinite, depth, true);
        if (searchController->stop)
            break;

        int bestMove = transpositionTable->getMove();
        if(depth != 1 && searchController->fh){
			ordering = ((float)searchController->fhf / (float)searchController->fh) * 100;
        }
    }

    searchController->thinking = false;
}

int getCurrTime();

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

    if (searchController->ply >= maxDepth)
    {
        return evalPosition();
    }

    bool inCheck = board->checkSq != noSq;
    if (inCheck)
    {
        ++depth;
    }
    int score = -Infinite;
    TableData *ttEntry = transpositionTable->get(board->positionKey);
    int pvMove = 0;
    if(ttEntry){
        pvMove = ttEntry->move;
        if(ttEntry->depth >= depth){
            score = ttEntry->score;
            if(score > Mate) score -= searchController->ply;
            else if(score < -Mate) score += searchController->ply;
			if(ttEntry->flag == AlphaFlag && score <= alpha) return alpha;
			if(ttEntry->flag == BetaFlag && score >= beta) return beta;
			if(ttEntry->flag == ExactFlag) return score;
        }
    }

    //NULL Move Pruning
    if (doNull && !inCheck && searchController->ply && depth >= 4)
    {
        makeNullMove();
        score = -alphaBeta(-beta, -beta + 1, depth - 4, false);
        takeNullMove();
        if (searchController->stop)
            return 0;
        if (score >= beta)
        {
            return beta;
        }
    }

    std::vector<int> moves = generateMoves();
    int legalMoves = 0;
    int prevAlpha = alpha;
    int bestMove = 0;

    // iterate all moves and give the score 2000000 to order pv move
    if(pvMove){

    }
    
    for (int i = 0; i < moves.size(); ++i)
    {
        // swapWithBest(i, moves);
        int move = moves[i];
        if (makeMove(move) == false)
            continue;
        legalMoves++;
        searchController->ply++;
        score = -alphaBeta(-beta, -alpha, depth - 1, true);

        takeMove();
        searchController->ply--;

        if (searchController->stop)
            return 0;

        if (score > alpha)
        {
            bestMove = move;

            if (score >= beta)
            {
                if (legalMoves == 1)
                    searchController->fhf++;
                searchController->fh++;
                if (!(move & captureFlag))
                {
                    searchController->killers[searchController->ply][1] = searchController->killers[searchController->ply][0];
                    searchController->killers[searchController->ply][0] = move;
                }

                transpositionTable->add(board->positionKey, move, beta, BetaFlag, depth);
                return beta;
            }
            alpha = score;
            if (!(move & captureFlag))
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
            return -Infinite + searchController->ply;
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

    if ((searchController->nodes % 2048) == 0)
    {
        checkTimeUp();
    }

    searchController->nodes++;

    if ((isRepetition() || board->fiftyMove >= 100) && searchController->ply != 0)
    {
        return 0;
    }

    if (searchController->ply >= maxDepth)
    {
        return evalPosition();
    }

    int score = evalPosition();

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    int legalMove = 0;
    std::vector<int> moves = generateCaptureMoves();

    for (int i = 0; i < moves.size(); ++i)
    {
        swapWithBest(i, moves);
        const int move = moves[i];

        if (makeMove(move) == false)
            continue;
        legalMove++;
        searchController->ply++;

        score = -quiescence(-beta, -alpha);

        takeMove();
        searchController->ply--;

        if (searchController->stop)
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

void swapWithBest(int i, std::vector<int> &moves)
{
    
}

bool isRepetition() {
	for (int i = board->ply - board->fiftyMove; i < board->ply - 1; ++i) {
		if (board->positionKey == board->history[i]->positionKey) {
			return true;
		}
	}

	return false;
}