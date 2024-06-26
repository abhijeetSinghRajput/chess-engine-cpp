#pragma once
#include "defs.hpp"
#include <atomic>

class SearchController
{
private:
public:
    long long startTime;
	long long stopTime;
	int depth;
	int timeSet;

	long long nodes;

	bool quit;
	std::atomic<bool> stopped;

	float fh;
	float fhf;
	bool useBook;
    int ply;
    int killers[64][2];   // killer[ply][0/1];
    int history[13][120]; // history[piece][toSq]
	SearchController();
    void clear();
};

int searchPosition();
int alphaBeta(int alpha, int beta, int depth, bool doNull);
int quiescence(int alpha, int beta);
void swapWithBest(int i, std::vector<std::pair<int, int>> &moves);
void checkTimeUp();
bool isRepetition();

extern SearchController *searchController;
