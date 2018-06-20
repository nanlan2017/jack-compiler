#include "Parser.h"

#include "Errors.h"

using TreeNode = Parser::TreeNode;

void __usecase__() {
	vector<string> files;
	/*files.push_back("Dog.jack");
	files.push_back("Cat.jack");*/
	files.emplace_back("Dog.jack");

	Parser parser(files);
	parser.parse_program();
	Parser::TreeNode* tree = parser.getSyntaxTree();
	parser.print_tree();
}

Parser::Parser(vector<string>& filenames) {
	files = filenames;
	syntaxTree = new TreeNode();

	//TODO  vector<> δ���г�ʼ����������null�ģ�
	/*
	Dog dog;��Ա����ν��г�ʼ���ģ�
		��������ʱ�Ѿ�Ϊ������Ĭ�ϵĹ��캯���ˣ�
		�����Ĺ�����ô����  Dog dog(); ??   Dog d; dog=d;
	*/
}

Token& Parser::getToken() {
	if (tokenBuf_waiting.empty()) {
		Token token = scanner->nextToken();
		tokenBuf_used.push_back(token);
	} else {
		Token token = tokenBuf_waiting.front();
		//Token& token = tokenBuf_waiting.front(); //wrong: reference 'token' lost its instance;
		tokenBuf_waiting.pop_front();
		tokenBuf_used.push_back(token);
	}
	return tokenBuf_used.back();
}

void Parser::ungetToken() {
	Token token = tokenBuf_used.back();
	tokenBuf_used.pop_back();
	//push by ref      :=           get target directly +  then store its value
	tokenBuf_waiting.push_front(token);
}

//true: varDec
bool Parser::isFollowing_varDec_or_stat() {
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	if (following is not var dec) {
	return EMPTY;
	} else {
	head (LIST-NODE)  -> # -> #
	}
	-------------------------------------------
	@ how to judge ?
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	//TODO ����˼����ط���ƾ��һ��token�޷��ж��治���ڱ�������������Ӧtype,��type������ID��
	/*
	��û�б���������������statementҲ������ID��ͷ
	�� ��νLR(0)�ķ���1��tokenȷ����һ��ƥ��Ĳ���ʽ��
	LR(0)�������ˣ����ķ�����LR(0)�ģ�����

	�� ��Ȼ��Ҳ��������ȥ��������token���жϡ��ж���rollback����
	�� ����˵��Ҫ������������ �Ծ�parse_item()�������᷵�ظ���
	//TODO ����漰�����������һ��ԭ������ˣ�ֻ�� ��ȷ���˿��п��޵ĳɷִ���ʱ���ŵ���parse_it();
	*/

	/*
	���ݺ�����token�Ϳ����ж���
	*/
	bool hasVarDec;

	Token token1 = getToken();
	Token token2 = getToken();

	if (token1.text == "int" || token1.text == "float" || token1.text == "char" ||
		token1.text == "boolean" || token1.text == "void") {
		hasVarDec = true;
	} else {
		if (token1.type != ID) { //���ǻ������ͣ��ֲ�������ID���򲻿�����var dec
			hasVarDec = false;
		} else {
			if (token2.type == ID) { //�������� ID�� token�ģ�ֻ�����Ǳ��������� Man man
				hasVarDec = true;
			} else {
				hasVarDec = false;
			}
		}
	}
	ungetToken();
	ungetToken();

	return hasVarDec;
}

Parser::TreeNode * Parser::getSyntaxTree() {
	return nullptr;
}

Parser::TreeNode* Parser::parse_program() {
	syntaxTree = parse_class_list();
	return syntaxTree;
}



Parser::TreeNode* Parser::parse_class_list() {
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	TreeNode* bigtree;
	for (file: files) {
		// TreeNode* tree = parse_class(file);
		1. Retarget file   
		2. call parse_class;

		add tree to bigtree;(����)
	}
	return bigtree;
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	auto bigtree = new TreeNode();
	auto lasttree = bigtree;

	auto fileIter = this->files.cbegin();
	while (fileIter != files.end()) {
		currentFile = *fileIter;
		scanner->openFile(currentFile);
		auto tree = parse_class(); //���Ϊ�޲Σ�parse_classҪͨ��״̬ȡ��parse���ļ�

		lasttree->next = tree;
		lasttree = tree;

		++fileIter;
	}

	return bigtree;
}

