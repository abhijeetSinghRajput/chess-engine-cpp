// search.hpp
#pragma once
#include "core/defs.hpp"
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
    int killers[MAX_DEPTH][2];   // killer[ply][0/1];
    int history[13][64]; // history[piece][toSq]
	int counterMoves[64][64]; // counterMoves[from_sq][to_sq]
	SearchController();
    void clear();

	// History Decay
	void ageHistory() {
		for (int i = 0; i < 13; i++) {
			for (int j = 0; j < 64; j++) {
				history[i][j] *= 0.8; // 20% deacy
			}
		}
	}
};

int searchPosition();
int alphaBeta(int alpha, int beta, int depth, bool doNull);
int quiescence(int alpha, int beta, int checkPly = 0);
void swapWithBest(int start, MoveList& moveList);
void checkTimeUp();
bool isRepetition();

extern SearchController *searchController;
