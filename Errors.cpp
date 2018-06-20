#include "Errors.h"

#include <iostream>


using namespace std;

void unExpectedToken(const Token& wrongToken, const Token& expectedToken) {
	cerr << "unexpectd token:\t line:" + to_string(wrongToken.lineNo) + "\t value:" + wrongToken.text
		+ "-------> expected: " + expectedToken.text << endl;
}
