#include <iostream>
#include <fstream>
#include <algorithm>
#include <typeinfo>
#include "buffer.hpp"
#include "bytes.hpp"
#include "cryptopals.hpp"
#include "AES.hpp"

template <typename T>
class TD;

using namespace std;

extern void testfn(void);

//TODO: Migrate to my rewritten bytes class

int main() {
	cout << "Challenge 1" << endl;
	bytes b("49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d", bytes::HEX);
	cout << b.toHex() << endl;
	cout << b.toB64() << endl;
	
	cout << "Challenge 2" << endl;
	bytes part1("1c0111001f010100061a024b53535009181c", bytes::HEX);
	bytes part2("686974207468652062756c6c277320657965", bytes::HEX);
	cout << (part1^part2) << endl;
	
	
	
	cout << "Challenge 3" << endl;
	bytes enc("1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736", bytes::HEX);
	scx_dec d = likelyDecode(enc);
	cout << d.str << endl;
	
	
	cout << "Challenge 4" << endl;
	ifstream fp("4.txt", ios::in);
	
	string line;
	/*getline(fp, line);
	d = likelyDecode(bytes(line, bytes::HEX));
	while (getline(fp, line)) {
		scx_dec tmp = likelyDecode(bytes(line, bytes::HEX));
		if (tmp > d) {
			d = tmp;
		}
	}*/
	fp.close();
	
	//cout << d.str << endl;
	
	
	cout << "Challenge 5" << endl;
	b = bytes("Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal", bytes::ASCII);
	enc = b % bytes("ICE", bytes::ASCII);
	cout << enc.toHex() << endl;
	
	cout << "Challenge 6" << endl;
	
	fp.open("6.txt", ios::in);
	string res;
	while(getline(fp,line)) res+=line;
	
	enc = bytes(res, bytes::BASE64);
	fp.close();
	
	vector<pair<int, float> > vf;
	
	//I'm getting the right key size
	for (int ks = 2; ks <= 40 ; ks++) {
		auto chunk1 = enc.nsample(0,1,ks);
		auto chunk2 = enc.nsample(ks,1,ks);
		auto chunk3 = enc.nsample(2*ks,1,ks);
		auto chunk4 = enc.nsample(3*ks,1,ks);
		auto chunk5 = enc.nsample(4*ks,1,ks);
		auto chunk6 = enc.nsample(5*ks,1,ks);
		int hdist = (chunk1 - chunk2) + (chunk1 - chunk3) + (chunk1 - chunk4) + (chunk1 - chunk5) + (chunk1 - chunk6)
			+ (chunk2 - chunk3) + (chunk2 - chunk4) + (chunk2 - chunk5) + (chunk2 - chunk6)
			+ (chunk3 - chunk4) + (chunk3 - chunk5) + (chunk3 - chunk6)
			+ (chunk4 - chunk5) + (chunk4 - chunk6)
			+ (chunk5 - chunk6);
		float nhdist = (float)(hdist) / (15.0*(float) ks);
		//cout << "ks = " << ks << ", nhdist = " << nhdist << endl;
		vf.push_back({ks, nhdist});
	}
	
	nth_element(vf.begin(), vf.begin(), vf.end(), [](pair<int, float> a, pair<int, float> b) {return get<1>(a) < get<1>(b);});
	
	vector<string> vs;
	//We'll use the best key size
	int ks = get<0>(vf[0]); //UGLY, but the lambda function in the sort makes it better than C
	cout << "ks = " << ks << endl;
	for (int j = 0; j < ks; j++) {
		vs.push_back(likelyDecode(enc.sample(j, ks)).str);
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
	
	enc = bytes(res, bytes::BASE64);
	enc.defaultmode = bytes::ASCII;
	auto key = bytes("YELLOW SUBMARINE", bytes::ASCII);
	decrypt(enc, key);
	cout << enc << endl;
	
	/*
	cout << "Challenge 8" << endl;
	fp.open("8.txt", ios::in);
	
	vector<bytebuf> vb;
	while(getline(fp,line)) vb.push_back(bytebuf(line, bytebuf::HEX));
	fp.close();
	//for (auto &b: vb)
	//cout << string(decrypt(b,key)) << endl << endl;
	*/
	return 0;
}
