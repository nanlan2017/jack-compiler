#ifndef _PARSER_H_
#define _PARSER_H_

/**********************************************************************************
�²�ӿڣ� 
	sanner.openfile()��һ��jack�ļ��󣬾Ϳ���nextToken()��ͣȡtoken�������Ļ����һ���� EOF

��ģ�鹦�ܣ�
��һ��program(���jack Class�ļ�) parse��һ���﷨����
	1. ָ���ļ���parse_program()��getSyntaxTree�õ��﷨����  
	   �������쳣�������������Ϣ���ļ�-����-�ַ���
	
**********************************************************************************/
#include "Tokenizer.h"

#include <vector>
#include <string>
#include <deque>

using namespace std;

class Parser {
public:
	/*
	ֻҪ���� �ӳɷ��еķ��ս��Node��Type�����ˣ�
			�������ӽڵ�Type��ȷ�������ʽ��ȷ�����﷨��parse��ȷ��
	�~�~�~�~�﷨���:  ��������Node-Type����Node��value
	*/
	enum NodeKind {
		/*
		���壺List��Ԫ�ء�
		�����ڣ�List��Ԫ�أ���NodeType = XxList
		�÷�������List nodeʱ���жϣ���Ϊempty�ٶ�listֵ��if(node.type != EMPTY)
		*/
		EMPTY_LIST_K,

		TOKEN_K, //�ս�������;�Ϊtoken

		NONE_K, // ? 

		//class
		CLASS_K,
		CLASS_VAR_DEC_LIST_K, VAR_DEC_K,
		SUBROUTINE_DEC_LIST_K,

		//subroutine 
		SUBROUTINE_DEC_K,
		PARAM_LIST_K,
		SUBROUTINE_BODY_K,
		VAR_DEC_LIST_K,
		STATEMENT_LIST_K,

		LEFT_VALUE_K, LEFT_VALUE_ARRAY_K,
		CALL_STAT_K,CALL_STAT_MEMEBR_K,

		EXPRESSION_LIST_K,
		BOOL_EXPRESSION_LIST_K,
		TERM_LIST_K,
		FACTOR_LIST_K,

		POSTIVE_FACTOR_K,
		NEGTIVE_FACTOR_K,

		NOT_POSTIVE_FACTOR_K,
		NOT_NEGTIVE_FACTOR_K,

		CALL_EXPRESSION_K, CALL_EXPRESSION_MEMEBR_K,
	};

	/*
	�����ݽṹֻҪ�ܱ�֤���������ˡ�
		
		child�� �ӳɷ֣�����ʽ�Ҳ�� 
		token�����˽ڵ�Ϊ�ս����Leaf������ֱ�Ӵ��ڡ�
		next��list�е�Ԫ���ô�ָ���γ�����
		nodekind: ��Ҫʱ���ϡ�
	*/
	struct TreeNode
	{
		NodeKind nodekind;
		TreeNode* next;
		TreeNode* child[5];
		Token token;

		TreeNode() {
			nodekind = NONE_K;
			next = nullptr;
			child[0] = child[1] = child[2] = child[3] = child[4] = nullptr;
		}

		TreeNode(Token token):token(std::move(token)) {
			nodekind = TOKEN_K;
		}
	};

	/*--------------------------------------------------------------------*/

	Parser(vector<string>& filenames);

	TreeNode* parse_program();
	TreeNode* getSyntaxTree();
	void print_tree();

	/*--------------------------*/

private:
	vector<string> files;
	Tokenizer* scanner;
	TreeNode* syntaxTree;

	/*--------------------------*/

	string currentFile;
	deque<Token> tokenBuf_used;
	deque<Token> tokenBuf_waiting;

	Token& getToken();
	void ungetToken();

	bool isFollowing_varDec_or_stat();

	//��������������������������������������������������������������������������������������������������������������������������������������������������������//
	/*
	program -> classlist

	classlist -> classlist class
	| class
	*/
	TreeNode * parse_class_list();
	/*
	class -> class ID { classVarDecList subroutineDecList }
	*/
	TreeNode * parse_class();

