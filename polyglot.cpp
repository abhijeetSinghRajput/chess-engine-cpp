#include "polyglot.hpp"
#include "board.hpp"
#include <fstream>
#include <iostream>

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
    if (board->side == white)
    {
        if (board->pieces[ep - 9] == wp || board->pieces[ep - 11] == wp)
        {
            return true;
        }
    }
    else
    {
        if (board->pieces[ep + 9] == bp || board->pieces[ep + 11] == bp)
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
        if (piece != empty)
        {
            polyKey ^= piecePolyKeys[polyPieces[piece]][sq];
        }
    }

    if (board->castlePermission & castle_K) polyKey ^= castlePolyKeys[0];
    if (board->castlePermission & castle_Q) polyKey ^= castlePolyKeys[1];
    if (board->castlePermission & castle_k) polyKey ^= castlePolyKeys[2];
    if (board->castlePermission & castle_q) polyKey ^= castlePolyKeys[3];

    if (board->side == white) polyKey ^= sidePolyKey;
    if (board->enPassantSq != noSq && hasPawnToCapture())
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
    // open the .bin file
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    // check if it succefully opened
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
    
    char buffer[16];
    U64 key;
    polyEntry entry;

    while (file.read(buffer, sizeof(buffer))) {
        std::memcpy(&key, buffer, sizeof(key));
        std::memcpy(&entry, buffer + sizeof(key), sizeof(polyEntry));
        openingBook[endian_swap_u64(key)].push_back(entry);
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
    // Extract components from the move using bit manipulation
    int toFile = (move & 0x7);                 // bits 0,1,2
    int toRow = ((move >> 3) & 0x7);           // bits 3,4,5
    int fromFile = ((move >> 6) & 0x7);        // bits 6,7,8
    int fromRow = ((move >> 9) & 0x7);         // bits 9,10,11
    int promotionPiece = ((move >> 12) & 0x7); // bits 12,13,14

    // Convert the extracted bits into meaningful chess move components
    char fromFileChar = 'a' + fromFile;
    char toFileChar = 'a' + toFile;
    int fromRank = fromRow + 1;
    int toRank = toRow + 1;

    std::string promotion;
    switch (promotionPiece)
    {
    case 1:
        promotion = "N";
        break;
    case 2:
        promotion = "B";
        break;
    case 3:
        promotion = "R";
        break;
    case 4:
        promotion = "Q";
        break;
    default:
        promotion = "";
        break;
    }

    // Construct and return the human-readable move notation
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