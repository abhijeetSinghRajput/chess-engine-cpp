#include "bitboard.hpp"
#include <iomanip>
#include <random>
#include <iostream>

U64 randomU64()
{
    static std::random_device rd;                                 // Seed
    static std::mt19937_64 gen(rd());                             // Mersenne Twister engine
    static std::uniform_int_distribution<U64> dis(0, UINT64_MAX); // Uniform distribution

    return dis(gen);
}
U64 random_64_fewbits()
{
    return randomU64() & randomU64() & randomU64();
}

// ==========================================================
// ======================= constructor ======================
// ==========================================================

U64 Bitboard::rookLookupTable[64][4096] = {};
U64 Bitboard::bishopLookupTable[64][4096] = {};
U64 Bitboard::fileMasks[8] = {0ULL};
U64 Bitboard::rankMasks[8] = {0ULL};
U64 Bitboard::isolatedPawnMask[64] = {0ULL};
U64 Bitboard::passedPawnMask[2][64] = {0ULL};
std::vector<U64> Bitboard::rookBlockers[64];
std::vector<U64> Bitboard::bishopBlockers[64];

// ==========================================================
// ======================= constructor ======================
// ==========================================================

Bitboard *bitboard = new Bitboard;

Bitboard::Bitboard()
{
    for (int i = 0; i < 13; ++i)
    {
        pieces[i] = 0ULL;
    }
    initMasks();
    generateBlockersFor(rookAttacks, rookBlockers);
    generateBlockersFor(bishopAttacks, bishopBlockers);
}

// ==========================================================
// ===================== helper functions ===================
// ==========================================================

void Bitboard::clearBit(int piece, int sq)
{
    pieces[piece] &= ~(1ULL << sq);
}
void Bitboard::setBit(U64 &bitBoard, int sq)
{
    bitBoard |= (1ULL << sq);
}
void Bitboard::setBit(int piece, int sq)
{
    pieces[piece] |= (1ULL << sq);
}
int Bitboard::getPieceCount(int piece)
{
    return __builtin_popcountll(pieces[piece]);
}
void Bitboard::movePiece(int piece, int fromSq, int toSq)
{
    if (!(pieces[piece] & (1ULL << fromSq)))
        return;
    clearBit(piece, fromSq);
    setBit(piece, toSq);
}

