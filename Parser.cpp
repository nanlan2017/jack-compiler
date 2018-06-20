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

	//TODO  vector<> 未进行初始化，现在是null的？
	/*
	Dog dog;成员是如何进行初始化的？
		？对象构造时已经为它调用默认的构造函数了？
		？它的构造怎么调：  Dog dog(); ??   Dog d; dog=d;
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

		add tree to bigtree;(链表)
	}
	return bigtree;
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	auto bigtree = new TreeNode();
	auto lasttree = bigtree;

	auto fileIter = this->files.cbegin();
	while (fileIter != files.end()) {
		currentFile = *fileIter;
		scanner->openFile(currentFile);
		auto tree = parse_class(); //设计为无参，parse_class要通过状态取该parse的文件

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
	//▇▇▇▇▇▇▇▇▇▇▇▇  initialize son-node:   child[0] : 类名是个token
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
	@注意：提前取token进行判断后，需rollback

	if(接下来不是变量声明) ｛
		list-node. type = EMPTY;
		
		return;
	｝else {
		list-node.type = LIST;
		do {
			get var-dec node;
			add to list-node;
		} while (接下来是变量声明);
	}
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	TreeNode* tree = nullptr;

	Token token = getToken();
	auto tok_static = Token(KEYWORD,"static");
	auto tok_field = Token(KEYWORD,"field");

	if (token!= tok_static && token!= tok_field) {
		tree = new TreeNode();
		tree->nodekind = NodeKind::EMPTY_K;
		ungetToken();
		return tree;

	} else {
		TreeNode* incoming = nullptr;
		TreeNode* list_end = nullptr;
		do {
			if (!tree) { //first one
				tree->nodekind = NodeKind::CLASS_VAR_DEC_LIST_K;
				tree = parse_class_var_dec();
				list_end = tree;
			} else {
				incoming = parse_class_var_dec();
				list_end->next = incoming;
				list_end = incoming;
			}
			token = getToken();
		} while ( !(token != tok_static && token != tok_field) );
		ungetToken();
	}

	return tree;
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
List的通用算法：

	tree; last; incoming;

	while(true){
		incoming = ▲parse_item();

		if(tree = null) { //first
			tree = last = incoming;
			tree.next = last;
		} else{
			last.next = incoming;
			last = incoming;
		}
		
		if(该结束了)｛
			break;
		｝
	}
*/
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
	//TODO 校验变量声明的类型
	tree->token = token;
	return tree;
}

// Copy 自 class_var_dec_list
TreeNode* Parser::parse_subroutine_dec_list() {
	TreeNode* tree = nullptr;

	Token token = getToken();
	auto tok1 = Token(KEYWORD, "constructor");
	auto tok2 = Token(KEYWORD, "method");
	auto tok3 = Token(KEYWORD, "function");

	if (token != tok1 && token != tok2 && token!= tok3) {
		tree = new TreeNode();
		tree->nodekind = NodeKind::EMPTY_K;
		ungetToken();
		return tree;

	} else {
		TreeNode* incoming = nullptr;
		TreeNode* list_end = nullptr;
		do {
			if (!tree) { //first one
				tree->nodekind = NodeKind::SUBROUTINE_DEC_LIST_K;
				tree = parse_subroutin_dec();
				list_end = tree;
			} else {
				incoming = parse_subroutin_dec();
				list_end->next = incoming;
				list_end = incoming;
			}
			token = getToken();
		} while (!(token != tok1 && token != tok2 && token != tok3));
		ungetToken();
	}
	return tree;
}

TreeNode* Parser::parse_subroutin_dec() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_params() {
	TreeNode* tree = new TreeNode;

	return tree;
}
TreeNode* Parser::parse_param_list() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_param() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_subroutine_body() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_var_dec_list() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_var_dec() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_statements() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_assign_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_left_value() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_if_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_while_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_return_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_call_statement() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_expressions() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_expression_list() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_expression() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_bool_expression() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_additive_expression() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_term() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_factor() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_positive_factor() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_not_factor() {
	TreeNode* tree = new TreeNode;

	return tree;
}

TreeNode* Parser::parse_call_expression() {
	TreeNode* tree = new TreeNode;

	return tree;
}










