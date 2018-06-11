#pragma once

#include <string>
#include <vector>
#include <set>
#include <fstream>

enum TokenType {
	KEYWORD,
	ID,

	SYMBOL,

	CHAR,
	INT, FLOAT,
	COMMENT,

	STRING,
	BOOL,

	ENDOFFILE,
};

enum State {
	_start, _done, _error,

	_id,
	_string, _string_trans,

	_int, _float,

	_char, _char_trans,
	_symbol,

	_comment, _comment_ending,
};

struct Token {
	TokenType type;
	std::string text;
	unsigned int lineNo;
};

class Tokenizer {

public: //▇▇▇  public:站在user角度，需要的能调用的方法、能直接访问的data
	Tokenizer();
	void openFile(const std::string& file);
	void reset();
	Token nextToken();

private:
	//TODO 就这几个状态data就已经完全表示了。更多附加的data只会让更新状态时也变得繁琐。
	std::fstream fin;  //持有引用这instance并不在该Class的数据内存段，内存段中只持有个pointer
	unsigned int row;
	unsigned int pos;

	std::string lineBuffer;
	std::vector<Token> results;

	std::set<std::string> Symbols;
	std::set<std::string> Keywords;

	char nextChar();
	void rollback();
	void initSymbols();
	void initKeywords();

};