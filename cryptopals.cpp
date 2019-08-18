#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <unordered_set>
#include <map>
#include "conversions.hpp"
#include "bytevector.hpp"
#include "byteview.hpp"
#include "cryptopals.hpp"
#include "scx_dec.hpp"

using namespace std;

#include "txt/tables.txt"

bytes operator^(byteview a, byteview b) {
	if (a.size() != b.size()) {
		string msg = "Data size mismatch in bytes operator^(";
		msg += to_string(a.size());
		msg += ", ";
		msg += to_string(b.size());
		msg += ")";
		throw std::runtime_error(msg);
	}
	bytes ret(a.size());
	for (int i = 0; i < int(ret.size()); i++) {
		ret[i] = a[i] ^ b[i];
	}
	
	return ret;
}

bytes operator^(byteview b, const byte c) {
	bytes ret(b.size());
	
	for (int i = 0; i < b.size(); i++) {
		ret[i] = b[i] ^ c;
	}
	
	return ret;
}

bytes operator%(byteview a, byteview b) {
	bytes ret(a.size());
	
	for (int i = 0; i < a.size(); i++) {
		ret[i] = a[i] ^ b[i % b.size()];
	}
	
	return ret;
}

//From Stanford bit-twiddling hacks. Very clever!
static const int BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

int operator- (byteview a, byteview b) {
	if (a.size() != b.size()) {
		throw runtime_error("Length mismatch in operator-(byteview a, byteview b)");
	}
	int sum = 0;
	for (int i = 0; i < a.size(); i++) {
		sum += BitsSetTable256[a[i]^b[i]];
	}
	
	return sum;
}



int englishScore(byteview other) {
	/*static const unordered_map<char, int> scores= {
		{'E', 13},
		{'e', 13},
		{'T', 12},
		{'t', 12},
		{'A', 11},
		{'a', 11},
		{'O', 10},
		{'o', 10},
		{'I', 9},
		{'i', 9},
		{'N', 8},
		{'n', 8},
		{' ', 7},
		{'S', 6},
		{'s', 6},
		{'H', 5},
		{'h', 5},
		{'R', 4},
		{'r', 4},
		{'D', 3},
		{'d', 3},
		{'L', 2},
		{'l', 2},
		{'U', 1},
		{'u', 1}
	};*/
	static const unordered_map<char, int> scores= {
		{'E', 1},
		{'e', 2},
		{'T', 1},
		{'t', 2},
		{'A', 1},
		{'a', 2},
		{'O', 1},
		{'o', 2},
		{'I', 1},
		{'i', 2},
		{'N', 1},
		{'n', 2},
		{' ', 2},
		{'S', 1},
		{'s', 2},
		{'H', 1},
		{'h', 2},
		{'R', 1},
		{'r', 2},
		{'D', 1},
		{'d', 2},
		{'L', 1},
		{'l', 2},
		{'U', 1},
		{'u', 2}
	};
	
	int score = 0;
	for(auto c: other) {
		auto ptr = scores.find(c);
		if (ptr != scores.end()) score += ptr->second;
		if (!isprint(c)) score-=1;
		if (isalpha(c)) score++;
	}
	
	return score;
}

scx_dec likelyDecode(byteview other) {
	byte c = 0;
	byte bestc = 0;
	bytes bestBytes = to_bytes(other);
	int maxScore = englishScore(other);
	do {
		bytes tmp = other^c;
		int score = englishScore(tmp);
		if (score > maxScore) {
			maxScore = score;
			bestc = c;
			bestBytes = move(tmp);
		}
	} while (++c);
	return {.str = to_string(bestBytes),.score = maxScore,.key = bestc};
}


static bytes keyschedule(byteview key) {
	bytes ret(16*11); 
	
	//Start by copying the key
	for (int i = 0; i < 16; i++) ret[i] = key[i];
	
	int last = 0; //Last block index
	for (int i = 0; i < 10; i++) {
		//In the end, normal C pointer fiddling is best for this stuff
		unsigned char tmp[4];
		int next = last + 16;
		tmp[0] = sbox[ret[last + 13]] ^ rcon[i][0];
		tmp[1] = sbox[ret[last + 14]] ^ rcon[i][1];
		tmp[2] = sbox[ret[last + 15]] ^ rcon[i][2];
		tmp[3] = sbox[ret[last + 12]] ^ rcon[i][3];
		//cout << "tmp = " << bytebuf(string((char *)(tmp),4),bytebuf::ASCII).toHex() << endl;
		
		ret[next + 0] = ret[last + 0]^tmp[0];
		ret[next + 1] = ret[last + 1]^tmp[1];
		ret[next + 2] = ret[last + 2]^tmp[2];
		ret[next + 3] = ret[last + 3]^tmp[3];
		
		for (int j = 4; j < 16; j++) ret[next + j] = ret[last + j]^ret[next + j - 4];
		
		last = next;
	}
	
	return ret;
}