Parser::TreeNode* Parser::parse_class() {
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	parse this.currentFile (scanner already bind);
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	auto tree = new TreeNode();
	tree->nodekind = CLASS_K;

	//'class'
	Token token = getToken();
	//r-value reference can match l-value reference param.
	Token&& expectedToken = Token(KEYWORD,"class"); 
	if (token != expectedToken) {
		unExpectedToken(token, expectedToken);
	}

	//class-id
	token = getToken();
	//�~�~�~�~�~�~�~�~�~�~�~�~  initialize son-node:   child[0] : �����Ǹ�token
	tree->child[0] = new TreeNode();
	tree->child[0]->token = token;  //TODO tree.child[0] == nullptr ???

	//'{'
	token = getToken();

	//var-dec_list
	tree->child[1] = parse_class_var_dec_list();

	//subroutine-dec list
	tree->child[2] = parse_subroutine_dec_list();

	//'}'
	token = getToken();

	return tree;
}



Parser::TreeNode* Parser::parse_class_var_dec_list() {
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	@ע�⣺��ǰȡtoken�����жϺ���rollback

	if(���������Ǳ�������) ��
		list-node. type = EMPTY;
		
		return;
	��else {
		list-node.type = LIST;
		do {
			get var-dec node;
			add to list-node;
		} while (�������Ǳ�������);
	}
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	TreeNode* head = nullptr;

	Token token = getToken();
	auto tok_static = Token(KEYWORD,"static");
	auto tok_field = Token(KEYWORD,"field");

	if (token!= tok_static && token!= tok_field) {
		head = new TreeNode();
		head->nodekind = NodeKind::EMPTY_LIST_K;
		ungetToken();
		return head;

	} else {
		TreeNode* item = nullptr;
		TreeNode* end = nullptr;
		do {
			if (!head) { //first one
				head->nodekind = NodeKind::CLASS_VAR_DEC_LIST_K;
				head = parse_class_var_dec();
				end = head;
			} else {
				item = parse_class_var_dec();
				end->next = item;
				end = item;
			}
			token = getToken();
		} while ( !(token != tok_static && token != tok_field) );
		ungetToken();
	}

	return head;
}

Parser::TreeNode* Parser::parse_class_var_dec() {
	TreeNode* tree = new TreeNode;
	tree->nodekind = VAR_DEC_K;
	tree->child[0] = tree->child[1] = tree->child[2] = new TreeNode();

	//static | field
	Token token = getToken();
	tree->child[0]->token = token;

	//type
	tree->child[1] = parse_type();

	//varNameList
	tree->child[2] = parse_var_name_list();

	return tree;
}

/*
List��ͨ���㷨��

	tree; last; incoming;

	while(true){
		incoming = ��parse_item();

		if(tree = null) { //first
			tree = last = incoming;
			tree.next = last;
		} else{
			last.next = incoming;
			last = incoming;
		}
		
		if(�ý�����)��
			break;
		��
	}
*/

//TODO ��������ʽ��parse���󣡣���
TreeNode* Parser::parse_var_name_list() {
	TreeNode* tree = nullptr;
	TreeNode* last = nullptr;
	TreeNode* incoming = nullptr;

	Token token;
	Token end_tok = Token(SYMBOL, ",");
	while (true) {

		token = getToken();

		if (!tree) {
			tree = new TreeNode();
			tree->token = token;
			last = tree;
		} else {
			incoming = new TreeNode();
			incoming->token = token;

			last->next = incoming;
			last = incoming;
		}
		
		token = getToken(); // pass ','
		if ( !(token!= end_tok)) {
			break;
		}
	}

	return tree;
}

TreeNode* Parser::parse_type() {
	TreeNode* tree = new TreeNode;

	Token token = getToken();
	//TODO У���������������
	tree->token = token;
	return tree;
}

// Copy �� class_var_dec_list
TreeNode* Parser::parse_subroutine_dec_list() {
	TreeNode* tree = nullptr;

	Token token = getToken();
	auto tok1 = Token(KEYWORD, "constructor");
	auto tok2 = Token(KEYWORD, "method");
	auto tok3 = Token(KEYWORD, "function");

	if (token != tok1 && token != tok2 && token!= tok3) {
		tree = new TreeNode();
		tree->nodekind = NodeKind::EMPTY_LIST_K;
		ungetToken();
		return tree;

	} else {
		TreeNode* incoming = nullptr;
		TreeNode* end = nullptr;
		do {
			if (!tree) { //first one
				tree->nodekind = NodeKind::SUBROUTINE_DEC_LIST_K;
				tree = parse_subroutin_dec();
				end = tree;
			} else {
				incoming = parse_subroutin_dec();
				end->next = incoming;
				end = incoming;
			}
			token = getToken();
		} while (!(token != tok1 && token != tok2 && token != tok3));
		ungetToken();
	}
	return tree;
}

