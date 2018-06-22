#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include "Parser.h"
#include "SymbolTable.h"

using namespace std;
/*****************************************************
�﷨���ģ�飺\n
	��Ϸ��ű����﷨���ϸ��ڵ��Value���м�顣

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

	void checkMain();                        // ���Program��ڣ�Main.main()
	void checkStatements(TreeNode* t);       // ��麯���е���� (1.��ֵ 2.if 3.while 4.return��� 5.call��������)
	void checkStatment(TreeNode* t);	     // ���
	void checkExpression(TreeNode* t);	     // �����ʽ (��foo[5]-foo��Ϊ����)
	void checkArguments(TreeNode* t);        // ��麯�����õ�ʵ�����������β������Ƿ�ƥ��

private:
	TreeNode* bigTree;                       // ��������Program���﷨��
	SymbolTable* table;                      // ���﷨���е����б�ʶ���ķ��ű�
};

#endif // _ANALYZER_H_