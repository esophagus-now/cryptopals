#ifndef CRYPTOPALS_HPP
#define CRYPTOPALS_HPP 1

#include "bytes.hpp"
#include "scx_dec.hpp"

bytes operator^(byteview a, byteview b);

bytes operator^(byteview b, const byte c);

bytes operator%(byteview a, byteview b);

int operator- (byteview a, byteview b);

int englishScore(byteview other);

scx_dec likelyDecode(byteview other);

void decrypt(bytes &enc, bytes &key);

#endif
