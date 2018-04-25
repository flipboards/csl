#pragma once

#ifndef CSL_AST_BUILDER_H
#define CSL_AST_BUILDER_H

#include "ast.h"

class ExprASTBuilder {
public:

	ExprASTBuilder() :_rootast(nullptr), _curast(nullptr) {

	}

	void add_child(ExprAST* child) {
		if (!_check_init(child)) {
			_curast->add_child(child);
		}
	}

	void extend_child(ExprAST* child) {
		if (!_check_init(child)) {
			_curast->add_child(child);
			_curast = child;
		}
	}

	void extend_parent(ExprAST* parent) {
		if (!_check_init(parent)) {
			_curast->add_child(_rootast);
			_curast = parent;
			_rootast = parent;
		}
	}

	ExprAST* get_ast() {
		return _rootast;
	}
	
private:

	bool _check_init(ExprAST* ast) {
		if (_rootast == nullptr) {
			_rootast = ast;
			_curast = ast;
			return true;
		}
		else {
			return false;
		}
	}

	ExprAST * _rootast;
	ExprAST* _curast;
};


#endif