TreeNode* Parser::parse_subroutin_dec() {
	TreeNode* tree = new TreeNode;

	// constructor | function | method
	tree->child[0]->token = getToken();

	// ����
	tree->child[1] = parse_type();

	// ������
	tree->child[2]->token = getToken();

	//(
	getToken();
	//params
	tree->child[3] = parse_params();
	//)
	getToken();

	//body
	tree->child[4] = parse_subroutine_body();

	return tree;
}

/*
�����ǿյģ��� param_list���Բ�Ϊ��Ϊǰ��ģ���������һ��Ҫ�ж�
*/
TreeNode* Parser::parse_params() {
	TreeNode* tree = nullptr;

	Token token = getToken();
	Token right_bracket(SYMBOL,")");
	if (token != right_bracket) {
		ungetToken();
		tree = parse_param_list();
	}else {
		tree->nodekind = EMPTY_LIST_K;
		return tree;
	}

	return tree;
}

/*
List-Node��head�ڵ�� NodeType����Ϊ **List !! ��Ϊ��ֱ�����Ӹ��ڵ㣬����list����ɷ֡�
*/
TreeNode* Parser::parse_param_list() {
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out= nullptr;

	Token right_bracket(SYMBOL, ")"); 

	while (true) {
		out = parse_param();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = PARAM_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//�жϽ���ѭ��
		Token token = getToken(); //pass ','
		if (token == right_bracket) {
			ungetToken();
			break;
		}
	}

	return head;
}

TreeNode* Parser::parse_param() {
	TreeNode* tree = new TreeNode;

	//type
	tree->child[1] = parse_type();
	//param name: ID
	tree->child[2]->token = getToken();

	return tree;
}

TreeNode* Parser::parse_subroutine_body() {
	TreeNode* tree = new TreeNode;

	//{
	Token token = getToken();
	
	//varDecList
	tree->child[0] = parse_var_dec_list();

	//statements
	tree->child[1] = parse_statements();

	//}
	getToken();

	return tree;
}

TreeNode* Parser::parse_var_dec_list() {
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	if (following is not var dec) {
		return EMPTY;
	} else {
		head (LIST-NODE)  -> # -> #
	}
	-------------------------------------------
	@ how to judge ?
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	//TODO ����˼����ط���ƾ��һ��token�޷��ж��治���ڱ�������������Ӧtype,��type������ID��
	/*
	��û�б���������������statementҲ������ID��ͷ
	�� ��νLR(0)�ķ���1��tokenȷ����һ��ƥ��Ĳ���ʽ��
			LR(0)�������ˣ����ķ�����LR(0)�ģ�����

	�� ��Ȼ��Ҳ��������ȥ��������token���жϡ��ж���rollback����
	�� ����˵��Ҫ������������ �Ծ�parse_item()�������᷵�ظ���
	//TODO ����漰�����������һ��ԭ������ˣ�ֻ�� ��ȷ���˿��п��޵ĳɷִ���ʱ���ŵ���parse_it();
	*/
	
	bool hasVarDec = isFollowing_varDec_or_stat();

	//------------- �ж���ϣ�����parse ---------------------
	TreeNode* head = new TreeNode();

	if (!hasVarDec) {
		head->nodekind = EMPTY_LIST_K;
		return head;
	} else {
		TreeNode* end = nullptr;
		TreeNode* out = nullptr;

		Token semicolon(SYMBOL, ";");

		while (true) {
			out = parse_var_dec();

			if (head == nullptr) {
				head = end = out;
				head->nodekind = VAR_DEC_LIST_K;
			} else {
				end->next = out;
				end = out;
			}

			//�жϽ���ѭ��: ��statement�˲Ž�����
			if (!isFollowing_varDec_or_stat()) {
				break;
			}
		}
		
	}

	return head;
}

TreeNode* Parser::parse_var_dec() {
	TreeNode* tree = new TreeNode;

	tree->child[0]->token = getToken();
	tree->child[1] = parse_var_name_list();
	getToken(); //pass ';'

	return tree;
}

//TODO wocao��Ϊʲô������Ϊstatement_list����������multi ��listͳһ
TreeNode* Parser::parse_statements() {
	TreeNode* head = nullptr;

	Token token = getToken();
	Token tok_semicolon = Token(SYMBOL, ";");
	if (token == tok_semicolon) {
		ungetToken();
		head = new TreeNode;
		head->nodekind = EMPTY_LIST_K;
	} else {
		ungetToken();

		TreeNode* end = nullptr;
		TreeNode* out = nullptr;
		while (true) {
			out = parse_statement();

			if (head==nullptr) {
				head = end = out;
				head->nodekind = STATEMENT_LIST_K;
			} else {
				end->next = out;
				end = out;
			}

			if (getToken() == tok_semicolon) {
				ungetToken();
				break;
			}
		}
	}

	return head;
}

