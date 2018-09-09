//This is a rewrite of my bytebuffer stuff.
//At first it was working really well, but then it started getting
//clunky.

#ifndef BYTES_HPP
#define BYTES_HPP 1

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>

typedef unsigned char byte;

std::string const b64_table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

class byteview;

class bytes {
	typedef std::vector<byte>::iterator it;
	typedef std::vector<byte>::difference_type dt;
	typedef std::vector<byte>::const_iterator cit;
	std::vector<byte> data;
	
	public:
	
	enum MODE {
		HEX,
		ASCII,
		BASE64
	};
	
	MODE defaultmode = HEX;
	
	///Constructors, destructors, etc.
	bytes(std::string str, MODE mode) {
		defaultmode = mode;
		if (mode == ASCII) {
			data = std::vector<byte>(str.begin(), str.end());
		} else if (mode == HEX) {
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
		} else if (mode == BASE64) {
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
				v.push_back(static_cast<unsigned char>(b64_table.find(c)));
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
	}
	
	///Pretty-printing
	//I'm not sure if RVO happens when you return stringstream.str(), but
	//performance really isn't critical in this function. 
	std::string toHex() const {
		std::stringstream s;
		s << std::setfill('0') << std::hex; //Output in hex
		for(auto &c: data) {
			s << std::setw(2) << +c; //The unary plus is finally useful!
		}
		return s.str();
	}
	
	std::string toB64() const {
		//Every three bytes represent four b64 characters. So, we have
		//a special case for if len % 3 is 0, 1, or 2
		
		std::stringstream s;
		
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
	operator std::string() const {
		return std::string(data.begin(), data.end());
	}
	
	///Iterators
	it begin() {
		return data.begin();
	}
	cit cbegin() {
		return data.cbegin();
	}
	
	it end() {
		return data.end();
	}
	cit cend() {
		return data.cend();
	}
	
	///Byteview stuff
	operator byteview();
	
	byteview sample(int b, int s);
	
	byteview nsample(int b, int s, int n);
	
	std::vector<byteview> inBlocks(int bsize);
};

//Assumes that std::vector uses random access iterators
//TODO: Add in const iterator stuff
//TODO: Readability is suffering (esp. when it comes to the iterator typedefs). Try to clean it up
class byteview {
	typedef std::vector<byte>::iterator it;
	typedef std::vector<byte>::difference_type dt;
	it my_start;
	dt my_step;
	it my_end;
	
	public:
	
	bytes::MODE defaultmode = bytes::MODE::HEX;
	
	///Constructors, desctructors, etc.
	byteview(bytes &other) : my_start(other.begin()), my_step(1), my_end(other.end()), defaultmode(other.defaultmode) {}
	
	byteview(it start, dt step, it end) : my_start(start), my_step(step), my_end(end) {}
	
	///"Arithmetic" related to the crypto challenges
	//TODO: Should this be defined as member functions to byteview, or
	//as STL-style free functions?
	
	///Iterators
	class byteview_iterator : public it {
		typedef std::vector<byte>::difference_type dt;
		dt my_step;
		
		public:
		///Overloads
		//We can keep the default ctors, the relops, and dereferences
		//However, we need to overload all pointer arithmetic inlucding operator[] (ugh)
		
		byteview_iterator(it ptr_in) : it(ptr_in), my_step(1) {}
		byteview_iterator(it ptr_in, dt step) : it(ptr_in), my_step(step) {}
		
		byteview_iterator &operator++() {
			it::operator+=(my_step);
			return *this;
		}
		byteview_iterator operator++(int) {
			byteview_iterator ret = *this;
			it::operator+=(my_step);
			return ret;
		}
		byteview_iterator &operator--() {
			it::operator-=(my_step);
			return *this;
		}
		byteview_iterator operator--(int) {
			byteview_iterator ret = *this;
			it::operator-=(my_step);
			return ret;
		}
		
		byteview_iterator operator+(int n) {
			return it::operator+(my_step * n);
		}
		byteview_iterator operator-(int n) {
			return it::operator-(my_step * n);
		}
		
		it::value_type &operator[](int n) {
			return it::operator[](my_step*n);
		}
		
		
	};
	
	typedef byteview_iterator iterator; //Give outside code access to this typename
	
	iterator begin() {
		return byteview_iterator(my_start, my_step);
	}
	iterator end() {
		return byteview_iterator(my_end);
	}
	
	it::value_type &operator[](int n) {
		return my_start[my_step*n];
	}
	
	///Pretty-printing
	
	//I'm not sure if RVO happens when you return stringstream.str(), but
	//performance really isn't critical in this function. 
	std::string toHex() {
		std::stringstream s;
		s << std::setfill('0') << std::hex; //Output in hex
		for(auto &c: *this) {
			s << std::setw(2) << +c; //The unary plus is finally useful!
		}
		return s.str();
	}
	
	std::string toB64() {
		//Every three bytes represent four b64 characters. So, we have
		//a special case for if len % 3 is 0, 1, or 2
		
		std::stringstream s;
		
		int len = (my_end - my_start) / my_step;
		int i = 0;
		switch (len % 3) {
			case 2:
				//01234567 01234567
				//22221111 11000000
				s << b64_table[(*this)[0] >> 4];
				s << b64_table[(((*this)[0] & 0xF)<<2) + ((*this)[1]>>6)];
				s << b64_table[(*this)[1] & 0x3F];
				i += 2;
				break;
			case 1:
				//01234567
				//11000000
				s << b64_table[(*this)[0] >> 6];
				s << b64_table[(*this)[0] & 0x3F];
				i++;
				break;
			default:
				break;
		}
		
		for (; i < len; i+=3) {
			//01234567 01234567 01234567
			//33333322 22221111 11000000
			s << b64_table[(*this)[i] >> 2];
			s << b64_table[(((*this)[i]&0x3)<<4) + ((*this)[i+1]>>4)];
			s << b64_table[(((*this)[i+1]&0xF)<<2) + ((*this)[i+2]>>6)];
			s << b64_table[(*this)[i+2]&0x3F];
		}
		
		return s.str();
	}
	
	//Results in an unnecessary copy, but that's OK
	operator std::string() {
		return std::string(begin(), end());
	}	
};

///The following few functions had to be defined "outside" the class
///in order to compile properly

bytes::operator byteview() {
	return byteview(*this);
}

//This is really finicky fencepost stuff
byteview bytes::sample(int b, int s) {
	//There's a helpful diagram in the nsample function
	it my_begin = data.begin() + b;
	it real_end = data.end();
	it my_end;
	if (s == 0) my_end = my_begin; //Not sure if this is worth including
	dt tmp = real_end - my_begin;
	if (tmp % s == 0) my_end = real_end;
	else my_end = my_begin + s*(1+tmp/s);
	return byteview(my_begin, s, my_end);
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

#endif