void Bitboard::print(int piece)
{
    U64 bitBoard = pieces[piece];
    printf("bitMask for piece \033[32m%s\033[0m\n", pieceName[piece]);
    std::cout << "0x" << std::hex << bitboard << std::dec << std::endl;
    for (int rank = rank8; rank >= rank1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = rank * 8 + file;
            std::cout << ((bitBoard & (1ULL << sq)) ? "\033[32m1\033[0m" : "\033[30m.\033[0m") << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
void Bitboard::print(U64 bitBoard)
{
    std::cout << "0x" << std::hex << bitboard << std::dec << std::endl;
    for (int rank = rank8; rank >= rank1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = rank * 8 + file;
            std::cout << ((bitBoard & (1ULL << sq)) ? "\033[32m1\033[0m" : "\033[30m.\033[0m") << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// ex key = (blokerBitboard * rookmagic[sq]) >> rookShifts[sq];
// call this function for each different 64 squares
/**
 * @param sq square for which you want to get magic number.
 * @param blockerBitboards all the possible blockerBitboards of 'sq' you are giving.
 * @param ortho true for rook : false for bishop
 * @return magic number if found
 */
U64 Bitboard::findMagic(int sq, const std::vector<U64> &blockerBitboards, bool ortho)
{
    std::vector<U64> attacksBitboards(blockerBitboards.size());
    U64 used[4096];
    U64 attackMask = ortho ? rookAttacks[sq] : bishopAttacks[sq];
    bool fail = false;

    // Generate attack bitboards for each blocker
    for (size_t i = 0; i < blockerBitboards.size(); ++i)
    {
        attacksBitboards[i] = legalMoveBitboardFromBlockers(sq, blockerBitboards[i], ortho);
    }

    // Try finding a suitable magic number
    for (int i = 0; i < 100000000; ++i)
    {
        U64 magic = random_64_fewbits();
        if (__builtin_popcountll((attackMask * magic) & 0xFF00000000000000ULL) < 6)
        {
            continue;
        }

        // Clear used array
        for (int i = 0; i < 4096; ++i)
        {
            used[i] = 0;
        }
        fail = false;

        for (size_t j = 0; j < blockerBitboards.size(); ++j)
        {
            int index = (blockerBitboards[j] * magic) >> (64 - 12); // Adjust shift for your table size
            if (used[index] == 0)
            {
                used[index] = attacksBitboards[j];
            }
            else if (used[index] != attacksBitboards[j])
            {
                fail = true;
                break;
            }
        }

        if (!fail)
        {
            return magic;
        }
    }

    // If no suitable magic number found, return 0
    return 0;
}

U64 Bitboard::getPieces(int side)
{
    if (side == white)
    {
        return pieces[wp] | pieces[wr] | pieces[wn] | pieces[wb] | pieces[wq] | pieces[wk];
    }
    if (side == black)
    {
        return pieces[bp] | pieces[br] | pieces[bn] | pieces[bb] | pieces[bq] | pieces[bk];
    }

    return pieces[wp] | pieces[wr] | pieces[wn] | pieces[wb] | pieces[wq] | pieces[wk] |
           pieces[bp] | pieces[br] | pieces[bn] | pieces[bb] | pieces[bq] | pieces[bk];
}

// ==========================================================
// ============ function to initialize the makes ============
// ==========================================================

void Bitboard::initMasks()
{
    init_fileMasks();
    init_rankMasks();
    init_isolatedPawnMask();
    init_passedPawnMask();
}

void Bitboard::init_fileMasks()
{
    for (int file = fileA; file <= fileH; ++file)
    {
        for (int rank = rank1; rank <= rank8; ++rank)
        {
            fileMasks[file] |= (1ULL << file) << (8 * rank);
        }
    }
}

void Bitboard::init_rankMasks()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        rankMasks[rank] |= 255ULL << (rank * 8);
    }
}
void Bitboard::init_isolatedPawnMask()
{
    for (int sq = 0; sq < 64; sq++)
    {
        int file = fileOf(sq64To120[sq]);
        if (file > fileA)
        {
            isolatedPawnMask[sq] |= fileMasks[file - 1];
        }
        if (file < fileH)
        {
            isolatedPawnMask[sq] |= fileMasks[file + 1];
        }
    }
}

void Bitboard::init_passedPawnMask()
{
    for (int i = 0; i < 2; ++i)
    {

        for (int sq = 0; sq < 64; sq++)
        {
            int file = fileOf(sq64To120[sq]);
            int rank = rankOf(sq64To120[sq]);

            passedPawnMask[i][sq] |= fileMasks[file];
            if (file > fileA)
            {
                passedPawnMask[i][sq] |= fileMasks[file - 1];
            }
            if (file < fileH)
            {
                passedPawnMask[i][sq] |= fileMasks[file + 1];
            }

            if (i == white)
            {
                while (rank >= rank1)
                {
                    passedPawnMask[i][sq] &= ~rankMasks[rank--];
                }
            }
            else
            {
                while (rank <= rank8)
                {
                    passedPawnMask[i][sq] &= ~rankMasks[rank++];
                }
            }
        }
    }
}

// ==========================================================
// ============= function to initialize Blockers ============
// ==========================================================

void Bitboard::generateBlockersFor(U64 (&attackMasks)[64], std::vector<U64> (&storage)[64])
{
    for (int i = 0; i < 64; ++i)
    {
        U64 bit = attackMasks[i];
        generateBlockerBoards(bit, storage[i]);
    }
}

void Bitboard::generateBlockerBoards(U64 bitBoard, std::vector<U64> &blockerBoards)
{
    std::vector<int> setBitIndices;
    while (bitBoard)
    {
        setBitIndices.push_back(__builtin_ctzll(bitBoard)); // store the least significant set bit
        bitBoard &= (bitBoard - 1);                         // Remove the least significant set bit
    }

    int setBitCount = setBitIndices.size();
    int totalCombinations = 1 << setBitCount; // 2 ^ n
    blockerBoards.resize(totalCombinations, 0ULL);

    for (int i = 0; i < totalCombinations; ++i)
    {
        // for each bit of i, shift bits correcponding to bitBoard's setBitIndices
        for (int j = 0; j < setBitCount; ++j)
        {
            U64 bit = (i >> j) & 1;
            blockerBoards[i] |= bit << setBitIndices[j];
        }
    }
}

// ==========================================================
// ============== functions that needs board ref =============
// ==========================================================

void Bitboard::initBoard(Board *board)
{
    this->board = board;
    init_pieces();
    init_attackMasks();

    init_rookLookupTable();
    init_bishopLookupTable();
}

void Bitboard::init_attackMasks()
{
    init_pawnAttacks();
    init_kingAttacks();
    init_knighAttack();
    init_rookAttacks();
    init_bishopAttacks();
}

void Bitboard::init_pieces()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            int piece = board->pieces[sq];
            if (piece)
            {
                setBit(piece, sq120To64[sq]);
            }
        }
    }
}

