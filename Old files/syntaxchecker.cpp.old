#include "bytes.hpp"

using namespace std;

void testfn (void) {
	bytes test("deadbeefcafedeadfee1dead08675309", bytes::ASCII);
	auto blocks = test.inBlocks(3);
	for (auto &b : blocks) {
		cout << hex << b << endl;
	}
	cout << endl;
	cout << dec;
	cout << endl << test << endl;
}
