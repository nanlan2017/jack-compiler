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
	COMMENT_LINE,COMMENT_MULTI,

	STRING,
	BOOL,

	ENDOFFILE,
};

/*
// 可以任意定义状态，只要明确代表一种情况、并能正确跳转
比如：遇到运算符时，若运算符是可能multi-char的运算符（>=,<=,!=,==），则跳去新定义的状态 _symbol_maybe2
*/
enum State {  
	_start, 
	_done, 
	_error,

	_id,  // _myvar  Student   foo
	
	_string, // "wj   
	_string_trans,  // "wj\

	_int,  //142
	_float, //13.    13.4

	_char, //   '  'c
	_char_trans, //  '\

	_symbol, // +  - * / 
	_symbol_maybe2, // >  <  = !
	_comment, //    /*    /* this is    //
	_comment_ending,  //    /* this is it *
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
	Token nextToken_new();

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