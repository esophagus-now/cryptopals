#include <stdexcept>
#include <unordered_map>
#include "cryptopals.hpp"
#include "scx_dec.hpp"

using namespace std;

#include "tables.txt"

bytes operator^(byteview a, byteview b) {
	if (a.size() != b.size()) {
		throw std::runtime_error("Data size mismatch in bytes operator^(byteview a, byteview b)");
	}
	bytes ret(a.size());
	for (int i = 0; i < ret.size(); i++) {
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
	bytes bestBytes(other);
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
	return {.str = string(bestBytes),.score = maxScore,.key = bestc};
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
	auto blocks = enc.inBlocks(16);
	
	for (auto &b : blocks) {
		if (b.size() != 16) {
			cout << "Skipping last non-full block" << endl;
		}
		decrypt128(b, ks);
	}
	
	return;
}
