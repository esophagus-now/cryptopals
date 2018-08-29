#ifndef BUFFER_HPP
#define BUFFER_HPP 1

#include <iterator>
#include <vector>
#include <string>

//single-character XOR decode
struct scx_dec {
	std::string str;
	int score;
	unsigned char key;
	bool operator>(const scx_dec &other);
};

class bytebuf {
	std::vector<unsigned char> data;
	
	public:
	static const int HEX = 0;
	static const int ASCII = 1;
	static const int BASE64 = 2;
	
	bytebuf();
	bytebuf(std::string str, int mode);
	~bytebuf();
	
	bytebuf operator^(const bytebuf &other) const;
	bytebuf operator^(const unsigned char other) const;
	
	bytebuf operator% (const bytebuf &other); //Repeating-key XOR
	int operator- (const bytebuf &other); //Hamming distance
	
	std::string toHex() const;
	std::string toB64() const;
	
	operator std::string() const; //Print the buffer as though it were an ASCII string
	
	int englishScore() const; //Score of how much this looks like English
	
	scx_dec likelyDecode() const; //Most likely single-character XOR decode
};

extern char const * const b64_table;

#endif
