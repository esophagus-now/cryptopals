#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP 1

#include <string>
#include <sstream>
#include <iomanip>


std::string const b64_table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

//I'm not sure if RVO happens when you return stringstream.str(), but
//performance really isn't critical in this function. 
template <typename T>
std::string to_hex(T const& bv) {
	
	static_assert(sizeof(*bv.begin()) == 1, "Error: data type too large");
	
	std::stringstream s;
	s << std::setfill('0') << std::hex; //Output in hex
	for(auto &c: bv) {
		s << std::setw(2) << +c; //The unary plus is finally useful!
	}
	return s.str();
}

template <typename T>
std::string to_b64(T const& bv) {
	
	static_assert(sizeof(*bv.begin()) == 1, "Error: data type too large");
	
	//Every three bytes represent four b64 characters. So, we have
	//a special case for if len % 3 is 0, 1, or 2
	
	std::stringstream s;
	
	int len = bv.size();
	int i = 0;
	switch (len % 3) {
		case 2:
			//01234567 01234567
			//22221111 11000000
			s << b64_table[bv[0] >> 4];
			s << b64_table[((bv[0] & 0xF)<<2) + (bv[1]>>6)];
			s << b64_table[bv[1] & 0x3F];
			i += 2;
			break;
		case 1:
			//01234567
			//11000000
			s << b64_table[bv[0] >> 6];
			s << b64_table[bv[0] & 0x3F];
			i++;
			break;
		default:
			break;
	}
	
	for (; i < len; i+=3) {
		//01234567 01234567 01234567
		//33333322 22221111 11000000
		s << b64_table[bv[i] >> 2];
		s << b64_table[((bv[i]&0x3)<<4) + (bv[i+1]>>4)];
		s << b64_table[((bv[i+1]&0xF)<<2) + (bv[i+2]>>6)];
		s << b64_table[bv[i+2]&0x3F];
	}
	
	return s.str();
}

#endif