	/*
	classVarDecList -> classVarDecList classVarDec
	|
	*/
	TreeNode * parse_class_var_dec_list();
	/*
	classVarDec -> static type varNameList ;
	| field type varNameList ;
	*/
	TreeNode * parse_class_var_dec();
	/*
	varNameList -> varNameList , ID
	| ID
	*/
	TreeNode * parse_var_name_list();
	/*
	type -> int
	| float
	| char
	| boolean
	| void
	| ID
	*/
	TreeNode * parse_type();
	//- -- - -- - -- - - - -- - - - - -- - - -- - -- - -- - - -- - -- - - - -- 
	/*
	subroutineDecList -> subroutineDecList subroutineDec
	|
	*/
	TreeNode * parse_subroutine_dec_list();
	/*
	subroutineDec -> constructor type ID ( params ) subroutineBody
	| function type ID ( params ) subroutineBody
	| method type ID (params ) subroutineBody
	*/
	TreeNode * parse_subroutin_dec();
	/*
	params -> paramList
	|
	*/
	TreeNode * parse_params();
	/*
	paramList -> paramList , param
	| param
	*/
	TreeNode * parse_param_list();
	/*
	param -> type ID
	*/
	TreeNode * parse_param();
	/*
	subroutineBody -> { varDecList statements }
	*/
	TreeNode * parse_subroutine_body();
	/*
	varDecList -> varDecList varDec
	|
	*/
	TreeNode * parse_var_dec_list();
	/*
	varDec -> type varNameList ;
	*/
	TreeNode * parse_var_dec();

	//- -- - -- - -- - - - -- - - - - -- - - -- - -- - -- - - -- - -- - - - -- 
	/*
	statements -> statements statement
	|
	*/
	TreeNode * parse_statements();
	/*
	statement -> assign_statement
	| if_statement
	| while_statement
	| return_statement
	| call_statement ;
	*/
	TreeNode * parse_statement();
	/*
	assign_statement -> leftValue = expression ;
	*/
	TreeNode * parse_assign_statement();
	/*
	leftValue -> ID
	| ID [ expression ]
	*/
	TreeNode * parse_left_value();
	/*
	if_statement -> if ( expression ) statement
	| if ( expression ) statement else statement
	*/
	TreeNode * parse_if_statement();
	/*
	while_statement -> while ( expression ) { statement }
	*/
	TreeNode * parse_while_statement();
	/*
	return_statement -> return ;
	| return expression ;
	*/
	TreeNode * parse_return_statement();
	/*
	call_statement -> ID ( expressions )
	| ID . ID ( expressions )
	*/
	TreeNode * parse_call_statement();

	//- -- - -- - -- - - - -- - - - - -- - - -- - -- - -- - - -- - -- - - - -- 
	/*
	expressions -> expression_list
	|
	*/
	TreeNode * parse_expressions();
	/*
	expression_list -> expression_list , expression
	| expression
	*/
	TreeNode * parse_expression_list();
	/*
	expression -> expression & boolExpression
	| expression | boolExpression
	| boolExpression
	*/
	TreeNode * parse_expression();
	/*
	boolExpression -> additive_expression relational_operator additive_expression
	| additive_expression
	*/
	TreeNode * parse_bool_expression();
	/*
	additive_expression -> additive_expression + term
	| additive_expression �C term
	| term
	*/
	TreeNode * parse_additive_expression();

	//- -- - -- - -- - - - -- - - - - -- - - -- - -- - -- - - -- - -- - - - -- 
	/*
	term -> term * factor
	| term / factor
	| factor
	*/
	TreeNode * parse_term();
	/*
	factor -> - positive_factor
	| positive_factor
	*/
	TreeNode * parse_factor();
	/*
	positive_factor -> ~ not_factor
	| not_factor
	*/
	TreeNode * parse_positive_factor();
	/*
	not_factor -> INT_CONST
	| CHAR_CONST
	| STRING_CONST
	| keywordConstant
	| ID
	| ID [ expression ]
	| call_expression
	| ( expression )
	*/
	TreeNode * parse_not_factor();
	/*
	call_expression -> ID ( expression )
	| ID . ID ( expression )
	*/
	TreeNode * parse_call_expression();
};

#endif