static void decrypt128(byteview ctxt, bytes &keysched) {
	//Add key 11
	for (int i = 0; i < 16; i++) {
		ctxt[i] ^= keysched[10*16 + i];
	}
	
	for (int i = 9; i > 0; i--) {
		//cout << "round[" << 10-i << "].istart   " << ret.toHex() << endl;
		//	InvShiftRows
		//COLUMN-MAJOR ORDERING! Oops!
		/* [ 0  4  8 12
		 *   1  5  9 13
		 *   2  6 10 14
		 *   3  7 11 15]
		 * */
		unsigned char tmp = ctxt[3];
		ctxt[3] = ctxt[7];
		ctxt[7] = ctxt[11];
		ctxt[11] = ctxt[15];
		ctxt[15] = tmp;
		
		tmp = ctxt[2];
		ctxt[2] = ctxt[10];
		ctxt[10] = tmp;
		tmp = ctxt[6];
		ctxt[6] = ctxt[14];
		ctxt[14] = tmp;
		
		tmp = ctxt[13];
		ctxt[13] = ctxt[9];
		ctxt[9] = ctxt[5];
		ctxt[5] = ctxt[1];
		ctxt[1] = tmp;
		
		//cout << "round[" << 10-i << "].is_row   " << ret.toHex() << endl;
		//	InvSubBytes
		for (auto &a : ctxt) {
			a = isbox[a];
		}
		//cout << "round[" << 10-i << "].is_box   " << ret.toHex() << endl;
		
		//	Add key i
		for (int j = 0; j < 16; j++) {
			ctxt[j] ^= keysched[i*16 + j];
		}
		
		//	InvMixColumns
		/*[14 11 13  9
		 *  9 14 11 13
		 * 13  9 14 11
		 * 11 13  9 14]
		 * */
		 
		//COLUMN-MAJOR ORDERING! Oops!
		/* [ 0  4  8 12
		 *   1  5  9 13
		 *   2  6 10 14
		 *   3  7 11 15]
		 * */
		 
		 for (int j = 0; j < 16; j += 4) {
			 unsigned char tvec[4];
			 tvec[0] = times14[ctxt[0 + j]] ^ times11[ctxt[1 + j]] ^ times13[ctxt[2 + j]] ^  times9[ctxt[3 + j]];
			 tvec[1] =  times9[ctxt[0 + j]] ^ times14[ctxt[1 + j]] ^ times11[ctxt[2 + j]] ^ times13[ctxt[3 + j]];
			 tvec[2] = times13[ctxt[0 + j]] ^  times9[ctxt[1 + j]] ^ times14[ctxt[2 + j]] ^ times11[ctxt[3 + j]];
			 tvec[3] = times11[ctxt[0 + j]] ^ times13[ctxt[1 + j]] ^  times9[ctxt[2 + j]] ^ times14[ctxt[3 + j]];
			 ctxt[0 + j] = tvec[0];
			 ctxt[1 + j] = tvec[1];
			 ctxt[2 + j] = tvec[2];
			 ctxt[3 + j] = tvec[3];
		 }
	}
	
	//InvShiftRows
	unsigned char tmp = ctxt[3];
	ctxt[3] = ctxt[7];
	ctxt[7] = ctxt[11];
	ctxt[11] = ctxt[15];
	ctxt[15] = tmp;
	
	tmp = ctxt[2];
	ctxt[2] = ctxt[10];
	ctxt[10] = tmp;
	tmp = ctxt[6];
	ctxt[6] = ctxt[14];
	ctxt[14] = tmp;
	
	tmp = ctxt[13];
	ctxt[13] = ctxt[9];
	ctxt[9] = ctxt[5];
	ctxt[5] = ctxt[1];
	ctxt[1] = tmp;
		
	//InvSubBytes
	for (auto &a : ctxt) {
		a = isbox[a];
	}
	
	//Add key 0
	for (int i = 0; i < 16; i++) {
		ctxt[i] ^= keysched[i];
	}
	return;
}

void decrypt(bytes &enc, bytes &key) {
	bytes ks = keyschedule(key);
	auto blocks = inBlocks(enc, 16);
	
	for (auto &b : blocks) {
		if (b.size() != 16) {
			cout << "Skipping last non-full block" << endl;
		}
		decrypt128(b, ks);
	}
	
	return;
}