// ============== functions to initialize LOOKUP TABLES =============

void Bitboard::init_rookLookupTable()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        U64 magic = rookMagics[sq];
        for (U64 blocker : rookBlockers[sq])
        {
            int index = (blocker * magic) >> (64 - 12);
            rookLookupTable[sq][index] = legalMoveBitboardFromBlockers(sq, blocker, true);
        }
    }
}

void Bitboard::init_bishopLookupTable()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        U64 magic = bishopMagics[sq];
        for (U64 blocker : bishopBlockers[sq])
        {
            int index = (blocker * magic) >> (64 - 12);
            bishopLookupTable[sq][index] = legalMoveBitboardFromBlockers(sq, blocker, false);
        }
    }
}

U64 Bitboard::legalMoveBitboardFromBlockers(int sq, U64 blockerBitboard, bool ortho)
{
    U64 bitboard = 0ULL;
    const int *directions = ortho ? rookDirections : bishopDirections;

    for (int i = 0; i < 4; ++i)
    {
        int targetSq = sq64To120[sq] + directions[i];
        while (board->pieces[targetSq] != offBoard)
        {
            setBit(bitboard, sq120To64[targetSq]);
            if (blockerBitboard & (1ULL << sq120To64[targetSq]))
            {
                break;
            }
            targetSq += directions[i];
        }
    }
    return bitboard;
}

// ============== functions to initialize attacker =============

void Bitboard::init_pawnAttacks()
{
    for (int rank = rank2; rank <= rank7; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            U64 bitBoard = 0ULL;
            int sq = fileRank2Sq(file, rank);
            if (board->pieces[sq + 11] != offBoard)
            {
                setBit(bitBoard, sq120To64[sq + 11]);
            }
            if (board->pieces[sq + 9] != offBoard)
            {
                setBit(bitBoard, sq120To64[sq + 9]);
            }
            pawnAttacks[white][sq120To64[sq]] = bitBoard;
        }
    }
    for (int rank = rank7; rank >= rank2; --rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            U64 bitBoard = 0ULL;
            int sq = fileRank2Sq(file, rank);
            if (board->pieces[sq - 11] != offBoard)
            {
                setBit(bitBoard, sq120To64[sq - 11]);
            }
            if (board->pieces[sq - 9] != offBoard)
            {
                setBit(bitBoard, sq120To64[sq - 9]);
            }
            pawnAttacks[black][sq120To64[sq]] = bitBoard;
        }
    }
}
void Bitboard::init_kingAttacks()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 8; ++i)
            {
                int direction = kingDirections[i];
                if (board->pieces[sq + direction] != offBoard)
                {
                    setBit(bitBoard, sq120To64[sq + direction]);
                }
            }

            kingAttacks[sq120To64[sq]] = bitBoard;
        }
    }
}
void Bitboard::init_knighAttack()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 8; ++i)
            {
                int direction = knightDirections[i];
                if (board->pieces[sq + direction] != offBoard)
                {
                    setBit(bitBoard, sq120To64[sq + direction]);
                }
            }

            knightAttacks[sq120To64[sq]] = bitBoard;
        }
    }
}
void Bitboard::init_rookAttacks()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 4; ++i)
            {
                int direction = rookDirections[i];
                int targetSq = sq + direction;
                while (board->pieces[targetSq] != offBoard)
                {
                    setBit(bitBoard, sq120To64[targetSq]);
                    // erase the edge bits
                    if (rank != rank8)
                        bitBoard &= ~rankMasks[rank8];
                    if (rank != rank1)
                        bitBoard &= ~rankMasks[rank1];
                    if (file != fileA)
                        bitBoard &= ~fileMasks[fileA];
                    if (file != fileH)
                        bitBoard &= ~fileMasks[fileH];
                    targetSq += direction;
                }
            }

            rookAttacks[sq120To64[sq]] = bitBoard;
        }
    }
}
void Bitboard::init_bishopAttacks()
{
    for (int rank = rank1; rank <= rank8; ++rank)
    {
        for (int file = fileA; file <= fileH; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 4; ++i)
            {
                int direction = bishopDirections[i];
                int targetSq = sq + direction;
                while (board->pieces[targetSq] != offBoard)
                {
                    setBit(bitBoard, sq120To64[targetSq]);
                    if (rank != rank8)
                        bitBoard &= ~rankMasks[rank8];
                    if (rank != rank1)
                        bitBoard &= ~rankMasks[rank1];
                    if (file != fileA)
                        bitBoard &= ~fileMasks[fileA];
                    if (file != fileH)
                        bitBoard &= ~fileMasks[fileH];
                    targetSq += direction;
                }
            }

            bishopAttacks[sq120To64[sq]] = bitBoard;
        }
    }
}

