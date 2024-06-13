#pragma once

void addCaptureMove(int move);
void addQuiteMove(int move);
void addEnPassantMove(int move);
void addWhitePawnQuietMove(int from,int to);
void addBlackPawnQuietMove(int from, int to);
void addWhiteCaptureMove(int from,int to,int capture);
void addBlackCaptureMove(int from,int to,int capture);

void genNonSlidingMoves(bool capturesOnly = false);
void genSlidingMoves(bool capturesOnly = false);

std::vector<int> &generateMoves();
std::vector<int> &generateCaptureMoves();
