#include "Tokenizer.h"

int main() {
	Tokenizer tokenizer;
	tokenizer.openFile("Test.jack");
	tokenizer.reset();
	Token token;
	while (token.type != TokenType::ENDOFFILE && tokenizer.row<10) {
		tokenizer.nextToken();
	}
	system("pause");
	return 1;
}