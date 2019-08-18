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

void cbc_encrypt(bytes &enc, bytes &key, bytes &iv);

bytes mystery_encryptor_11(bytes &enc);

//Function we are attacking in challenge 12
bytes app_and_enc_12(bytes attack_str);

void byte_at_a_time_simple();
#endif
