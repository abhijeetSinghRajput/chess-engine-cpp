#pragma once
extern const int PawnTable[];
extern const int KnightTable[];
extern const int BishopTable[];
extern const int RookTable[];
extern const int KingO[];
extern const int KingE[];

extern const int BishopPair;
extern const int PawnIsolated;
extern const int PawnPassed[];
extern const int RookOpenFile;
extern const int RookSemiOpenFile;
extern const int QueenOpenFile;
extern const int QueenSemiOpenFile;
extern const int EndGame_Material;

int evalPosition();