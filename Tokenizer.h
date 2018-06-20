#pragma once

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string>
#include <vector>
#include <set>
#include <fstream>

enum TokenType {
	INIT,

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
std::string tokenTypeName[] = { "keyword","id","symbol","char",
								"int","float","comment_line","comment_multi","string","boolean","EOF" };
*/

/*-----------------------------------------------------------
▇▇ 《如何写跳转》
见草稿纸：
  X-State  ——> {char集合1}  ——> ★ X-State自身
		   ——> {char集合2}  ——> B-State
		   ——> {char集合3}  ——> C-State
			… …
		   ——> {char集合k}  ——> ★ Done识别完成 (当遇到可认为收尾的ch时，进入_done。比如string遇到"。注意rollback)
		   ——> {char集合n}  ——> ★ 无法累计、无法跳转、也不能认为识别完成： _error

  关键：以上的｛char集合｝全不重复、且涵盖了所有可能的字符。

------------------------------------------------------------
▇▇ 《如何定义状态枚举》(√ 先定义好细分状态、写状态跳转时就方便多了)
正因为你这1个state对应的可能情况太多了，（分的不够细），

比如 _symbol状态现在可能是：  >   >= 
————> 应该设计更多的辅助小状态，特指那一两种情况

比如：_start时遇到 +， 已经可以直接认为_done了（不支持+=或++）。  但这里跳去 _symbol，

————> 可以任意定义状态，只要明确代表一种情况、并能正确跳转
比如：遇到运算符时，若运算符是可能multi-char的运算符（>=,<=,!=,==），则跳去新定义的状态 _symbol_maybe2

------------------------------------------------------------
▇▇ 《_error状态？》
 Q:  什么时候进入 _error状态？
 A： 当前状态无法继续跳转、包括无法认为_done时， 进入_error

举例1： "fuck it'       '时，error
举例2： 123.a           a时，error
	    "fucki\k        k时，error
------------------------------------------------------------*/

enum State {  
	_start, 
	_done, 
	_error,

	_id,              // _myvar  S34t   foo  ▇▇ 注意：后面不是空格，如 foo{}时，是合法的！所以最后的else并不对应error
	
	_string,          // "wj   
	_string_trans,    // "wj\

	_int,             //142
	_float_dot,       //13.
	_float,           // 13.4

	                  //  ' 和 'c   区分开：两者遇到ch='a'的 判断是不同的。
	_char,            //   '
	_char_trans,      //   { '\  }
	_char_hasOne,     // 'c

	_symbol,          // +  - * / 
	_symbol_maybe2,   // >  <  = !

	//_comment,       
	_comment_line,    //    //
	_comment_block,   //    /* this is
	_comment_ending,  //    /* this is it *
};

struct Token {
	TokenType type;
	std::string text;
	unsigned int lineNo;

	Token() = default;
	Token(TokenType type,std::string&& text) {
		this->type = type;
		this->text = text;
	}

	bool operator!=(Token& token) {
		return !(type ==token.type && text==token.text);
	}
};

class Tokenizer {

public: //▇▇  public:站在user角度，需要的能调用的方法、能直接访问的data
	Tokenizer();
	void openFile(const std::string& file);
	void reset();
	Token nextToken();

	unsigned int row;
private:
	//▇▇ 就这几个状态data就已经完全表示了。更多附加的data只会让更新状态时也变得繁琐。
	std::fstream fin;
	
	unsigned int pos;

	std::string lineBuffer;

	std::vector<char> scannedChars;
	std::vector<Token> results;

	std::set<std::string> Symbols;
	std::set<std::string> Keywords;

	char nextChar();
	void rollback();
	void initSymbols();
	void initKeywords();
	bool isKeyword(const std::string& text);
};

#endif
