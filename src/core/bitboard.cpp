#include "core/defs.hpp"
#include "core/bitboard.hpp"
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
    occupied[0] = 0ULL;
    occupied[1] = 0ULL;

    initMasks();
    init_attackMasks();

    generateBlockersFor(rookAttacks, rookBlockers);
    generateBlockersFor(bishopAttacks, bishopBlockers);
}

// ==========================================================
// ===================== helper functions ===================
// ==========================================================

void Bitboard::clearBit(int piece, int sq)
{
    U64 bit = 1ULL << sq;
    pieces[piece] &= ~bit;
    occupied[PIECE_COLOR[piece]] &= ~bit;
}
void Bitboard::setBit(U64 &bitBoard, int sq)
{
    bitBoard |= (1ULL << sq);
}
void Bitboard::setBit(int piece, int sq)
{
    U64 bit = 1ULL << sq;
    pieces[piece] |= bit;
    occupied[PIECE_COLOR[piece]] |= bit;
}
int Bitboard::getPieceCount(int piece)
{
    return __builtin_popcountll(pieces[piece]);
}
void Bitboard::movePiece(int piece, int fromSq, int toSq)
{
    // Dropped the "is this piece actually on fromSq" guard that used to
    // sit here: it's a defensive check against a state move generation
    // should never produce, and this function runs on every make/unmake
    // of every move in the tree. Trust the invariant; if it's ever
    // violated, perft will tell you loudly instead of this silently
    // no-op'ing.
    U64 mask = (1ULL << fromSq) | (1ULL << toSq);
    pieces[piece] ^= mask;
    occupied[PIECE_COLOR[piece]] ^= mask;
}

void Bitboard::print(int piece)
{
    U64 bitBoard = pieces[piece];
    printf("bitMask for piece \033[32m%s\033[0m\n", PIECE_NAME[piece]);
    std::cout << "0x" << std::hex << bitboard << std::dec << std::endl;
    for (int rank = RANK_8; rank >= RANK_1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int sq = rank * 8 + file;
            std::cout << ((bitBoard & (1ULL << sq)) ? "\033[32m1\033[0m" : "\033[90m.\033[0m") << " ";
        }
        std::cout << std::endl;
    }
    std::cout<<"    ";
    for (char file = 'a'; file <= 'h'; ++file)
    {
        std::cout << file << " ";
    }
    std::cout << std::endl
              << std::endl;
}
void Bitboard::print(U64 bitBoard)
{
    std::cout << "0x" << std::hex << bitBoard << std::dec << std::endl;
    for (int rank = RANK_8; rank >= RANK_1; --rank)
    {
        std::cout << rank + 1 << "   ";
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int sq = rank * 8 + file;
            std::cout << ((bitBoard & (1ULL << sq)) ? "\033[32m1\033[0m" : "\033[90m.\033[0m") << " ";
        }
        std::cout << std::endl;
    }
    std::cout<<"    ";
    for (char file = 'a'; file <= 'h'; ++file)
    {
        std::cout << file << " ";
    }
    std::cout << std::endl
              << std::endl;
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
    if (side == WHITE)
    {
        return occupied[WHITE];
    }
    if (side == BLACK)
    {
        return occupied[BLACK];
    }
    return occupied[WHITE] | occupied[BLACK];
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
    for (int file = FILE_A; file <= FILE_H; ++file)
    {
        for (int rank = RANK_1; rank <= RANK_8; ++rank)
        {
            fileMasks[file] |= (1ULL << file) << (8 * rank);
        }
    }
}

