#ifndef BUFFER_HPP
#define BUFFER_HPP 1

#include <vector>
#include <string>

class bytebuf {
	std::vector<unsigned char> data;
	
	public:
	static const int HEX = 0;
	static const int ASCII = 1;
	static const int BASE64 = 2;
	
	bytebuf();
	bytebuf(std::string str, int mode);
	~bytebuf();
	
	bytebuf operator^(const bytebuf &other);
	bytebuf operator^(const unsigned char other);
	
	bytebuf operator% (const bytebuf &other); //Repeating-key XOR
	
	std::string toHex() const;
	std::string toB64() const;
	
	operator std::string() const; //Print the buffer as though it were an ASCII string
	
	int englishScore() const; //Score of how much this looks like English
};

extern char const * const b64_table;

#endif
