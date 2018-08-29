#ifndef BUFFER_HPP
#define BUFFER_HPP 1

#include <vector>
#include <string>

class bytebuf {
	std::vector<unsigned char> data;
	
	public:
	
	bytebuf();
	bytebuf(std::string hexstr);
	~bytebuf();
	
	bytebuf operator^(const bytebuf &other);
	bytebuf operator^(const unsigned char other);
	
	std::string toHex();
	std::string toB64();
	
	operator std::string() const; //Print the buffer as though it were an ASCII string
	
	int englishScore(); //Score of how much this looks like English
};

extern char const * const b64_table;

#endif