TreeNode* Parser::parse_statement() {
	TreeNode* tree = nullptr;

	Token tok_if(KEYWORD, "if");
	Token tok_while(KEYWORD, "while");
	Token tok_return(KEYWORD, "return");

	Token token = getToken();
	token = getToken();
	bool isAssign = false;
	if (token.text == "=" || token.text == "[") {
		isAssign = true;
	}
	ungetToken();
	ungetToken();

	token = getToken();
	if (token == tok_if) {
		ungetToken();
		tree = parse_if_statement();
	} else if (token == tok_while) {
		ungetToken();
		tree = parse_while_statement();
	} else if (token == tok_return) {
		ungetToken();
		tree = parse_return_statement();
	} else if (isAssign) {
		ungetToken();
		tree = parse_assign_statement();
	} else {
		ungetToken();
		tree = parse_call_statement();
	}

	return tree;
}

TreeNode* Parser::parse_assign_statement() {
	TreeNode* tree = new TreeNode;

	tree->child[0] = parse_left_value();
	getToken();
	tree->child[1] = parse_expression();
	getToken();

	return tree;
}

TreeNode* Parser::parse_left_value() {
	TreeNode* tree = new TreeNode;

	//-----------  peek to judge kind
	bool isArrayItem = false;
	Token token = getToken();
	token = getToken();
	Token t = Token(SYMBOL, "[");
	if (token == t) {
		isArrayItem = true;
	}
	ungetToken();
	ungetToken();

	//------------ parse
	if (!isArrayItem) {
		tree->nodekind = LEFT_VALUE_K;
		tree->token = getToken();
	} else {
		tree->nodekind = LEFT_VALUE_ARRAY_K;
		tree->child[0]->token = getToken();
		getToken(); // pass [
		tree->child[1]->token = getToken();
		getToken();// pass ]
	}

	return tree;
}

TreeNode* Parser::parse_if_statement() {
	TreeNode* tree = new TreeNode;

	Token tok_else = Token(KEYWORD, "else");

	getToken();
	getToken();
	tree->child[0] = parse_expression();
	getToken();
	tree->child[1] = parse_statement(); //if-body��ֻ֧��һ��statement������
	auto token = getToken();
	if (token == tok_else) {
		getToken(); //pass {
		tree->child[2] = parse_statement();
		getToken(); //pass }
	} else {
		ungetToken();
	}

	return tree;
}

TreeNode* Parser::parse_while_statement() {
	TreeNode* tree = new TreeNode;

	getToken();
	getToken();
	tree->child[0] = parse_expression();
	getToken();
	getToken();
	tree->child[1] = parse_statement();
	getToken();

	return tree;
}

TreeNode* Parser::parse_return_statement() {
	TreeNode* tree = new TreeNode;

	getToken();
	Token token = getToken();
	if (token.text != ";") {
		ungetToken();
		tree->child[0] = parse_expression(); //ͨ���Ƿ�child[0] == nullptr������ �Ƿ��return;
	} else {
		// do nothing
	}

	return tree;
}

TreeNode* Parser::parse_call_statement() {
	TreeNode* tree = new TreeNode;

	tree->child[0]->token = getToken();
	Token token = getToken();
	if (token.text==".") {
		tree->nodekind = CALL_STAT_MEMEBR_K;
		tree->child[1]->token = getToken();
		getToken();
		tree->child[2] = parse_expressions();
		getToken();

	}else {
		tree->nodekind = CALL_STAT_K;
		tree->child[1] = parse_expressions();
		getToken();
	}

	return tree;
}

// ͬ parse_params();

TreeNode* Parser::parse_expressions() {
	TreeNode* tree = nullptr;

	Token token = getToken();
	Token right_bracket(SYMBOL, ")");
	if (token != right_bracket) {
		ungetToken();
		tree = parse_expression_list();
	} else {
		tree->nodekind = EMPTY_LIST_K;
		return tree;
	}

	return tree;
}

TreeNode* Parser::parse_expression_list() {
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	Token right_bracket(SYMBOL, ")");

	//TODO ����list�Ľ������Գ�ɹ����ķ����� �ṩ3�������� �ٻص���parse���� �ڽ������� �� NodeKind
	while (true) {
		out = parse_expression();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = EXPRESSION_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//�жϽ���ѭ��: ������һ��Node
		Token token = getToken(); //pass ','
		if (token == right_bracket) {
			ungetToken();
			break;
		}
	}

	return head;
}

