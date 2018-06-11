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

public: //�~�~�~  public:վ��user�Ƕȣ���Ҫ���ܵ��õķ�������ֱ�ӷ��ʵ�data
	Tokenizer();
	void openFile(const std::string& file);
	void reset();
	Token nextToken();

private:
	//TODO ���⼸��״̬data���Ѿ���ȫ��ʾ�ˡ����฽�ӵ�dataֻ���ø���״̬ʱҲ��÷�����
	std::fstream fin;  //����������instance�����ڸ�Class�������ڴ�Σ��ڴ����ֻ���и�pointer
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