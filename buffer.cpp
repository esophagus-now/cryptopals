#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include "buffer.hpp"

using namespace std;

char const * const b64_table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

bytebuf::bytebuf() {} //Nothing to do; std::vector() automatically called
bytebuf::bytebuf(string hexstr) {
	//This doesn't need to be efficient, since it happens very rarely
	int len = hexstr.length();
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
		char lo = hexstr[0];
		if (lo > '9') lo += 9; //Tricky hack
		lo &= 0xF;
		data.push_back(lo);
		i++;
	}
	
	for (; i < len; i+=2) {
		char hi = hexstr[i];
		if (hi > '9') hi += 9; //Tricky hack
		hi <<= 4;
		
		char lo = hexstr[i+1];
		if (lo > '9') lo += 9; //Tricky hack
		lo &= 0xF;
		
		data.push_back(hi + lo); //Should never trigger a resize
	}
}

bytebuf::~bytebuf() {} //Nothing to do; std::~vector() automatically called

bytebuf bytebuf::operator^(const bytebuf &other) {
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

bytebuf bytebuf::operator^(const unsigned char other) {
	bytebuf b;
	b.data.reserve(data.size());
	
	for(auto c: data) b.data.push_back(c ^other);
	
	return b;
}

//I'm not sure if RVO happens when you return stringstream.str(), but
//performance really isn't critical in this function. It could take an
//entire millisecond and it wouldn't matter, since it is so rarely called.
string bytebuf::toHex() {
	stringstream s;
	s << hex; //Output in hex
	for(unsigned char &c: data) {
		s << +c; //The unary plus is finally useful!
	}
	return s.str();
}

string bytebuf::toB64() {
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
