#include <iostream>
#include "bytes.hpp"
#include "scx_dec.hpp"

using namespace std;

string const b64_table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

///The following few functions had to be defined "outside" the class
///in order to compile properly

bytes::bytes(byteview other) : data(other.begin(), other.end()), defaultmode(other.defaultmode) {}

//This is really finicky fencepost stuff
byteview bytes::sample(int b, int s) {
	it my_begin = data.begin() + b;
	it real_end = data.end();
	it my_end;
	if (s == 0) my_end = my_begin; //Not sure if this is worth including
	dt tmp = real_end - my_begin;
	if (tmp % s == 0) my_end = real_end;
	else my_end = my_begin + s*(1+tmp/s);	
	byteview ret(my_begin, s, my_end);
	ret.defaultmode = defaultmode;
	return ret;
}

byteview bytes::nsample(int b, int s, int n) {
	//We need this byteview's end iterator to line up with a multiple of
	//of the step size past the beginning
	//s = 3, E means where the end iterator should end up, $ is the vector's real end
	
	//Case 1: b + s*n > data.end() but less than data.end() + s [EASY CASE]
	//n = 5
	//0123456789012$xx
	//b__^__^__^__^__E
	
	//Case 2: b + s*n >= data.end() + s
	//n = 7
	//01234567890123$xxxxxxx
	//b__^__^__^__^__E__^__^
	//In this case, E should be at b + ceil(size/n)
	
	//Case 3: b + s*n == data.end() but greater than data.end() - s [EASY CASE]
	//n = 6
	//0123456789012345$xx
	//b__^__^__^__^__^__E
	
	//Case 4: b + s*n <= data.end() - s [EASY CASE]
	//n = 4
	//012345678901234$xx
	//b__^__^__^__E
	using it = bytes::it;
	it my_begin = data.begin() + b;
	it real_end = data.end();
	it my_end = data.begin() + b + s*n;
	while (my_end >= real_end + s) my_end -= s; //Could be more efficient if we used division
	return byteview(my_begin, s, my_end < real_end ? my_end : real_end);
}

std::vector<byteview> bytes::inBlocks(int bsize) {
	if (bsize == 0) {
		return std::vector<byteview>();
	}
	int size = data.size();
	int numFullBlocks = size/bsize;
	int numBlocks = numFullBlocks + (size % bsize == 0 ? 0 : 1);
	std::vector<byteview> ret;
	ret.reserve(numBlocks);
	
	it curStart = data.begin();
	int i;
	for (i = 0; i < numFullBlocks; i++) {
		ret.emplace_back(curStart, 1, curStart + bsize);
		curStart += bsize;
	}
	
	if (i < numBlocks) {
		ret.emplace_back(curStart, 1, data.end());
	}
	
	return ret;
}

std::ostream &operator<<(std::ostream &o, const bytes &other) {
	switch (other.defaultmode) {
		case bytes::HEX:
			o << other.toHex();
			break;
		case bytes::ASCII:
			o << std::string(other);
			break;
		case bytes::BASE64:
			o << other.toB64();
			break;
	}
	
	return o;
}

std::ostream &operator<<(std::ostream &o, byteview &other) {
	switch (other.defaultmode) {
		case bytes::HEX:
			o << other.toHex();
			break;
		case bytes::ASCII:
			o << std::string(other);
			break;
		case bytes::BASE64:
			o << other.toB64();
			break;
	}
	return o;
}