void Bitboard::init_rankMasks()
{
    for (int rank = RANK_1; rank <= RANK_8; ++rank)
    {
        rankMasks[rank] |= 255ULL << (rank * 8);
    }
}
void Bitboard::init_isolatedPawnMask()
{
    for (int sq = 0; sq < 64; sq++)
    {
        int file = fileOf(sq);
        if (file > FILE_A)
        {
            isolatedPawnMask[sq] |= fileMasks[file - 1];
        }
        if (file < FILE_H)
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
            int file = fileOf(sq);
            int rank = rankOf(sq);

            passedPawnMask[i][sq] |= fileMasks[file];
            if (file > FILE_A)
            {
                passedPawnMask[i][sq] |= fileMasks[file - 1];
            }
            if (file < FILE_H)
            {
                passedPawnMask[i][sq] |= fileMasks[file + 1];
            }
            if (i == WHITE)
            {
                while (rank >= RANK_1)
                {
                    passedPawnMask[i][sq] &= ~rankMasks[rank--];
                }
            }
            else
            {
                while (rank <= RANK_8)
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

    init_rookLookupTable();
    init_bishopLookupTable();
}

void Bitboard::init_attackMasks()
{
    for (int sq = SQ_A1; sq <= SQ_H8; sq++)
    {
        Board::pieces[sq] = PIECE_EMPTY;
    }

    init_pawnAttacks();
    init_kingAttacks();
    init_knighAttack();
    init_rookAttacks();
    init_bishopAttacks();
}

void Bitboard::init_pieces()
{
    for (int i = 0; i < 13; ++i)
    {
        pieces[i] = 0ULL;
    }
    occupied[0] = 0ULL;
    occupied[1] = 0ULL;

    for (int rank = RANK_1; rank <= RANK_8; ++rank)
    {
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            int piece = board->pieces[sq];
            if (piece)
            {
                setBit(piece, sq);
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
    const int *df = ortho ? ROOK_DF : BISHOP_DF;
    const int *dr = ortho ? ROOK_DR : BISHOP_DR;
    int f0 = fileOf(sq), r0 = rankOf(sq);

    for (int i = 0; i < 4; ++i)
    {
        int f = f0 + df[i], r = r0 + dr[i];
        while (f >= 0 && f < 8 && r >= 0 && r < 8)
        {
            int targetSq = fileRank2Sq(f, r);
            setBit(bitboard, targetSq);
            if (blockerBitboard & (1ULL << targetSq))
                break;
            f += df[i];
            r += dr[i];
        }
    }
    return bitboard;
}

// ============== functions to initialize attacker =============

void Bitboard::init_pawnAttacks()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int f = fileOf(sq), r = rankOf(sq);
        U64 bb = 0ULL;
        if (f > FILE_A && r < RANK_8) setBit(bb, fileRank2Sq(f - 1, r + 1));
        if (f < FILE_H && r < RANK_8) setBit(bb, fileRank2Sq(f + 1, r + 1));
        pawnAttacks[WHITE][sq] = bb;

        bb = 0ULL;
        if (f > FILE_A && r > RANK_1) setBit(bb, fileRank2Sq(f - 1, r - 1));
        if (f < FILE_H && r > RANK_1) setBit(bb, fileRank2Sq(f + 1, r - 1));
        pawnAttacks[BLACK][sq] = bb;
    }
}

void Bitboard::init_kingAttacks()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int f = fileOf(sq), r = rankOf(sq);
        U64 bb = 0ULL;
        for (int i = 0; i < 8; ++i)
        {
            int nf = f + KING_DF[i], nr = r + KING_DR[i];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8)
                setBit(bb, fileRank2Sq(nf, nr));
        }
        kingAttacks[sq] = bb;
    }
}

void Bitboard::init_knighAttack()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        int f = fileOf(sq), r = rankOf(sq);
        U64 bb = 0ULL;
        for (int i = 0; i < 8; ++i)
        {
            int nf = f + KNIGHT_DF[i], nr = r + KNIGHT_DR[i];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8)
                setBit(bb, fileRank2Sq(nf, nr));
        }
        knightAttacks[sq] = bb;
    }
}

void Bitboard::init_rookAttacks()
{
    for (int rank = RANK_1; rank <= RANK_8; ++rank)
    {
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 4; ++i)
            {
                int f = file + ROOK_DF[i], r = rank + ROOK_DR[i];
                while (f >= 0 && f < 8 && r >= 0 && r < 8)
                {
                    setBit(bitBoard, fileRank2Sq(f, r));
                    if (rank != RANK_8) bitBoard &= ~rankMasks[RANK_8];
                    if (rank != RANK_1) bitBoard &= ~rankMasks[RANK_1];
                    if (file != FILE_A) bitBoard &= ~fileMasks[FILE_A];
                    if (file != FILE_H) bitBoard &= ~fileMasks[FILE_H];
                    f += ROOK_DF[i];
                    r += ROOK_DR[i];
                }
            }
            rookAttacks[sq] = bitBoard;
        }
    }
}

void Bitboard::init_bishopAttacks()
{
    for (int rank = RANK_1; rank <= RANK_8; ++rank)
    {
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int sq = fileRank2Sq(file, rank);
            U64 bitBoard = 0ULL;

            for (int i = 0; i < 4; ++i)
            {
                int f = file + BISHOP_DF[i], r = rank + BISHOP_DR[i];
                while (f >= 0 && f < 8 && r >= 0 && r < 8)
                {
                    setBit(bitBoard, fileRank2Sq(f, r));
                    if (rank != RANK_8) bitBoard &= ~rankMasks[RANK_8];
                    if (rank != RANK_1) bitBoard &= ~rankMasks[RANK_1];
                    if (file != FILE_A) bitBoard &= ~fileMasks[FILE_A];
                    if (file != FILE_H) bitBoard &= ~fileMasks[FILE_H];
                    f += BISHOP_DF[i];
                    r += BISHOP_DR[i];
                }
            }
            bishopAttacks[sq] = bitBoard;
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