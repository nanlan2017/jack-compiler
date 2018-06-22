#include "SymbolTable.h"

void __usecase__symbolTable___() {
	Parser::TreeNode* syntaxTree = nullptr;
	SymbolTable* symbolTable = SymbolTable::getSymbolTable();
	symbolTable->buildTable(syntaxTree);

	//访问符号表
}

SymbolTable* SymbolTable::getSymbolTable() {
	if (symbolTable==nullptr) {
		symbolTable = new SymbolTable();
	}
	return symbolTable;
}

/*
build 'programTable'
*/
void SymbolTable::buildTable(TreeNode* syntaxTree) {
	TreeNode* t = syntaxTree;
	while (t != nullptr) {
		/*
		scan this Class-node:
			child[0] = classid;
			child[1] = classVarDecList;
			child[2] = subroutineDecList
		*/
		string classid = t->child[0]->token.text;
		ClassScope* scope = scanClass(t);
		programTable.insert({classid,scope});


		// go to next Class-node
		t = t->next;
	}
}

/*
Class-node:
	child[0] = classid;
	child[1] = classVarDecList;
	child[2] = subroutineDecList
*/
SymbolTable::ClassScope * SymbolTable::scanClass(TreeNode * class_node) {

	ClassScope* classScope = nullptr;

	if (class_node->nodekind==NodeKind::CLASS_K) {
		classScope = new ClassScope();
		string classname = class_node->child[0]->token.text;
		resetClassIndex();

		/*************************************************************************
		ClassvarDec:
			0: static/field
			1: type
			2: varnameList
		*/
		for (auto dec = class_node->child[1]; 
					dec != nullptr && dec->nodekind !=NodeKind::EMPTY_LIST_K; dec= dec->next) { // i:一行成员变量声明语句： static int i,j,k;

			// scan varDec：1. kind ,type (一行内的相同)   2. varid,index
			SymbolKind kind;
			if (dec->child[0]->token.text =="static") {
				kind = SymbolKind::STATIC;
			} else {
				kind = SymbolKind::FIELD;
			}
			string type = dec->child[1]->token.text;

			for (auto var = dec->child[2]; var!=nullptr;var=var->next) {
				string varid = var->token.text;        // 一个varid

				Info info;                             // 该varid的Info
				info.type = type;
				info.kind = kind;
				if (kind == STATIC) {
					info.index = ++static_index;
				} else {
					info.index = ++field_index;
				}

				classScope->vars.insert({varid,info});      //计入scope
			}
		}

		/*************************************************************************
		subroutineDec:
			0: constructor/function/method
			1: type
			2: id
			3. params
			4. subroutineBody
		*/
		for (auto dec = class_node->child[2]; 
						dec != nullptr&&dec->nodekind!=NodeKind::EMPTY_LIST_K;dec=dec->next) {

			SubroutineScope* subScope = new SubroutineScope();
			subScope->parentEnv = classScope;
			resetSubroutineIndex();
			//--------------- subroutine signature （放在class scope内）---------
			// kind
			SymbolKind kind;
			string child0 = dec->child[0]->token.text;
			if (child0 == "constructor") {
				kind = SymbolKind::CONSTRUCTOR;
			} else if(child0 =="method") {
				kind = SymbolKind::METHOD;
			} else {
				kind = SymbolKind::FUCTION;
			}
			// type
			string type = dec->child[1]->token.text;
			// function-id
			string subID = dec->child[2]->token.text;

			Info subinfo;
			subinfo.kind = kind;
			subinfo.type = type;
			switch (kind) {
				case CONSTRUCTOR: subinfo.index = ++constructor_index;	 break;
				case METHOD     : subinfo.index = ++method_index;		 break;
				case FUCTION    : subinfo.index = ++function_index;      break;
				default:break;
			}
			classScope->vars.insert({subID,subinfo});
			//------------  vars in subroutine （放在subroutine scope内）----------
			// this
			Info thisInfo;
			thisInfo.index = 0;
			thisInfo.kind = THIS;
			thisInfo.type = classname;

			subScope->vars.insert({"this",thisInfo });
			/*
			param:
				0: type
				1: id
			*/
			for (auto param = dec->child[3]; 
						param != nullptr&& param->nodekind!=NodeKind::EMPTY_LIST_K; param = param->next) {
				string paramID = param->child[1]->token.text;
				Info info;
				info.kind = PARAM;
				info.index = ++param_index;
				info.type = param->child[0]->token.text;

				subScope->vars.insert({paramID,info});
			}

			/*  locals */
			auto locals = scanSubroutineBody(dec->child[4]);
			for (auto iter = locals.begin; iter != locals.end;iter++) {
				subScope->vars.insert(iter);
			}
			//-------------------------------------
			classScope->subroutines.insert({subID,subScope});
		}
	}
	return classScope;
}

map<string, SymbolTable::Info> SymbolTable::scanSubroutineBody(TreeNode* node) {
	/*…………………………………………………………………………………………………………………………………………
	subroutineBody -> { varDecList statements }
    varDecList -> varDecList varDec
                | 
    varDec -> type varNameList ;
	varNameList -> varNameList , ID
				| ID
	…………………………………………………………………………………………………………………………………………*/
	map<string, Info> locals;
	if (node->nodekind==NodeKind::SUBROUTINE_BODY_K) {
		for (auto vardec = node->child[0]; 
					vardec != nullptr&& vardec->nodekind != NodeKind::EMPTY_LIST_K; vardec=vardec->next) {
			//type
			string type = vardec->child[0]->token.text;
			for (auto name = vardec->child[1];
					name != nullptr&& name->nodekind != NodeKind::EMPTY_LIST_K; name=name->next) {
				string localid = name->token.text;

				Info info;
				info.index = SymbolKind::LOCAL;
				info.type = type;
				info.index = ++local_index;

				locals.insert({localid,info});
			}
		}
	}
	return locals;
}


