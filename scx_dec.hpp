#ifndef SCX_DEC_HPP
#define SCX_DEC_HPP 1

#include <string>

//single-character XOR decode
struct scx_dec {
	std::string str;
	int score;
	unsigned char key;
	bool operator>(const scx_dec &other);
};

#endif
