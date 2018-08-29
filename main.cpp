#include <iostream>
#include <fstream>
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
	string best = string(enc);
	unsigned char c = 0;
	int maxScore = enc.englishScore();
	do {
		bytebuf tmp = enc^c++;
		int score = tmp.englishScore();
		if (score > maxScore) {
			maxScore = score;
			best = string(tmp);
		}
	} while (c);
	
	cout << best << endl;
	
	cout << "Challenge 4" << endl;
	ifstream fp("4.txt", ios::in);
	
	string line;
	maxScore = -1; //We can get away with this since scores are always positive
	while (getline(fp, line)) {
		c = 0;
		b = bytebuf(line, bytebuf::HEX);
		do {
			bytebuf tmp = b^c++;
			int score = tmp.englishScore();
			if (score > maxScore) {
				maxScore = score;
				best = string(tmp);
			}
		} while(c);
	}
	fp.close();
	
	cout << best << endl;
	
	cout << "Challenge 5" << endl;
	b = bytebuf("Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal", bytebuf::ASCII);
	b = b % bytebuf("ICE", bytebuf::ASCII);
	cout << b.toHex() << endl;
	
	return 0;
}
