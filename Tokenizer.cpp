#include "Tokenizer.h"
#include <iostream>
#include<stdio.h>

using namespace std;

Tokenizer::Tokenizer() {
	row = 1;
	pos = 0;
	//results
	initKeywords();
	initSymbols();
}

std::string tokenTypeName[] = { "keyword","id","symbol","char",
"int","float","comment_line","comment_multi","string","boolean","EOF" };

void Tokenizer::openFile(const std::string& file) {

	//文件校验:须是.jack文件
	string suffix = file.substr(file.size() - 5, file.size());
	if (suffix != ".jack") {
		cout <<"must be .jack file"<< endl;
	}

	fin = fstream(file);
	if (!fin.fail()) {
		getline(fin, lineBuffer);
		lineBuffer += '\n';
	} else {
		cout<<"failed to open file!"<<endl;
	}
}

char Tokenizer::nextChar() {
	if (pos >= lineBuffer.size()) {
		row++;
		getline(fin, lineBuffer);

		if (!fin.fail()) {
			return EOF;
		}

		lineBuffer += '\n';
		pos = 0;
	}
	//return lineBuffer[pos++];   // wrong code
	char ch = lineBuffer[pos];
	pos++;
	return ch;
}

void Tokenizer::initSymbols() {
	//括号
	Symbols.insert("{");
	Symbols.insert("}");
	Symbols.insert("(");
	Symbols.insert(")");
	Symbols.insert("[");
	Symbols.insert("]");
	//间隔
	Symbols.insert(".");
	Symbols.insert(",");
	Symbols.insert(";");
	//加减乘除
	Symbols.insert("+");
	Symbols.insert("-");
	Symbols.insert("*");
	Symbols.insert("/");
	//位运算符
	Symbols.insert("&");
	Symbols.insert("|");
	Symbols.insert("~");
	//大小比较
	Symbols.insert("<");
	Symbols.insert(">");
	Symbols.insert("=");
	//多char的					
	Symbols.insert(">=");
	Symbols.insert("<=");
	Symbols.insert("==");
	Symbols.insert("!=");

	//逻辑运算符
	//    Symbols.insert("!");
	//    Symbols.insert("&&");
	//    Symbols.insert("||");
}

void Tokenizer::initKeywords() {
	//类定义
	Keywords.insert("class");
	Keywords.insert("constructor");
	Keywords.insert("function");
	Keywords.insert("method");
	Keywords.insert("field");
	Keywords.insert("static");
	Keywords.insert("this");

	//基本类型
	Keywords.insert("int");
	Keywords.insert("char");
	Keywords.insert("boolean");
	Keywords.insert("void");
	Keywords.insert("true");
	Keywords.insert("false");

	//流程控制
	Keywords.insert("if");
	Keywords.insert("else");
	Keywords.insert("while");
	Keywords.insert("return");
	//    Keywords.insert("null");
}

void Tokenizer::rollback() {
	pos--;
	scannedChars.pop_back();
}

void Tokenizer::reset() {
	row = 0;
	pos = 0;
}

/*
▇▇ 词法分析不要考虑语法上的可能。
比如  foo'_ 这种，tokenizer应当识别为3个token， 而不是说认为error
*/
Token Tokenizer::nextToken() {
	Token token;
	token.lineNo = row+1;
	token.type = ID;
	//string& text = token.text;

	State state = _start;
	while (state != _done && state != _error) {
		char ch = nextChar();
		//printf("%u", ch);
		//cout << "scanning " + {ch} +"   in row " + to_string(token.lineNo) << endl;
		scannedChars.push_back(ch);

		switch (state) {
			case _start:
				if (ch == ' ' || ch == '\t' || ch == '\n') {

				}else if (isalpha(ch) || ch=='_') {
					state = _id;

					token.text += ch;
					token.type = TokenType::ID;
					
				}else if (isdigit(ch)) {
					state = _int; 
					
					token.text += ch;
					token.type = TokenType::INT;
				}else if (ch=='\"') {
					state = _string;

					//token.text += ch;
					token.type = TokenType::STRING;
				}else if (ch=='\'') {
					state = _char;

					token.type = TokenType::CHAR;
				} else if (Symbols.find({ch}) != Symbols.end()) {
					state = _symbol;

					token.text += ch;
					token.type = TokenType::SYMBOL;
				} else if (ch==EOF) {
					state = _done;

					token.type = TokenType::ENDOFFILE;
				} else { //其他不在上述范围的ch，无法跳转、也无法_done
					state = _error;

					token.text += ch;
				}
				break;
			case _id:
				if (isalpha(ch) || ch == '_' || isdigit(ch)) {
					token.text += ch;

				} else {
					state = _done;
					rollback();
				}
				break;
			case _int:
				if (isdigit(ch)) {
					token.text += ch;

				}else if (ch=='.') {
					state = _float_dot;

					token.text += ch;
					token.type = TokenType::FLOAT;
				}else {
					state = _done;
					rollback();
				}
				break;

			case _float_dot:
				if (isdigit(ch)) {
					state = _float;

					token.text += ch;
				} else {
					state = _error;

					token.text += ch;
				}
				break;
			case _float:
				if (isdigit(ch)) {
					token.text += ch;
				} else  {
					state = _done;
					rollback();
				}

				break;
			case _string:
				if (ch != '\"'  && ch!= '\\') {
					token.text += ch;
				} else if (ch == '\"') {
					state = _string_trans;
				} else if (ch == '\"') {
					state = _done;
				}

				break;
			case _string_trans:
				if (ch=='\\' || ch=='\'' ||ch=='\?') {
					state = _string;
					token.text += ch;
				}else {
					state = _error;
				}
				break;
			case _char:
				if (isalpha(ch)) {
					state = _char_hasOne;

					token.text += ch;
				} else if (ch=='\\') {
					state = _char_trans;

				} else {
					state = _error;
				}

				break;
			case _char_trans:
				if (ch == '\\' || ch == '\'' || ch == '\?') {
					state = _char_hasOne;
					token.text += ch;
				}else {
					state = _error;
					token.text += ch;
				}
				break;
			case _char_hasOne:
				if (ch=='\'') {
					state = _done;
				} else {
					state = _error;
				}
				break;
			case _symbol:
				if (token.text=="/" && ch=='/') {
					state = _comment_line;
				} else if (token.text=="/" && ch=='*') {
					state = _comment_block;
				} else if ((token.text==">" || token.text == "<" 
					|| token.text == "!" || token.text == "=" ) && ch=='=' ) {

					state = _done;
					token.text += ch;
				} else {
					state = _done;
				}

				break;
			case _comment_line:
				pos = lineBuffer.length(); //TODO
				break;
			case _comment_block:
				if (ch=='*') {
					state = _comment_ending;
				} else {
					token.text += ch;
				}
				break;
			case _comment_ending:
				if (ch=='/') {
					state = _done;
				} else {
					state = _comment_block;
					token.text += "*";
				}
			default:
				continue;
		}
	}

	if (state == _done) {
		if (token.type == ID) {
			if (isKeyword(token.text)) {
				token.type = KEYWORD;
			}
		}
		cout<<"line-"+ to_string(token.lineNo)+"\t"+tokenTypeName[token.type]+"\t\t"+token.text<<endl;
		results.push_back(token);

	}else if (state == _error) {
		cout<<"error happend. line "+to_string(token.lineNo)+",character is "+token.text<<endl;
	}
	return token;
}

bool Tokenizer::isKeyword(const std::string& text) {
	return Keywords.find(text) != Keywords.end();
}


