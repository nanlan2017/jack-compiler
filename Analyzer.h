#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include "Parser.h"
#include "SymbolTable.h"

using namespace std;
/*****************************************************
语法检查模块：\n
	结合符号表，对语法树上各节点的Value进行检查。

*****************************************************/
class Analyzer {
	using TreeNode = Parser::TreeNode;
	using NodeKind = Parser::NodeKind;
	using SymbolInfo = SymbolTable::Info;

public:

	Analyzer(TreeNode* tree):bigTree(tree) {
		table = SymbolTable::getSymbolTable();
		table->buildTable(tree);
	}

	void checkMain();                        // 检查Program入口：Main.main()
	void checkStatements(TreeNode* t);       // 检查函数中的语句 (1.赋值 2.if 3.while 4.return语句 5.call函数调用)
	void checkStatment(TreeNode* t);	     // 检查
	void checkExpression(TreeNode* t);	     // 检查表达式 (如foo[5]-foo须为数组)
	void checkArguments(TreeNode* t);        // 检查函数调用的实参与声明的形参类型是否匹配

private:
	TreeNode* bigTree;                       // 代表整个Program的语法树
	SymbolTable* table;                      // 该语法树中的所有标识符的符号表
};

#endif // _ANALYZER_H_