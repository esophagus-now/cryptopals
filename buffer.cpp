#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include "buffer.hpp"

using namespace std;

bool scx_dec::operator>(const scx_dec &other) {
	return score >= other.score;
}


char const * const b64_table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

bytebuf::bytebuf() {} //Nothing to do; std::vector() automatically called
bytebuf::bytebuf(string str, int mode) {
	if (mode == ASCII) {
		data = vector<unsigned char>(str.begin(), str.end());
	} else if (mode == HEX) {
		//This doesn't need to be efficient, since it happens very rarely
		int len = str.length();
		int newsz = (len>>1) + (len&1); //ceil(len/2)
		data.clear();
		data.reserve(newsz);
		//I know I'm doing this too much like a C programmer, which is why
		//reinterpret_cast had to be invoked here. It looks bad...
		//But for Christ's sake! Why should static_cast complain if I'm
		//converting from const char * to const unsigned char *???
		int i = 0;
		if (len&1) {
			//Special case if we have an odd number of nybbles
			char lo = str[0];
			if (lo > '9') lo += 9; //Tricky hack
			lo &= 0xF;
			data.push_back(lo);
			i++;
		}
		
		for (; i < len; i+=2) {
			char hi = str[i];
			if (hi > '9') hi += 9; //Tricky hack
			hi <<= 4;
			
			char lo = str[i+1];
			if (lo > '9') lo += 9; //Tricky hack
			lo &= 0xF;
			
			data.push_back(hi + lo); //Should never trigger a resize
		}
	} else if (mode == BASE64) {
		throw new runtime_error("bytebuf init from b64 string not implemented yet");
	}
}

bytebuf::~bytebuf() {} //Nothing to do; std::~vector() automatically called

bytebuf bytebuf::operator^(const bytebuf &other) const {
	if (data.size() != other.data.size()) {
		throw runtime_error("Data size mismatch in bytebuf::operator^");
	}
	
	bytebuf b;
	b.data.reserve(data.size());
	
	for (unsigned i = 0; i < data.size(); i++) {
		b.data.push_back(data[i] ^ other.data[i]);
	}
	
	return b; //RVO for the win
}

bytebuf bytebuf::operator^(const unsigned char other) const {
	bytebuf b;
	b.data.reserve(data.size());
	
	for(auto c: data) b.data.push_back(c ^other);
	
	return b;
}

bytebuf bytebuf::operator% (const bytebuf &other) {
	unsigned len = data.size();
	unsigned otherlen = other.data.size();
	bytebuf ret;
	ret.data.reserve(len);
	for (unsigned i = 0; i < len; i++) {
		ret.data.push_back(data[i] ^ other.data[i%otherlen]);
	}
	return ret; //Hooray for RVO
}

//From Stanford bit-twiddling hacks. Very clever!
static const int BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

int bytebuf::operator- (const bytebuf &other) {
	if (data.size() != other.data.size()) {
		throw runtime_error("Length mismatch in bytebuf::operator-");
	}
	int sum = 0;
	for (unsigned i = 0; i < data.size(); i++) {
		sum += BitsSetTable256[data[i]^other.data[i]];
	}
	
	return sum;
}

//I'm not sure if RVO happens when you return stringstream.str(), but
//performance really isn't critical in this function. It could take an
//entire millisecond and it wouldn't matter, since it is so rarely called.
string bytebuf::toHex() const {
	stringstream s;
	s << setfill('0') << hex; //Output in hex
	for(auto &c: data) {
		s << setw(2) << +c; //The unary plus is finally useful!
	}
	return s.str();
}

string bytebuf::toB64() const {
	//Every three bytes represent four b64 characters. So, we have
	//a special case for if len % 3 is 0, 1, or 2
	
	stringstream s;
	
	int len = data.size();
	int i = 0;
	switch (len % 3) {
		case 2:
			//01234567 01234567
			//22221111 11000000
			s << b64_table[data[0] >> 4];
			s << b64_table[((data[0] & 0xF)<<2) + (data[1]>>6)];
			s << b64_table[data[1] & 0x3F];
			i += 2;
			break;
		case 1:
			//01234567
			//11000000
			s << b64_table[data[0] >> 6];
			s << b64_table[data[0] & 0x3F];
			i++;
			break;
		default:
			break;
	}
	
	for (; i < len; i+=3) {
		//01234567 01234567 01234567
		//33333322 22221111 11000000
		s << b64_table[data[i] >> 2];
		s << b64_table[((data[i]&0x3)<<4) + (data[i+1]>>4)];
		s << b64_table[((data[i+1]&0xF)<<2) + (data[i+2]>>6)];
		s << b64_table[data[i+2]&0x3F];
	}
	
	return s.str();
}

//Results in an unnecessary copy, but that's OK
bytebuf::operator std::string() const {
	return string(data.begin(), data.end());
}

int bytebuf::englishScore() const {
	static const unordered_map<char, int> scores= {
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
	};
	
	int score = 0;
	for(auto c: data) {
		auto ptr = scores.find(c);
		if (ptr != scores.end()) score += ptr->second;
	}
	
	return score;
}

scx_dec bytebuf::likelyDecode() const {
	unsigned char c = 0;
	unsigned char bestc = 0;
	int maxScore = englishScore();
	do {
		bytebuf tmp = (*this)^c;
		int score = tmp.englishScore();
		if (score > maxScore) {
			maxScore = score;
			bestc = c;
		}
	} while (++c);
	
	bytebuf tmp = (*this)^bestc;
	return {.str = string(tmp),.score = maxScore,.key = bestc};
}