// ==========================================================
// =============== pre computed magic numbers ===============
// ==========================================================

U64 bishopMagics[64] = {
    2253999440997636ULL,
    4903012653716226048ULL,
    864979750796656656ULL,
    9223689804573606017ULL,
    40533607827128336ULL,
    299577372120728576ULL,
    36319073592672642ULL,
    1193489111416705024ULL,
    4972541375373120016ULL,
    22946000217964576ULL,
    9017369804408064ULL,
    3388696984813568ULL,
    633602500984850ULL,
    18014982910509200ULL,
    1747683700970094752ULL,
    150997106696ULL,
    145140010793862272ULL,
    2458965397656174612ULL,
    29863011310239776ULL,
    8797192194216ULL,
    4670759777813568ULL,
    35253162883232ULL,
    234188280696938656ULL,
    4611880121018812945ULL,
    1157425173266301226ULL,
    622350107178172992ULL,
    2676266277592369729ULL,
    9250683922902554882ULL,
    9223512860276032512ULL,
    1161999141891559424ULL,
    9225677714898289537ULL,
    6197257669300996096ULL,
    585503204652123136ULL,
    624038862544443456ULL,
    281613086916618ULL,
    3379903173004288ULL,
    71606768502992ULL,
    13835077846525296706ULL,
    60948414703601026ULL,
    2960160785041410ULL,
    1190711721468114945ULL,
    232203117691904ULL,
    4646048265597371456ULL,
    6920132971833983234ULL,
    1405739018628378688ULL,
    17731810263041ULL,
    5770289833626896641ULL,
    563581347168260ULL,
    9873263690475642898ULL,
    4611686646584181764ULL,
    1137445337841741ULL,
    4899992261091393536ULL,
    9583351950742017ULL,
    2307012889862610973ULL,
    297259565815443456ULL,
    19826995167362ULL,
    5188166566236659712ULL,
    18300824511054208ULL,
    10376294709798123520ULL,
    9029279748760768ULL,
    9512447113385230912ULL,
    140738830565512ULL,
    2307540725094547536ULL,
    2883579206178964483ULL,
};
U64 rookMagics[64] = {
    108086959066380448ULL,
    9007336962131968ULL,
    13839579385616926736ULL,
    2310346695413203024ULL,
    37154971937934368ULL,
    2310347175777075266ULL,
    9270695171966632192ULL,
    648541994433397504ULL,
    9223389629485498368ULL,
    292804413244833800ULL,
    13228534924356ULL,
    9223416051750930564ULL,
    423708386069121024ULL,
    9223547960964808713ULL,
    580966688661717504ULL,
    77194669145985088ULL,
    36028968855404576ULL,
    37383530719360ULL,
    76561743832162304ULL,
    4611791597347079232ULL,
    4629735679759221012ULL,
    563035855929602ULL,
    1126467379528008ULL,
    324498868114884752ULL,
    2315202052466147872ULL,
    4611725688087777296ULL,
    2814784328433793ULL,
    1441332200800813570ULL,
    9223514973504816128ULL,
    9038294851649544ULL,
    4616263560882963200ULL,
    31669233551149124ULL,
    2594214397734289984ULL,
    4611897133258768968ULL,
    2355382622305130496ULL,
    40533498574012484ULL,
    2251825585586433ULL,
    2317664961459916832ULL,
    73482665268871689ULL,
    2936496526060945440ULL,
    1162069993523775488ULL,
    141838073790472ULL,
    576883171061665844ULL,
    2452212201562310666ULL,
    2199041736724ULL,
    148763923307316353ULL,
    27030397087006724ULL,
    9007487051276304ULL,
    4616190992530358284ULL,
    1805970939508187392ULL,
    579022633732669472ULL,
    10964294838882935297ULL,
    4828003974864974976ULL,
    40816070730195104ULL,
    9026990487119512ULL,
    576481644251185280ULL,
    9314642635405000834ULL,
    5348096498864514ULL,
    4611827039452269586ULL,
    2310384043809572930ULL,
    4755802350109720610ULL,
    360332020515111170ULL,
    5841169267563171908ULL,
    578994029245104193ULL,
};