void cbc_decrypt(bytes &enc, bytes &key, bytes &iv) {
	auto tmp = to_bytes(nsample(enc, 0, 1, enc.size()-16));
	auto mask = iv;
	mask.insert(mask.end(), tmp.begin(), tmp.end()); //Copy ciphertext after IV
	
	decrypt(enc, key);
	
	enc = enc ^ mask;
}


static void encrypt128(byteview ctxt, bytes &keysched) {
	//Add key 0
	for (int i = 0; i < 16; i++) {
		ctxt[i] ^= keysched[i];
	}
	
	for (int i = 1; i < 10; i++) {
		//SubBytes
		for (auto &a : ctxt) {
			a = sbox[a];
		}
		
		//ShiftRows
		//COLUMN-MAJOR ORDERING! Oops!
		/* [ 0  4  8 12
		 *   1  5  9 13
		 *   2  6 10 14
		 *   3  7 11 15]
		 * */
		unsigned char tmp = ctxt[15];
		ctxt[15] = ctxt[11];
		ctxt[11] = ctxt[7];
		ctxt[7] = ctxt[3];
		ctxt[3] = tmp;
		
		tmp = ctxt[2];
		ctxt[2] = ctxt[10];
		ctxt[10] = tmp;
		tmp = ctxt[6];
		ctxt[6] = ctxt[14];
		ctxt[14] = tmp;
		
		tmp = ctxt[1];
		ctxt[1] = ctxt[5];
		ctxt[5] = ctxt[9];
		ctxt[9] = ctxt[13];
		ctxt[13] = tmp;
		
		//MixColumns
		/*[02 03 01 01
		 * 01 02 03 01
		 * 01 01 02 03
		 * 03 01 01 02]
		 * */
		 
		//COLUMN-MAJOR ORDERING! Oops!
		/* [ 0  4  8 12
		 *   1  5  9 13
		 *   2  6 10 14
		 *   3  7 11 15]
		 * */
		 
		 for (int j = 0; j < 16; j += 4) {
			 unsigned char tvec[4];
			 tvec[0] = times2[ctxt[0 + j]] ^ times3[ctxt[1 + j]] ^ ctxt[2 + j] ^ ctxt[3 + j];
			 tvec[1] = ctxt[0 + j] ^ times2[ctxt[1 + j]] ^ times3[ctxt[2 + j]] ^ ctxt[3 + j];
			 tvec[2] = ctxt[0 + j] ^ ctxt[1 + j] ^ times2[ctxt[2 + j]] ^ times3[ctxt[3 + j]];
			 tvec[3] = times3[ctxt[0 + j]] ^ ctxt[1 + j] ^ ctxt[2 + j] ^ times2[ctxt[3 + j]];
			 ctxt[0 + j] = tvec[0];
			 ctxt[1 + j] = tvec[1];
			 ctxt[2 + j] = tvec[2];
			 ctxt[3 + j] = tvec[3];
		 }
		
		//Add round key
		for (int j = 0; j < 16; j++) {
			ctxt[j] ^= keysched[i*16 + j];
		}
	}
	
	//SubBytes
	for (auto &a : ctxt) {
		a = sbox[a];
	}
	
	//ShiftRows
	//COLUMN-MAJOR ORDERING! Oops!
	/* [ 0  4  8 12
	 *   1  5  9 13
	 *   2  6 10 14
	 *   3  7 11 15]
	 * */
	unsigned char tmp = ctxt[15];
	ctxt[15] = ctxt[11];
	ctxt[11] = ctxt[7];
	ctxt[7] = ctxt[3];
	ctxt[3] = tmp;
	
	tmp = ctxt[2];
	ctxt[2] = ctxt[10];
	ctxt[10] = tmp;
	tmp = ctxt[6];
	ctxt[6] = ctxt[14];
	ctxt[14] = tmp;
	
	tmp = ctxt[1];
	ctxt[1] = ctxt[5];
	ctxt[5] = ctxt[9];
	ctxt[9] = ctxt[13];
	ctxt[13] = tmp;
	
	//Add key 11
	for (int i = 0; i < 16; i++) {
		ctxt[i] ^= keysched[10*16 + i];
	}
	return;
}

void encrypt(bytes &enc, bytes &key) {
	bytes ks = keyschedule(key);
	auto blocks = inBlocks(enc, 16);
	
	for (auto &b : blocks) {
		if (b.size() != 16) {
			cout << "Skipping last non-full block" << endl;
			break;
		}
		encrypt128(b, ks);
	}
	
	return;
}


