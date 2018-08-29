#include <iostream>
#include <fstream>
#include <typeinfo>
#include "buffer.hpp"

using namespace std;

int main() {
	cout << "Challenge 1" << endl;
	bytebuf b("49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d", bytebuf::HEX);
	cout << b.toHex() << endl;
	cout << b.toB64() << endl;
	
	cout << "Challenge 2" << endl;
	bytebuf part1("1c0111001f010100061a024b53535009181c", bytebuf::HEX);
	bytebuf part2("686974207468652062756c6c277320657965", bytebuf::HEX);
	cout << (part1^part2).toHex() << endl;
	
	cout << "Challenge 3" << endl;
	bytebuf enc("1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736", bytebuf::HEX);
	scx_dec d = enc.likelyDecode();
	cout << d.str << endl;
	
	cout << "Challenge 4" << endl;
	ifstream fp("4.txt", ios::in);
	
	string line;
	getline(fp, line);
	d = bytebuf(line, bytebuf::HEX).likelyDecode();
	while (getline(fp, line)) {
		scx_dec tmp = bytebuf(line, bytebuf::HEX).likelyDecode();
		if (tmp > d) {
			d = tmp;
		}
	}
	fp.close();
	
	cout << d.str << endl;
	
	cout << "Challenge 5" << endl;
	b = bytebuf("Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal", bytebuf::ASCII);
	enc = b % bytebuf("FOUR", bytebuf::ASCII);
	cout << enc.toHex() << endl;
	
	cout << "Challenge 6" << endl;
	b = bytebuf("this is a test", bytebuf::ASCII);
	cout << b - bytebuf("wokka wokka!!!", bytebuf::ASCII) << endl;
	
	b = bytebuf("12345678abcdefgh", bytebuf::ASCII);
	bytebuf tmp = bytebuf(b.sample(0,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.sample(1,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.sample(2,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.sample(3,4));
	cout << string(tmp) << endl;
	
	tmp = bytebuf(b.nsample(0,1,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.nsample(4,1,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.nsample(8,1,4));
	cout << string(tmp) << endl;
	tmp = bytebuf(b.nsample(12,1,4));
	cout << string(tmp) << endl;
	
	for (const int &ks : {2,3,4,5,6,7}) {
		auto chunk1 = enc.nsample(0,ks,ks);
		auto chunk2 = enc.nsample(ks,ks,ks);
		cout << "chunk lengths: " << chunk1.size() << ", " << chunk2.size() << endl;
		int hdist = bytebuf(chunk1) - bytebuf(chunk2);
		float nhdist = (float)hdist / (float) ks;
		cout << "For key size " << ks << ", normalized dist = " << nhdist << endl;
	}
	
	return 0;
}
