#ifndef BUFFER_HPP
#define BUFFER_HPP 1

#include <iterator>
#include <vector>
#include <string>

//idea: move element-wise operators to bytebuf::slice, and get the 
//operators in bytebuf to work on slices instead.

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
	//~bytebuf();
	
	void resize(int len);
	
	bytebuf operator^(const bytebuf &other) const;
	bytebuf &operator^=(bytebuf &other);
	bytebuf operator^(const unsigned char other) const;
	
	bytebuf operator% (const bytebuf &other); //Repeating-key XOR
	int operator- (const bytebuf &other); //Hamming distance
	
	std::string toHex() const;
	std::string toB64() const;
	
	operator std::string() const; //Print the buffer as though it were an ASCII string
	
	int englishScore() const; //Score of how much this looks like English
	
	scx_dec likelyDecode() const; //Most likely single-character XOR decode
	
	class ranger { //I named this "ranger" before realizing it was a forward iterator
		unsigned char *p;
		int step;

		public:
		ranger(unsigned char *p, int step) : p(p), step(step) {} //will this work?
		unsigned char &operator*() const;
		bool operator!=(const ranger &other) const; //Hack: checks if this iterator is less than other, as opposed to simply not equal
		ranger &operator++();
	};
	
	class slice {
		unsigned char *start;
		int step;
		unsigned char *one_past_last;
		
		public:
		slice(unsigned char *start, int step, unsigned char *one_past_last) : start(start), step(step), one_past_last(one_past_last) {}
		ranger begin() const;
		ranger end() const;
		
		unsigned size() const;

		operator bytebuf() const;
	};
	
	slice sample(int begin, int step); //"Sample" the buffer at regular intervals
	slice nsample(int begin, int step, int n); //Like slice, but limited to n
	
	ranger begin();
	ranger end();
	
	unsigned char &operator[] (int index);
};

extern std::string const b64_table;
bytebuf operator"" _hbb(const char *, unsigned int len);

#endif