void cbc_encrypt(bytes &enc, bytes &key, bytes &iv) {
	bytes ks = keyschedule(key);
	auto blocks = inBlocks(enc, 16);
	
	bytes last = iv;
	
	for (auto &b : blocks) {
		if (b.size() != 16) {
			cout << "Skipping last non-full block" << endl;
			break;
		}
		
		bytes tmp = last ^ b;
		encrypt128(tmp, ks);
		last = tmp;
		
		for (int i = 0; i < 16; i++) b[i] = tmp[i]; //Good enough
	}
	
	return;
}

bytes mystery_encryptor_11(bytes &enc) {
	bytes key = mk_rand_bytes(16);
	bytes ret = mk_rand_bytes((rand() % 6) + 5);
	bytes append = mk_rand_bytes((rand() % 6) + 5);
	add_to(ret, enc);
	add_to(ret, append);
	
	pad_to_mult(ret, 16);
	
	if (rand() & 1) {
		//ECB
		cout << "ECB" << endl;
		encrypt(ret, key);
	} else {
		//CBC
		cout << "CBC" << endl;
		bytes iv = mk_rand_bytes(16);
		cbc_encrypt(ret, key, iv);
	}
	
	return ret;
}

//Function we are attacking in challenge 12
bytes app_and_enc_12(bytes attack_str) {
	static auto unk_str = to_bytes(
		"Um9sbGluJyBpbiBteSA1LjAKV2l0aCBteSByYWctdG9wIGRvd24gc28gbXkg"
		"aGFpciBjYW4gYmxvdwpUaGUgZ2lybGllcyBvbiBzdGFuZGJ5IHdhdmluZyBq"
		"dXN0IHRvIHNheSBoaQpEaWQgeW91IHN0b3A/IE5vLCBJIGp1c3QgZHJvdmUg"
		"YnkK", bvec::BASE64);
	static auto key = mk_rand_bytes(16);
	
	bytes to_enc = attack_str;
	add_to(to_enc, unk_str);
	pad_to_mult(to_enc, 16);
	
	encrypt(to_enc, key);
	
	return to_enc;
}

map<bytes, byte> build_dict(bytes filler, int bsz) {
	map<bytes, byte> dict;
	for (int i = 0; i < 1<<8; i++) {
		unsigned char test_byte = i & 0xFF;
		filler.back() = test_byte;
		bytes response = app_and_enc_12(filler);
		bytes key(response.begin(), response.begin() + bsz);
		dict[key] = test_byte;
	}
	
	return dict;
}

void byte_at_a_time_simple() {	
	bytes att_str = to_bytes("A", bvec::ASCII);
	bytes ciphertxt = app_and_enc_12(att_str);
	bytes last = bytes(ciphertxt.begin(), ciphertxt.begin() + 1);
	
	int bsz;
	
	for (int i = 2; i < 24; i++) {
		add_to(att_str, to_bytes("A", bvec::ASCII));
		bytes ciphertxt = app_and_enc_12(att_str);
		//cout << to_hex(last) << endl;
		//cout << to_hex(bytes(ciphertxt.begin(), ciphertxt.begin() + i)) << endl;
		if (equal(last.begin(), last.end(), ciphertxt.begin())) {
			cout << "Block size is " << i - 1 << endl;
			bsz = i - 1;
			break;
		}
		last = bytes(ciphertxt.begin(), ciphertxt.begin() + i);
	}
	
	att_str = bytes(3*bsz, 'A');
	bytes response = app_and_enc_12(att_str);
	
	unordered_set<string> seen;
	for (auto j: inBlocks(response, 16)) {
		bool inserted;
		tie(ignore, inserted) = seen.insert(j);
		if (!inserted) /*meaning it is a duplicate*/ {
			cout << "This was likely encoded with ECB!" << endl;
			break;
		}
	}
	
	bytes filler = bytes(bsz, 'B');
	int block_count = 0;
	while(1) {
		att_str = bytes(bsz, 'B');
		for (int i = bsz - 1; i >= 0; i--) {
			att_str.pop_back();
			//cout << "Attack string = " << to_string(att_str) << endl;
			//cout << "Dict filler = " << to_string(filler) << endl;
			//Build dictionary
			auto dict = build_dict(filler, bsz);
			//Decode next byte
			response = app_and_enc_12(att_str);
			bytes key(response.begin() + bsz*block_count, response.begin() + bsz*block_count + bsz);
			auto it_to_el = dict.find(key);
			if (it_to_el == dict.end()) {
				cout << "Not found!" << endl;
				goto done;
			}
			byte elem = (*it_to_el).second;
			if (elem == 0) goto done;
			cout << char(elem);
			
			filler.back() = elem;
			filler = bytes(filler.begin()+1, filler.end());
			filler.emplace_back();
		}
		block_count++;
	}
	done:
	
	cout << endl;
}
