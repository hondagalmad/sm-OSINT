#ifndef AES_HPP
#define AES_HPP
#include<fstream>
#include <iostream>
#include <iomanip>
#include <array>
#include <cstdint>

// AES S-box
uint8_t sbox[256] ;

bool loadSBox(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return false;
    }
    std::string value;
    int i = 0;
    while (file >> value) {
        if (i >= 256) {
            std::cerr << "Error: s-box.txt contains more than 256 values."
                 << std::endl;
            return false;
        }
        unsigned int x;
        std::stringstream ss;
        ss << std::hex << value;
        ss >> x;
        if (ss.fail() || x > 0xFF) {
            std::cerr << "Error: Invalid S-box value: " << value << std::endl;
            return false;
        }
        sbox[i++] = static_cast<uint8_t>(x);
    }

    if (i != 256) {
        std::cerr << "Error: s-box.txt must contain exactly 256 values."
             << std::endl;
        return false;
    }

    return true;
}
// SubByte
void subByte(uint8_t arr[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            arr[i][j] = sbox[arr[i][j]];
        }
    }
}

// shiftRows
void shiftRows(uint8_t arr[4][4])
{
    uint8_t curr[4][4];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            curr[i][j] = arr[i][j];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            arr[i][j] = curr[i][(i + j) % 4];
        }
    }
}

// Galois Field multiplication
uint8_t gmul(uint8_t a, uint8_t b)
{
    uint8_t result = 0;

    for (int i = 0; i < 8; i++)
    {
        if (b & 1)
            result ^= a;

        bool highBit = a & 0x80;

        a <<= 1;

        if (highBit)
            a ^= 0x1B;

        b >>= 1;
    }

    return result;
}


// MixColumns
void mixColumns(uint8_t arr[4][4])
{
    for(int j = 0;j < 4;j++)
    {
        uint8_t a = arr[0][j];
        uint8_t b = arr[1][j];
        uint8_t c = arr[2][j];
        uint8_t d = arr[3][j];
    

        arr[0][j] = gmul(a, 2) ^ gmul(b, 3) ^ c ^ d;
        arr[1][j] = a ^ gmul(b, 2) ^ gmul(c, 3) ^ d;
        arr[2][j] = a ^ b ^ gmul(c, 2) ^ gmul(d, 3);
        arr[3][j] = gmul(a, 3) ^ b ^ c ^ gmul(d, 2);

    }
}
uint8_t Rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10,
    0x20, 0x40, 0x80, 0x1B, 0x36
};
void addRoundKey(uint8_t roundKey[4][4],uint8_t state[4][4])
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            state[row][col] ^= roundKey[row][col];
        }
    }
}
void keyExpansion(
    const uint8_t key[16],
    uint8_t roundKeys[11][4][4])
{
    uint8_t expandedKey[176];

    // First 16 bytes are the original key
    for (int i = 0; i < 16; i++) {
        expandedKey[i] = key[i];
    }

    int bytesGenerated = 16;
    int rconIndex = 0;

    uint8_t temp[4];

    while (bytesGenerated < 176) {

        // Last 4 bytes
        for (int i = 0; i < 4; i++) {
            temp[i] = expandedKey[bytesGenerated - 4 + i];
        }

        // Every 16 bytes
        if (bytesGenerated % 16 == 0) {

            // RotWord
            uint8_t first = temp[0];

            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = first;

            // SubWord
            for (int i = 0; i < 4; i++) {
                temp[i] = sbox[temp[i]];
            }

            // XOR with Rcon
            temp[0] ^= Rcon[rconIndex++];
        }

        // Generate next 4 bytes
        for (int i = 0; i < 4; i++) {
            expandedKey[bytesGenerated] =
                expandedKey[bytesGenerated - 16] ^ temp[i];

            bytesGenerated++;
        }
    }

    // Convert expanded key into round keys
    for (int round = 0; round < 11; round++) {

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {

                roundKeys[round][row][col] =
                    expandedKey[round * 16 + col * 4 + row];
            }
        }
    }
}
void aesEncryptBlock(
    const uint8_t plaintext[16],
    uint8_t ciphertext[16],
    uint8_t roundKeys[11][4][4])
{
   uint8_t state[4][4];
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            state[row][col] =
                plaintext[col * 4 + row];
        }
    }

    // Initial AddRoundKey
    addRoundKey(roundKeys[0], state);

    // Rounds 1-9
    for (int round = 1; round <= 9; round++) {

        subByte(state);

        shiftRows(state);

        mixColumns(state);

        addRoundKey(roundKeys[round], state);
    }

    // Final round
    subByte(state);

    shiftRows(state);

    addRoundKey(roundKeys[10], state);

    // Copy state to ciphertext
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            ciphertext[col * 4 + row] =
                state[row][col];
        }
    }
}

#endif