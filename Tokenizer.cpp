#include "Tokenizer.h"
#include <iostream>

using namespace std;

Tokenizer::Tokenizer() {
	row = 1;
	pos = 0;
	//results
}

void Tokenizer::openFile(const std::string& file) {
	
	//�ļ�У��:����.jack�ļ�
	string suffix = file.substr(file.size() - 5, file.size());
	if (suffix != ".jack") {
		cout << "������.jackԴ�ļ���" << endl;
	}

	fin = fstream(file);
	if (!fin.fail()) {
		getline(fin, lineBuffer);
	} else {
		cout << "���ļ�ʧ�ܣ�" << endl;
	}
}

/*
 TokenType��Status������ȫһ��:
	���������ת���ַ���Stringʱ��tokentypeһֱ��string,����state������string��
  ------ --- - -- - - - - - -- - - - - -
  Ҫ������δ���Ļ��ܼ򵥣�
	����һ���Ϸ���abc123,������״̬ת�ƹ��̣�������һ����Ҫ�޸ġ�
 */
Token Tokenizer::nextToken() {
	Token token;
	token.lineNo = row;
	//string& text = token.text;

	State state = _start;

	while (state != _done && state!= _error) {
		char ch = nextChar();

		switch (state) {
		//����������������������������������������������������������������������������������������������������
		case _start:
			if (ch == ' ' || ch == '\t' || ch == '\n') continue; //��ʼʱ�ƹ����������
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
				//�쳣����
			}
		//����������������������������������������������������������������������������������������������������
		case _id:
			if (isalpha(ch) || isdigit(ch) || ch == '_') {
				token.text += ch;
			} else {
				state = _done;

				rollback();
			}
		//���������������������������������������������������������������������������������������������������� 
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
		//���������������������������������������������������������������������������������������������������� 
		case _string:
			if (isalpha(ch)) {
				token.text += ch;
			}else if (ch=='\\') {
				state = _string_trans;
			}else if (ch=='"') {
				state = _done;
				token.text += ch;
			}
		//���������������������������������������������������������������������������������������������������� 
		case _char:
			if (isalpha(ch)) {
				//state = _done;
				token.text += ch;
			}else if (ch=='\\') {
				token.text += ch;
			} else if (ch='\'') {
				state = _done;
				token.text += ch;
			} else {
				state = _error;
			}
		//���������������������������������������������������������������������������������������������������� 
		case _symbol:
			if (token.text=="/" ) {
				if (ch == '*') {
					state = _comment;
					token.text += ch;
				}
			} else if (token.text==">" && ch=='=') {
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
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	//����
	Symbols.insert("{");
	Symbols.insert("}");
	Symbols.insert("(");
	Symbols.insert(")");
	Symbols.insert("[");
	Symbols.insert("]");
	//���
	Symbols.insert(".");
	Symbols.insert(",");
	Symbols.insert(";");
	//�Ӽ��˳�
	Symbols.insert("+");
	Symbols.insert("-");
	Symbols.insert("*");
	Symbols.insert("/");
	//λ�����
	Symbols.insert("&");
	Symbols.insert("|");
	Symbols.insert("~");
	//��С�Ƚ�
	Symbols.insert("<");
	Symbols.insert(">");
	Symbols.insert("=");
						//��char��					
	Symbols.insert(">=");
	Symbols.insert("<=");
	Symbols.insert("==");
	Symbols.insert("!=");

	//�߼������
	//    Symbols.insert("!");
	//    Symbols.insert("&&");
	//    Symbols.insert("||");
}
              
void Tokenizer::initKeywords() {
	//�ඨ��
	Keywords.insert("class");
	Keywords.insert("constructor");
	Keywords.insert("function");
	Keywords.insert("method");
	Keywords.insert("field");
	Keywords.insert("static");
	Keywords.insert("this");

	//��������
	Keywords.insert("int");
	Keywords.insert("char");
	Keywords.insert("boolean");
	Keywords.insert("void");
	Keywords.insert("true");
	Keywords.insert("false");

	//���̿���
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



