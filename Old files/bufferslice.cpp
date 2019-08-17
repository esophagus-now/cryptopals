//Decided to split this into a separate file, since it's mostly wrangling
//types and stuff

#include "buffer.hpp"
#include <vector>

using ranger = bytebuf::ranger;
using slice = bytebuf::slice; //clean up code a little
using namespace std;

unsigned char &ranger::operator*() const {
	return *p;
}

bool ranger::operator!= (const ranger &other) const {
	return p < other.p;
}

ranger &ranger::operator++() {
	p += step;
	return *this;
}

ranger slice::begin() const {
	return ranger(start, step);
}

ranger slice::end() const {
	return ranger(one_past_last, 0);
}

unsigned slice::size() const {
	return (one_past_last - start) / step; //This is the number of elements.
}

slice::operator bytebuf() const {
	bytebuf ret;
	ret.data.reserve((one_past_last - start)/step);
	for(auto &c : *this) ret.data.push_back(c);
	return ret; //Will this work?
}

slice bytebuf::sample(int b, int s) {
	//It would be nice if I could change data.end() to the corresponding unsigned char*
	return slice(data.data() + b, s, data.data() + data.size()); //uuuuuuuuugly
}

slice bytebuf::nsample(int b, int s, int n) {
	unsigned char *real_end = data.data() + data.size();
	unsigned char *my_end = data.data() + b + s*n;
	return slice(data.data() + b, s, my_end < real_end ? my_end : real_end);
}

ranger bytebuf::begin() {
	return ranger(data.data(), 1);
}

ranger bytebuf::end() {
	return ranger(data.data() + data.size(), 0);
}
