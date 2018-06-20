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

	//TODO 有意思！这地方光凭下一个token无法判定存不存在变量声明！（对应type,但type可能是ID）
	/*
	若没有变量声明，则后面的statement也可能以ID开头
	★ 所谓LR(0)文法：1个token确定下一个匹配的产生式？
	LR(0)不够用了！这文法不是LR(0)的？？？

	？ 当然我也可以连续去后面数个token来判断、判断完rollback就行
	？ 还是说我要整个换方案： 仍旧parse_item()，但它会返回给我
	//TODO 这就涉及到最初设计里的一个原则策略了：只在 已确定此可有可无的成分存在时，才调用parse_it();
	*/

	/*
	根据后两个token就可以判断了
	*/
	bool hasVarDec;

	Token token1 = getToken();
	Token token2 = getToken();

	if (token1.text == "int" || token1.text == "float" || token1.text == "char" ||
		token1.text == "boolean" || token1.text == "void") {
		hasVarDec = true;
	} else {
		if (token1.type != ID) { //不是基本类型，又不是类型ID，则不可能是var dec
			hasVarDec = false;
		} else {
			if (token2.type == ID) { //连续两个 ID型 token的，只可能是变量声明： Man man
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

//TODO 这条产生式的parse有误！！！
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

	// 类型
	tree->child[1] = parse_type();

	// 函数名
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
可能是空的，而 param_list是以不为空为前提的，所以在这一步要判断
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
List-Node的head节点的 NodeType必须为 **List !! 因为它直接连接父节点，代表list这个成分。
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

		//判断结束循环
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

	//TODO 有意思！这地方光凭下一个token无法判定存不存在变量声明！（对应type,但type可能是ID）
	/*
	若没有变量声明，则后面的statement也可能以ID开头
	★ 所谓LR(0)文法：1个token确定下一个匹配的产生式？
			LR(0)不够用了！这文法不是LR(0)的？？？

	？ 当然我也可以连续去后面数个token来判断、判断完rollback就行
	？ 还是说我要整个换方案： 仍旧parse_item()，但它会返回给我
	//TODO 这就涉及到最初设计里的一个原则策略了：只在 已确定此可有可无的成分存在时，才调用parse_it();
	*/
	
	bool hasVarDec = isFollowing_varDec_or_stat();

	//------------- 判断完毕，进行parse ---------------------
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

			//判断结束循环: 到statement了才结束！
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

//TODO wocao，为什么不命名为statement_list，和其他的multi 的list统一
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
	tree->child[1] = parse_statement(); //if-body里只支持一条statement啊？！
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
		tree->child[0] = parse_expression(); //通过是否child[0] == nullptr来区别 是否空return;
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

// 同 parse_params();

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

	//TODO 这种list的解析可以抽成公共的方法： 提供3个参数： ①回调的parse函数 ②结束条件 ③ NodeKind
	while (true) {
		out = parse_expression();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = EXPRESSION_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//判断结束循环: 碰到下一种Node
		Token token = getToken(); //pass ','
		if (token == right_bracket) {
			ungetToken();
			break;
		}
	}

	return head;
}

//TODO 有点特殊

TreeNode* Parser::parse_expression() {
	//实际就是！~n个boolExpression 的且/或
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//用排除法也可以呀！！！！
	Token tok_and(SYMBOL, "&");
	Token tok_or(SYMBOL, "|");

	//TODO 这种list的解析可以抽成公共的方法： 提供3个参数： ①回调的parse函数 ②结束条件 ③ NodeKind
	while (true) {
		out = parse_bool_expression();

		if (head == nullptr) {
			head = end = out;
			head->nodekind = BOOL_EXPRESSION_LIST_K;
		} else {
			end->next = out;
			end = out;
		}

		//判断结束循环: 碰到下一种Node
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
	//实际就是！~n个term的 + -
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//用排除法也可以呀！！！！
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

		//判断结束循环: 碰到下一种Node
		Token token = getToken(); //pass ','
		if (token != tok_add && token != tok_dec) {
			ungetToken();
			break;
		}
	}

	return head;
}

//TODO 卧槽，没保存计算符啊！
TreeNode* Parser::parse_term() {
	//实际就是！~n个factor的 * /
	TreeNode* head = nullptr;
	TreeNode* end = nullptr;
	TreeNode* out = nullptr;

	//用排除法也可以呀！！！！
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

		//判断结束循环: 碰到下一种Node
		Token token = getToken(); //pass ','
		if (token != tok_mul && token != tok_div) {
			ungetToken();
			break;
		}
	}

	return head;
}

TreeNode* Parser::parse_factor() {
	// postive factor的（带符号）
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

//TODO 这个成分，举个例子是啥？？
TreeNode* Parser::parse_not_factor() {
	TreeNode* tree = new TreeNode;

	Token token = getToken();
	if (token.type != ID && token.text!="(") {
		tree->token = token;
	}else if(token.text != "(") {
		tree->child[0] = parse_expression();
		getToken();
	} else {
		//是ID了
		token = getToken();
		if (token.text=="."|| token.text=="(") {
			ungetToken();
			ungetToken();
			tree->child[0] = parse_call_expression();
		} else if(token.text=="[") {
			ungetToken();
			ungetToken(); //TODO unget()应当返回上一个值！！！
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
	//同call statement
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










