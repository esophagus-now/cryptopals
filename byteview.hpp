#ifndef BYTEVIEW_HPP
#define BYTEVIEW_HPP 1

#include <vector>

//Assumes that std::vector uses random access iterators
//TODO: Add in const iterator stuff
//TODO: Readability is suffering (esp. when it comes to the iterator typedefs). Try to clean it up
class byteview {
	typedef std::vector<unsigned char> bytevec;
	
	public:
	typedef bytevec::iterator it;
	typedef bytevec::difference_type dt;
	
	///Constructors, desctructors, etc.
	byteview(bytevec &other) : my_start(other.begin()), my_step(1), my_end(other.end()) {}
	byteview(bytevec &&other) : my_start(other.begin()), my_step(1), my_end(other.end()) {}
	
	byteview(it start, dt step, it end) : my_start(start), my_step(step), my_end(end) {}
	
	///"Arithmetic" related to the crypto challenges
	//TODO: Should this be defined as member functions to byteview, or
	//as STL-style free functions?
	
	///Misc
	dt size() const {
		return (my_end - my_start) / my_step;
	}
	
	///Iterators
	class byteview_iterator : public it {
		typedef bytevec::difference_type dt;
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
		
		//Source of a very tricky bug: I forgot to include difference _between_ byteview_iterators
		dt operator-(byteview_iterator other) {
			//There must be a better way...
			return (*static_cast<it*>(this) - *static_cast<it*>(&other)) / other.my_step;
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
	
	//Results in an unnecessary copy, but that's OK
	operator std::string() {
		return std::string(begin(), end());
	}	
	
	private:
	
	it my_start;
	dt my_step;
	it my_end;
};

#endif