//TODO �е�����

TreeNode* Parser::parse_expression() {
	//ʵ�ʾ��ǣ�~n��boolExpression ����/��
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//���ų���Ҳ����ѽ��������
	Token tok_and(SYMBOL, "&");
	Token tok_or(SYMBOL, "|");

	//TODO ����list�Ľ������Գ�ɹ����ķ����� �ṩ3�������� �ٻص���parse���� �ڽ������� �� NodeKind
	while (true) {
		out = parse_bool_expression();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = BOOL_EXPRESSION_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//�жϽ���ѭ��: ������һ��Node
		Token token = getToken(); //pass ','
		if (token != tok_and && token!= tok_or) {
			ungetToken();
			break;
		}
	}

	return head;
}

TreeNode* Parser::parse_bool_expression() {
	TreeNode* tree = new TreeNode;

	tree->child[0] = parse_additive_expression();
	Token token = getToken();
	if (token.text!=")") {
		tree->child[1]->token = token;
		tree->child[2] = parse_additive_expression();
	} else {
		ungetToken();
	}

	return tree;
}

TreeNode* Parser::parse_additive_expression() {
	//ʵ�ʾ��ǣ�~n��term�� + -
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//���ų���Ҳ����ѽ��������
	Token tok_add(SYMBOL, "+");
	Token tok_dec(SYMBOL, "-");

	while (true) {
		out = parse_term();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = TERM_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//�жϽ���ѭ��: ������һ��Node
		Token token = getToken(); //pass ','
		if (token != tok_add && token != tok_dec) {
			ungetToken();
			break;
		}
	}

	return head;
}

//TODO �Բۣ�û������������
TreeNode* Parser::parse_term() {
	//ʵ�ʾ��ǣ�~n��factor�� * /
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//���ų���Ҳ����ѽ��������
	Token tok_mul(SYMBOL, "*");
	Token tok_div(SYMBOL, "/");

	while (true) {
		out = parse_factor();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = FACTOR_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//�жϽ���ѭ��: ������һ��Node
		Token token = getToken(); //pass ','
		if (token != tok_mul && token != tok_div) {
			ungetToken();
			break;
		}
	}

	return head;
}

TreeNode* Parser::parse_factor() {
	// postive factor�ģ������ţ�
	TreeNode* tree = new TreeNode;

	Token token = getToken();
	if (token.text =="-") {
		tree = parse_positive_factor();
		tree->nodekind = NEGTIVE_FACTOR_K;
	} else {
		ungetToken();
		tree = parse_positive_factor();
		tree->nodekind = POSTIVE_FACTOR_K;
	}

	return tree;
}

TreeNode* Parser::parse_positive_factor() {
	TreeNode* tree = new TreeNode;

	Token token = getToken();
	if (token.text == "~") {
		tree = parse_not_factor();
		tree->nodekind = NOT_POSTIVE_FACTOR_K;
	} else {
		ungetToken();
		tree = parse_not_factor();
		tree->nodekind = NOT_NEGTIVE_FACTOR_K;
	}

	return tree;
}

//TODO ����ɷ֣��ٸ�������ɶ����
TreeNode* Parser::parse_not_factor() {
	TreeNode* tree = new TreeNode;

	Token token = getToken();
	if (token.type != ID && token.text!="(") {
		tree->token = token;
	}else if(token.text != "(") {
		tree->child[0] = parse_expression();
		getToken();
	} else {
		//��ID��
		token = getToken();
		if (token.text=="."|| token.text=="(") {
			ungetToken();
			ungetToken();
			tree->child[0] = parse_call_expression();
		} else if(token.text=="[") {
			ungetToken();
			ungetToken(); //TODO unget()Ӧ��������һ��ֵ������
			tree->child[0]->token = getToken();
			getToken();
			getToken();
			tree->child[1] = parse_expression();
			getToken();
		} else {
			tree->child[0]->token = getToken();
		}
	}

	return tree;
}

TreeNode* Parser::parse_call_expression() {
	//ͬcall statement
	TreeNode* tree = new TreeNode;

	tree->child[0]->token = getToken();
	Token token = getToken();
	if (token.text == ".") {
		tree->nodekind = CALL_EXPRESSION_MEMEBR_K;
		tree->child[1]->token = getToken();
		getToken();
		tree->child[2] = parse_expression();
		getToken();

	} else {
		tree->nodekind = CALL_EXPRESSION_K;
		tree->child[1] = parse_expression();
		getToken();
	}

	return tree;
}










