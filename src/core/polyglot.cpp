#include "core/polyglot.hpp"
#include "core/board.hpp"
#include "core/utils.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <random>

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

// Is there actually a pawn of the side to move that could legally execute
// the en passant capture? (Polyglot only XORs the en-passant key in when
// this is true.) Offsets are for a flat 0-63 board (a1=0 .. h8=63).
// File-edge checks prevent wrapping onto the opposite side of the board.
bool hasPawnToCapture()
{
    int ep = board->enPassantSq;
    int f = fileOf(ep);

    if (board->side == WHITE)
    {
        if (f != 0 && board->pieces[ep - 9] == PIECE_WP) return true; // capture from the file to the left
        if (f != 7 && board->pieces[ep - 7] == PIECE_WP) return true; // capture from the file to the right
    }
    else
    {
        if (f != 0 && board->pieces[ep + 7] == PIECE_BP) return true; // capture from the file to the left
        if (f != 7 && board->pieces[ep + 9] == PIECE_BP) return true; // capture from the file to the right
    }
    return false;
}

U64 getPolyKey()
{
    U64 polyKey = 0ULL;
    int piece;

    for (int sq = 0; sq < 64; ++sq)
    {
        piece = board->pieces[sq];
        if (piece != PIECE_EMPTY)
        {
            polyKey ^= piecePolyKeys[polyPieces[piece]][sq];
        }
    }

    if (board->castlePermission & castle_K) polyKey ^= castlePolyKeys[0];
    if (board->castlePermission & castle_Q) polyKey ^= castlePolyKeys[1];
    if (board->castlePermission & castle_k) polyKey ^= castlePolyKeys[2];
    if (board->castlePermission & castle_q) polyKey ^= castlePolyKeys[3];

    if (board->side == WHITE) polyKey ^= sidePolyKey;
    if (board->enPassantSq != SQ_NONE && hasPawnToCapture())
    {
        polyKey ^= enPassantPolyKeys[fileOf(board->enPassantSq)];
    }

    return polyKey;
}

std::unordered_map<U64, std::vector<polyEntry>> openingBook;

void loadPolyBook(const std::string &path)
{
    // open the .bin file
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    // check if it succefully opened
    if (!file)
    {
        std::cerr << "\033[31mgetting error while reading\033[0m" << std::endl;
        return;
    }

    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 16)
    {
        std::cerr << "\033[31mno entries found\033[0m" << std::endl;
        return;
    }

    if (fileSize % 16 != 0)
    {
        std::cerr << "\033[31mwarning: file size is not a multiple of 16 bytes, book may be truncated/corrupt\033[0m" << std::endl;
    }

    size_t entriesSize = fileSize / 16;
    std::cout << entriesSize << " entries found in file" << std::endl;

    char buffer[16];
    U64 key;
    polyEntry entry;

    // Polyglot books are stored big-endian. Byte-swap once here, at load
    // time, so every other site in the engine can treat openingBook as
    // already being in host byte order and never has to swap again.
    while (file.read(buffer, sizeof(buffer)))
    {
        std::memcpy(&key, buffer, sizeof(key));
        std::memcpy(&entry, buffer + sizeof(key), sizeof(polyEntry));

        key = endian_swap_u64(key);
        entry.move = endian_swap_u16(entry.move);
        entry.weight = endian_swap_u16(entry.weight);
        entry.learn = endian_swap_u32(entry.learn);

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
    auto it = openingBook.find(currPolyKey);
    if (it == openingBook.end())
    {
        std::cout << "no move found for this position" << std::endl;
        return;
    }

    printf("\nbook key: %llx\n", currPolyKey);
    // Entries are already in host byte order (swapped once in loadPolyBook),
    // so no endian_swap calls are needed here.
    for (auto &entry : it->second)
    {
        printf("move: %-5s  weight: %-5d  learn: %-10d\n",
            extractPolyMove(entry.move).c_str(),
            entry.weight,
            entry.learn
        );
    }
    std::cout << std::endl;
}

int getRandBookMove()
{
    if (openingBook.empty())
    {
        return 0;
    }

    U64 currPolyKey = getPolyKey();
    auto it = openingBook.find(currPolyKey);
    if (it == openingBook.end())
    {
        return 0;
    }

    std::vector<std::string> moves;
    // Entries are already in host byte order (swapped once in loadPolyBook).
    for (auto &entry : it->second)
    {
        moves.push_back(extractPolyMove(entry.move));
    }

    if (moves.empty()) return 0;

    // Seeded once (function-local static, initialized exactly once in
    // C++11+), instead of reseeding with time(0) on every call — reseeding
    // every call gave identical results for calls within the same second.
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
    size_t randIndex = dist(rng);

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
    int toFile = (move & 0x7);
    int toRow = ((move >> 3) & 0x7);
    int fromFile = ((move >> 6) & 0x7);
    int fromRow = ((move >> 9) & 0x7);
    int promotionPiece = ((move >> 12) & 0x7);

    char fromFileChar = 'a' + fromFile;
    char toFileChar = 'a' + toFile;
    int fromRank = fromRow + 1;
    int toRank = toRow + 1;

    // Convert to standard king-destination UCI notation so it matches what
    // e1h1 → e1g1
    // e1a1 → e1c1
    // e8h8 → e8g8
    // e8a8 → e8c8
    int fromSq = (fromRank - 1) * 8 + (fromFile);
    int fromPiece = board->pieces[fromSq];
    bool isKing = (fromPiece == PIECE_WK || fromPiece == PIECE_BK);
    
    if(isKing){
        if      (fromFileChar == 'e' && fromRank == 1 && toFileChar == 'h' && toRank == 1) { toFileChar = 'g'; }
        else if (fromFileChar == 'e' && fromRank == 1 && toFileChar == 'a' && toRank == 1) { toFileChar = 'c'; }
        else if (fromFileChar == 'e' && fromRank == 8 && toFileChar == 'h' && toRank == 8) { toFileChar = 'g'; }
        else if (fromFileChar == 'e' && fromRank == 8 && toFileChar == 'a' && toRank == 8) { toFileChar = 'c'; }
    }

    std::string promotion;
    switch (promotionPiece)
    {
        case 1: promotion = "n"; break;
        case 2: promotion = "b"; break;
        case 3: promotion = "r"; break;
        case 4: promotion = "q"; break;
        default: promotion = ""; break;
    }

    std::string result;
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