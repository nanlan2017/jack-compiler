/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
��map�Ļ���Ҫ��֤keyֵ��ͬ��������key���Ǳ�ʶ������ֵ������û��ȷ������Ҫ������vector? ��Ȼ����Ч�ʾ͵���


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include "Parser.h"

#include <vector>
#include <string>
#include <map>

using namespace std;

class SymbolTable
{
public:
	using NodeKind = Parser::NodeKind;
	using TreeNode = Parser::TreeNode;

	struct ClassScope;

	enum SymbolKind {
		NONE,

		FIELD, STATIC,

		CONSTRUCTOR, FUCTION, METHOD,

		THIS,
		PARAM,// ����ǩ���е��β���
		LOCAL,

	};

	struct Info {
		SymbolKind kind;
		int index;
		string type;
		//Info* params[];
		//TODO �������map�ǲ��� Info info; ����ܳ�ʼ���� ��map
		//map<string, int> test;

		Info() {
			kind = NONE;
			index = -1;
			type = "0";
		}
	};
	
	struct SubroutineScope {
		map<string, Info> vars;
		ClassScope* parentEnv;
	};

	struct ClassScope
	{
		map<string, Info> vars;
		map<string, SubroutineScope*> subroutines;
	};
	
public:
	//map<string, map<string, Info>> classesTable; // ÿһ��Class�����ڲ���n��������Ϣ��һ��Map
	//map<string, map<string, Info>> subroutinesTable; // ÿһ��subroutine�����ڲ���n��������Ϣ��һ��Map
	
	static SymbolTable* getSymbolTable();
	void buildTable(Parser::TreeNode* syntaxTree);

	Info lookupClassVar(string varid,string className);
	Info lookupSubroutine(string subroutineID, string className);
	Info lookupSubroutineVar(string varid, string className,string subroutineName);

private:
	static SymbolTable* symbolTable;
	SymbolTable() = default;

	map<string, ClassScope*> programTable;

	ClassScope* scanClass(TreeNode* class_node);
	map<string, Info> scanSubroutineBody(TreeNode* node);


	inline void resetClassIndex() {
		static_index = -1;
		field_index = -1;

		constructor_index = -1;
		function_index = -1;
		method_index = -1;
	}
	inline void resetSubroutineIndex() {
		local_index = -1;
		param_index = -1;
	}

	int static_index;
	int field_index;
	int constructor_index;
	int function_index;
	int method_index;
	int local_index;
	int param_index;
};


#endif // _SYMBOL_TABLE_
