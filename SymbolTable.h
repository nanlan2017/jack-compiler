/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
用map的话就要保证key值不同，而这里key都是标识符字面值，这样没法确定啊。要不改用vector? 虽然查找效率就低了


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
		PARAM,// 函数签名中的形参名
		LOCAL,

	};

	struct Info {
		SymbolKind kind;
		int index;
		string type;
		//Info* params[];
		//TODO 看看这个map是不是 Info info; 后就能初始化成 空map
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
	//map<string, map<string, Info>> classesTable; // 每一个Class，其内部的n个变量信息是一个Map
	//map<string, map<string, Info>> subroutinesTable; // 每一个subroutine，其内部的n个变量信息是一个Map
	
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
