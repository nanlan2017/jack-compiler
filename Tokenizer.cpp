#include "Tokenizer.h"
#include <iostream>

using namespace std;

Tokenizer::Tokenizer() {
	row = 1;
	pos = 0;
	//results
}

void Tokenizer::openFile(const std::string& file) {

	//文件校验:须是.jack文件
	string suffix = file.substr(file.size() - 5, file.size());
	if (suffix != ".jack") {
		cout << "必须是.jack源文件！" << endl;
	}

	fin = fstream(file);
	if (!fin.fail()) {
		getline(fin, lineBuffer);
	} else {
		cout << "打开文件失败！" << endl;
	}
}

/*
 TokenType和Status并不完全一致:
	比如解析含转移字符的String时，tokentype一直是string,但是state不能是string。
  ------ --- - -- - - - - - -- - - - - -
  要调试这段代码的话很简单：
	拿着一个合法的abc123,待入这状态转移过程，看看哪一步需要修改。
 */
Token Tokenizer::nextToken() {
	Token token;
	token.lineNo = row;
	//string& text = token.text;

	State state = _start;

	while (state != _done && state != _error) {
		char ch = nextChar();

		switch (state) {
			//——————————————————————————————————————————————————
			case _start:
				if (ch == ' ' || ch == '\t' || ch == '\n') continue; //开始时绕过无意义符号
				if (isalpha(ch)) {
					state = _id;

					token.type = TokenType::ID;
					token.text += ch;
				} else if (isdigit(ch)) {
					state = _int;

					token.type = TokenType::INT;
					token.text += ch;
				} else if (ch == '"') {
					state = _string;

					token.type = TokenType::STRING;
					token.text += ch;
				} else if (Symbols.find({ ch }) != Symbols.end()) {
					state = _symbol;

					token.type = TokenType::SYMBOL;
					token.text += ch;
				} else if (ch == '\'') {
					state = _char;

					token.type = TokenType::CHAR;
					token.text += ch;
				} else {
					//异常处理
				}
				//——————————————————————————————————————————————————
			case _id:
				if (isalpha(ch) || isdigit(ch) || ch == '_') {
					token.text += ch;
				} else {
					state = _done;

					rollback();
				}
				//—————————————————————————————————————————————————— 
			case _int:
				if (isdigit(ch)) {
					token.text += ch;
				} else if (ch == '.') {
					state = _float;

					token.type = FLOAT;
					token.text += ch;
				} else {
					state = _done;

					rollback();
				}
				//—————————————————————————————————————————————————— 
			case _string:
				if (isalpha(ch)) {
					token.text += ch;
				} else if (ch == '\\') {
					state = _string_trans;
				} else if (ch == '"') {
					state = _done;
					token.text += ch;
				}
				//—————————————————————————————————————————————————— 
			case _char:
				if (isalpha(ch)) {
					//state = _done;
					token.text += ch;
				} else if (ch == '\\') {
					token.text += ch;
				} else if (ch = '\'') {
					state = _done;
					token.text += ch;
				} else {
					state = _error;
				}
				//—————————————————————————————————————————————————— 
			case _symbol:
				if (token.text == "/") {
					if (ch == '*') {
						state = _comment;
						token.text += ch;
					}
				} else if (token.text == ">" && ch == '=') {
					token.text += ch;
				} else if (token.text == "<" && ch == '=') {
					token.text += ch;
				} else if (token.text == "!" && ch == '=') {
					token.text += ch;
				} else if (token.text == "=" && ch == '=') {
					//token.text += ch;
				} else {
					state = _done;
					//token.text += ch;
				}
				token.text += ch;
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			case _float:
				break;
		}
	}

	results.push_back(token);
	return token;
}

char Tokenizer::nextChar() {
	if (pos >= lineBuffer.size()) {
		row++;
		getline(fin, lineBuffer);
		lineBuffer += '\n';
		pos = 0;
	}
	return lineBuffer[pos++];
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
}

void Tokenizer::reset() {
	row = 0;
	pos = 0;
}

Token Tokenizer::nextToken_new() {
	Token token;
	//string& text = token.text;

	State state = _start;
	while (state != _done && state != _error) {
		char ch = nextChar();
		switch (state) {
			case _start:
				if (0) {
					//无效字符（空格等），仍保持_start状态
				}else if (1) {
					state = _id;
				}else if (2) {
					state = _int;
				}else if (3) {
					state = _string;
				}else if (4) {
					state = _char;
				}else if (5) {
					state = _symbol;
				} else { //其他不在上述范围的ch
					state = _error;
				}

			case _id:
				if (0) {
					//累计，仍为未完成的id
				}else if (1) {  //<———— 遇到此ch判定为id识别完成
					state = _done;
					//识别完成后再判此id是否keyword
				} else {
					state = _error;
				}

			case _int:
				if (1) {
					
				}else if (2) {
					state = _float;
				}else if (3) {
					state = _done;
				} else {
					state = _error;
				}

			case _float:
				if (1) {

				} else if (3) {
					state = _done;
				} else {
					state = _error;
				}

			case _string:
				if (1) {

				} else if (3) {
					state = _string_trans;
				} else if (2) {
					state = _done;
				} else {
					state = _error;
				}

			case _string_trans:
				if (1) {
					state = _string;
				}else {
					state = _error;
				}
			case _char:
				if (1) {

				} else if (2) {
					state = _char_trans;

				} else if (2) {
					state = _done;
				} else {
					state = _error;
				}

			case _char_trans:
				if (1) {
					state = _char;
				}else if (2) {
					state = _error;
				}

			case _symbol:
				

			case _comment:

			case _comment_ending:

		}
	}
}

