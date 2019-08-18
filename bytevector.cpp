#include "bytevector.hpp"
#include "byteview.hpp"
#include "conversions.hpp"
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

bytevec to_bytes(string str, bvec::mode mode) {
	vector<byte> data;
	
	if (mode == bvec::ASCII) {
		data = vector<byte>(str.begin(), str.end());
	} else if (mode == bvec::HEX) {
		//This doesn't need to be efficient, since it happens very rarely
		int len = str.length();
		int newsz = (len>>1) + (len&1); //ceil(len/2)
		data.clear();
		data.reserve(newsz + 1); //Plus one for safety, I guess
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
	} else if (mode == bvec::BASE64) {
		//NOTE: There is a HUGE difference between "left-aligned"
		//and "right-aligned" base64. This code deals with left-aligned,
		//even though that is not the most obvious solution. For some
		//reason, right-aligned makes more sense to me. Anyway, I should
		//make the alignment an option in the future. For now this works.
		
		//Four b64 bytes make three bytes
		//--000000 --111111 --222222 --333333
		//--000000 --001111 --111122 --222222
		
		if (str.back() == '=') str.pop_back();
				
		unsigned len = str.size();
		std::vector<byte> v;
		v.reserve(len);
		
		int newsz = 3 * (len >> 2) + (len % 3); //ceil(3*len/4)
		data.reserve(newsz + 1); //Plus one for safety, I guess
		
		//I'm just gonna use find(). Performance is really not critical here
		for(char &c : str) {
			//Convert the b64 chars to their 6 bit value.
			v.push_back(static_cast<byte>(b64_table.find(c)));
		}
		
		unsigned i = 0;		
		for (; i < len - (len % 4); i += 4) {
			//--000000 --111111 --222222 --333333
			//--000000 --001111 --111122 --222222
			data.push_back((v[i]<<2) + (v[i+1]>>4));
			data.push_back((v[i+1]<<4) + (v[i+2]>>2));
			data.push_back((v[i+2]<<6) + v[i+3]);
		}
		switch (len % 4) {
			case 3:
				//--000000 --111111 --222222
				//--000000 --001111 --1111--
				data.push_back((v[i]<<2) + (v[i+1]>>4));
				data.push_back((v[i+1]<<4) + (v[i+2]>>2));
				break;
			case 2:
				//--000000 --111111
				//--000000 --001111
				data.push_back((v[i]<<2) + (v[i+1]>>4));
				//data.push_back((v[i]<<6) + v[i+1]);
				break;
			case 1:
				//--000000
				//--000000
				data.push_back(v[i]<<2);//?
				break;
			default:
				break;
		}
	}
	
	return data;
}

bytevec to_bytes(byteview bv) {
	return bytevec(bv.begin(), bv.end());
}

bytevec mk_rand_bytes(int num) {
	bytevec ret;
	ret.reserve(num);
	
	for (int i = 0; i < num; i++) {
		ret.push_back(rand()); //Should auto-truncate to char?
	}
	
	return ret;
}

///Pretty-printing
//Results in an unnecessary copy, but that's OK
string to_string(bytevec const& bv) {
	return std::string(bv.begin(), bv.end());
}

//This is really finicky fencepost stuff
byteview sample(bytevec & bv, int b, int s) {
	using it = byteview::it; //iterator type
	using dt = byteview::dt; //difference type
	it my_begin = bv.begin() + b;
	it real_end = bv.end();
	it my_end;
	if (s == 0) my_end = my_begin; //Not sure if this is worth including
	dt tmp = real_end - my_begin;
	if (tmp % s == 0) my_end = real_end;
	else my_end = my_begin + s*(1+tmp/s);	
	return byteview(my_begin, s, my_end);;
}

byteview nsample(bytevec & bv, int b, int s, int n) {
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
	using it = byteview::it;
	it my_begin = bv.begin() + b;
	it real_end = bv.end();
	it my_end = bv.begin() + b + s*n;
	while (my_end >= real_end + s) my_end -= s; //Could be more efficient if we used division
	return byteview(my_begin, s, my_end < real_end ? my_end : real_end);
}

vector<byteview> inBlocks(bytevec & bv, int bsize) {
	using it = byteview::it;
	
	if (bsize == 0) {
		return vector<byteview>();
	}
	int size = bv.size();
	int numFullBlocks = size/bsize;
	int numBlocks = numFullBlocks + (size % bsize == 0 ? 0 : 1);
	vector<byteview> ret;
	ret.reserve(numBlocks);
	
	it curStart = bv.begin();
	int i;
	for (i = 0; i < numFullBlocks; i++) {
		ret.emplace_back(curStart, 1, curStart + bsize);
		curStart += bsize;
	}
	
	if (i < numBlocks) {
		ret.emplace_back(curStart, 1, bv.end());
	}
	
	return ret;
}

void pad_to_mult(bytevec &bv, int sz) {
	int amount_to_pad = (sz - (bv.size()%sz));
	if (amount_to_pad == sz) return;
	bytevec append(amount_to_pad, 0x04);
	add_to(bv, append);
}
