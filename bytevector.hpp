#ifndef BYTEVECTOR_HPP
#define BYTEVECTOR_HPP

#include <vector>
#include <string>
#include "byteview.hpp"

typedef unsigned char byte;
typedef std::vector<byte> bytevec;
typedef bytevec bytes;

namespace bvec {
	typedef enum {
		HEX,
		ASCII,
		BASE64
	} mode;
}

bytevec to_bytes(std::string str, bvec::mode mode);
bytevec to_bytes(byteview bv);

//Results in an unnecessary copy, but that's OK
std::string to_string(bytevec const& bv);

///byteview stuff
byteview sample(bytevec & bv, int b, int s);

byteview nsample(bytevec & bv, int b, int s, int n);

std::vector<byteview> inBlocks(bytevec & bv, int bsize);
#endif
