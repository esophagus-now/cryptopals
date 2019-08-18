#ifndef CRYPTOPALS_HPP
#define CRYPTOPALS_HPP 1

#include "bytevector.hpp"
#include "scx_dec.hpp"

bytes operator^(byteview a, byteview b);

bytes operator^(byteview b, const byte c);

bytes operator%(byteview a, byteview b);

//Hamming distance
int operator- (byteview a, byteview b);

int englishScore(byteview other);

scx_dec likelyDecode(byteview other);

void decrypt(bytes &enc, bytes &key);

void cbc_decrypt(bytes &enc, bytes &key, bytes &iv);

void encrypt(bytes &enc, bytes &key);
#endif
