#include <iostream>
#include <fstream>
#include <algorithm>
#include <typeinfo>
#include "buffer.hpp"
#include "AES.hpp"

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
	b = bytebuf("Burning 'em, if you ain't quick and nimble@I go crazy when I hear a cymbal", bytebuf::ASCII);
	enc = b % bytebuf("ICE", bytebuf::ASCII);
	cout << enc.toHex() << endl;
	
	cout << "Challenge 6" << endl;
	/*b = bytebuf("this is a test", bytebuf::ASCII);
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
	cout << string(tmp) << endl;*/
	
	fp.open("6.txt", ios::in);
	string res;
	while(getline(fp,line)) res+=line;
	
	enc = bytebuf(res, bytebuf::BASE64);
	//b = bytebuf("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.", bytebuf::ASCII);
	//enc = b % bytebuf("MercuryVenusEarth", bytebuf::ASCII);
	
	fp.close();
	
	vector<pair<int, float> > vf;
	
	//I'm getting the right key size
	for (int ks = 2; ks <= 40 ; ks++) {
		auto chunk1 = bytebuf(enc.nsample(0,1,ks));
		//cout << bytebuf(chunk1).toHex() << endl;
		auto chunk2 = bytebuf(enc.nsample(ks,1,ks));
		//cout << bytebuf(chunk2).toHex() << endl;
		auto chunk3 = bytebuf(enc.nsample(2*ks,1,ks));
		//cout << bytebuf(chunk3).toHex() << endl;
		auto chunk4 = bytebuf(enc.nsample(3*ks,1,ks));
		//cout << bytebuf(chunk4).toHex() << endl;
		auto chunk5 = bytebuf(enc.nsample(4*ks,1,ks));
		//cout << bytebuf(chunk5).toHex() << endl;
		auto chunk6 = bytebuf(enc.nsample(5*ks,1,ks));
		//cout << bytebuf(chunk6).toHex() << endl;
		int hdist = (chunk1 - chunk2) + (chunk1 - chunk3) + (chunk1 - chunk4) + (chunk1 - chunk5) + (chunk1 - chunk6)
			+ (chunk2 - chunk3) + (chunk2 - chunk4) + (chunk2 - chunk5) + (chunk2 - chunk6)
			+ (chunk3 - chunk4) + (chunk3 - chunk5) + (chunk3 - chunk6)
			+ (chunk4 - chunk5) + (chunk4 - chunk6)
			+ (chunk5 - chunk6);
		float nhdist = (float)(hdist) / (15.0*(float) ks);
		vf.push_back({ks, nhdist});
		//cout << "key size " << ks << ", ndist " << nhdist << endl;
	}
	
	nth_element(vf.begin(), vf.begin(), vf.end(), [](pair<int, float> a, pair<int, float> b) {return get<1>(a) < get<1>(b);});
	
	vector<string> vs;
	//We'll use the best key size
	int ks = get<0>(vf[0]); //UGLY, but the lambda function in the sort makes it better than C
	for (int j = 0; j < ks; j++) {
		vs.push_back(bytebuf(enc.sample(j, ks)).likelyDecode().str);
	}
		
	string ret;
	for (unsigned i = 0; i < vs[0].size(); i++) {
		for (unsigned j = 0; j < vs.size(); j++) {
			try {
				ret.push_back(vs[j].at(i));
			} catch (exception &e) {
				//do nothing
			}
		}
	}
	cout << ret << endl;
	
	cout << "Challenge 7" << endl;
	fp.open("7.txt", ios::in);
	
	res.clear();
	while(getline(fp,line)) res+=line;
	fp.close();
	
	enc = bytebuf(res, bytebuf::BASE64);
	auto key = bytebuf("YELLOW SUBMARINE", bytebuf::ASCII);
	key = keyschedule(key);
	//cout << key.toHex() << endl;
	
	//cout << keyschedule("2b7e151628aed2a6abf7158809cf4f3c"_hbb).toHex() << endl;
	//cout << keyschedule("000102030405060708090a0b0c0d0e0f"_hbb).toHex() << endl;
	
	//bytebuf willitworkfirsttry("d1aa4f6578926542fbb6dd876cd20508", bytebuf::HEX); //Got this by encrypting "YELLOW SUBMARINE" with "YELLOW SUBMARINE"
	//No, it didn't work on the first try.
	//bytebuf debug = "69c4e0d86a7b0430d8cdb78070b4c55a"_hbb;
	//key = keyschedule("000102030405060708090a0b0c0d0e0f"_hbb);
	//cout << string(decrypt128(willitworkfirsttry, key)) << endl;
	
	return 0;
}
