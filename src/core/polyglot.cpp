//polyglot.cpp
#include "core/polyglot.hpp"
#include "core/board.hpp"
#include "core/utils.hpp"
#include <fstream>
#include <iostream>
#include <ctime>


// endian_swap_u32 visual representation
//    11111111 00000000 00000000 00000000       00000000 00000000 00000000 11111111
//    00000000 11111111 00000000 00000000       00000000 00000000 11111111 00000000
//    00000000 00000000 11111111 00000000       00000000 11111111 00000000 00000000
//    00000000 00000000 00000000 11111111       11111111 00000000 00000000 00000000

// swap the byte order of a 16-bit unsigned integer
uint16_t endian_swap_u16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

// swap the byte order of a 32-bit unsigned integer
uint32_t endian_swap_u32(uint32_t value)
{
    return ((value >> 24) & 0x000000ff) |
           ((value >> 8) & 0x0000ff00) |
           ((value << 8) & 0x00ff0000) |
           ((value << 24) & 0xff000000);
}

// swap the byte order of a 64-bit unsigned integer
uint64_t endian_swap_u64(uint64_t value)
{
    return ((value >> 56) & 0x00000000000000ff) |
           ((value >> 40) & 0x000000000000ff00) |
           ((value >> 24) & 0x0000000000ff0000) |
           ((value >> 8) & 0x00000000ff000000) |
           ((value << 8) & 0x000000ff00000000) |
           ((value << 24) & 0x0000ff0000000000) |
           ((value << 40) & 0x00ff000000000000) |
           ((value << 56) & 0xff00000000000000);
}

bool hasPawnToCapture()
{
    int ep = board->enPassantSq;
    if (board->side == WHITE)
    {
        if (board->pieces[ep - 9] == PIECE_WP || board->pieces[ep - 11] == PIECE_WP)
        {
            return true;
        }
    }
    else
    {
        if (board->pieces[ep + 9] == PIECE_BP || board->pieces[ep + 11] == PIECE_BP)
        {
            return true;
        }
    }
    return false;
}

U64 getPolyKey()
{
    U64 polyKey = 0ULL;
    int piece;

    for (int sq = 0; sq < 64; ++sq)
    {
        piece = board->pieces[sq64To120[sq]];
        if (piece != PIECE_EMPTY)
        {
            polyKey ^= piecePolyKeys[polyPieces[piece]][sq];
        }
    }

    if (board->castlePermission & CASTLE_WK) polyKey ^= castlePolyKeys[0];
    if (board->castlePermission & CASTLE_WQ) polyKey ^= castlePolyKeys[1];
    if (board->castlePermission & CASTLE_BK) polyKey ^= castlePolyKeys[2];
    if (board->castlePermission & CASTLE_BQ) polyKey ^= castlePolyKeys[3];

    if (board->side == WHITE) polyKey ^= sidePolyKey;
    if (board->enPassantSq != SQ_NONE && hasPawnToCapture())
    {
        printf(" 1 ");
        polyKey ^= enPassantPolyKeys[fileOf(board->enPassantSq)];
    }

    return polyKey;
}

#include <cstring>
std::unordered_map<U64, std::vector<polyEntry>> openingBook;

void loadPolyBook(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
    {
        std::cerr << "\033[30mgetting error while reading\033[0m" << std::endl;
        return;
    }

    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 16)
    {
        std::cerr << "\033[30mno entries found\033[0m" << std::endl;
        return;
    }

    size_t entriesSize = fileSize / 16;
    std::cout << entriesSize << " entries found in file" << std::endl;

    unsigned char buffer[16];
    while (file.read(reinterpret_cast<char*>(buffer), sizeof(buffer)))
    {
        // Manually decode each big-endian field by byte position —
        // no dependency on struct layout, padding, or host endianness.
        U64 key = 0;
        for (int i = 0; i < 8; ++i)
            key = (key << 8) | buffer[i];

        polyEntry entry{};
        entry.move   = (static_cast<uint16_t>(buffer[8])  << 8)  | buffer[9];
        entry.weight = (static_cast<uint16_t>(buffer[10]) << 8)  | buffer[11];
        entry.learn  = (static_cast<uint32_t>(buffer[12]) << 24) |
                       (static_cast<uint32_t>(buffer[13]) << 16) |
                       (static_cast<uint32_t>(buffer[14]) << 8)  |
                        static_cast<uint32_t>(buffer[15]);

        openingBook[key].push_back(entry);
    }

    std::cout << openingBook.size() << " position loaded in Opening Book" << std::endl;
    file.close();
}

void readBook()
{
    if (openingBook.empty())
    {
        std::cout << "book is empty" << std::endl;
        return;
    }
    U64 currPolyKey = getPolyKey();
    if(openingBook.find(currPolyKey) == openingBook.end()){
        std::cout << "no move found for this position" << std::endl;
        return;
    }
    printf("\nbook key: %llx\n", currPolyKey);
    for (auto &entry : openingBook[currPolyKey])
    {
        printf("move: %-5s  weight: %-5d  learn: %-10d\n",
            extractPolyMove(endian_swap_u16(entry.move)).c_str(),
            endian_swap_u16(entry.weight),
            endian_swap_u32(entry.learn)
        );
    }
    std::cout<<std::endl;
}
int getRandBookMove(){
    std::vector<std::string> moves;
    U64 currPolyKey = getPolyKey();
    if(!openingBook.empty() && openingBook.find(currPolyKey) != openingBook.end()){
        for(auto &entry : openingBook[currPolyKey]){
            moves.push_back(extractPolyMove(endian_swap_u16(entry.move)));
        }
    }
    if(moves.empty()) return 0;
    // Seed the random number generator
    std::srand(std::time(0));
    int randIndex = std::rand() % moves.size();

    int move = parseMove(moves[randIndex]);
    return move;
}

// MOVE

// bits                meaning                   promotion pieces
// ===================================           ================
// 0,1,2               to file                   none       0
// 3,4,5               to row                    knight     1
// 6,7,8               from file                 bishop     2
// 9,10,11             from row                  rook       3
// 12,13,14            promotion piece           queen      4

std::string extractPolyMove(uint16_t move)
{
    int toFile   = (move & 0x7);
    int toRow    = ((move >> 3) & 0x7);
    int fromFile = ((move >> 6) & 0x7);
    int fromRow  = ((move >> 9) & 0x7);
    int promotionPiece = ((move >> 12) & 0x7);

    char fromFileChar = 'a' + fromFile;
    char toFileChar   = 'a' + toFile;
    int fromRank = fromRow + 1;
    int toRank   = toRow + 1;

    // Fix castling: polyglot uses king-to-rook, engine uses king-to-dest
    // white short: e1h1 -> e1g1
    // white long:  e1a1 -> e1c1
    // black short: e8h8 -> e8g8
    // black long:  e8a8 -> e8c8
    if (fromFileChar == 'e' && toFileChar == 'h' && fromFile == 4)
        toFileChar = 'g';
    else if (fromFileChar == 'e' && toFileChar == 'a' && fromFile == 4)
        toFileChar = 'c';

    std::string promotion;
    switch (promotionPiece)
    {
        case 1: promotion = "n"; break;
        case 2: promotion = "b"; break;
        case 3: promotion = "r"; break;
        case 4: promotion = "q"; break;
        default: promotion = "";
    }

    std::string result = "";
    result += fromFileChar;
    result += std::to_string(fromRank);
    result += toFileChar;
    result += std::to_string(toRank);
    result += promotion;
    return result;
}
// Castling moves
// white short      e1h1
// white long       e1a1
// black short      e8h8
// black long       e8a8