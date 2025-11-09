#ifndef SCOPE_ANALYZER_H
#define SCOPE_ANALYZER_H

#include "parse_tree.h"
#include <vector>
#include <unordered_map>
#include <iostream>

enum class ScopeError {
    UndeclaredVariable,
    UndefinedFunction,
    VariableRedefined,
    FunctionRedefined
};

class Scope {
public:
    Scope* parent;
    std::unordered_map<std::string, std::string> symbols;
    
    Scope(Scope* p = nullptr) : parent(p) {}
    
    bool add(const std::string& name, const std::string& type) {
        if (symbols.count(name)) return false;
        symbols[name] = type;
        return true;
    }
    
    bool in_scope(const std::string& name) {
        return symbols.count(name);
    }
    
    std::string find(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) return it->second;
        if (parent) return parent->find(name);
        return "";
    }
};

class ScopeAnalyzer {
    std::vector<ScopeError> errors;
    Scope* current;
    Scope* global;
    
    void error(ScopeError err, const std::string& name) {
        errors.push_back(err);
        std::string msg;
        switch(err) {
            case ScopeError::UndefinedFunction: msg = "Undefined function: " + name; break;
            case ScopeError::UndeclaredVariable: msg = "Undeclared variable: " + name; break;
            case ScopeError::VariableRedefined: msg = "Variable redefined: " + name; break;
            case ScopeError::FunctionRedefined: msg = "Function redefined: " + name; break;
        }
        std::cout << "Error: " << msg << std::endl;
    }
    
    void enter_scope() { current = new Scope(current); }
    void leave_scope() { Scope* old = current; current = current->parent; delete old; }
    
public:
    ScopeAnalyzer() {
        global = new Scope();
        current = global;
    }
    
    ~ScopeAnalyzer() {
        while (current != nullptr) {
            Scope* old = current;
            current = current->parent;
            delete old;
        }
    }
    
    bool check(ProgramNode* program) {
        // PHASE 1: Global declarations
        for (auto& var : program->globals) {
            if (!global->add(var.name, var.type)) {
                error(ScopeError::VariableRedefined, var.name);
            }
        }
        
        for (auto& func : program->functions) {
            if (!global->add(func.name, "function")) {
                error(ScopeError::FunctionRedefined, func.name);
            }
        }
        
        // PHASE 2: Function bodies
        for (auto& func : program->functions) {
            check_function(&func);
        }
        
        // PHASE 3: Global initializers
        for (auto& var : program->globals) {
            if (var.value) check_node(var.value.get());
        }
        
        return errors.empty();
    }
    
    const std::vector<ScopeError>& getErrors() const { return errors; }
    bool passed() const { return errors.empty(); }
    size_t errorCount() const { return errors.size(); }

private:
    void check_function(FunctionNode* func) {
        enter_scope();
        
        for (auto& param : func->params) {
            if (!current->add(param.name, param.type)) {
                error(ScopeError::VariableRedefined, param.name);
            }
        }
        
        if (func->body) check_node(func->body.get());
        
        leave_scope();
    }
    
    void check_node(ASTNode* node) {
        if (!node) return;
        
        if (auto block = dynamic_cast<BlockNode*>(node)) {
            enter_scope();
            for (auto& stmt : block->statements) {
                check_node(stmt.get());
            }
            leave_scope();
        }
        else if (auto var = dynamic_cast<VariableNode*>(node)) {
            if (current->in_scope(var->name)) {
                error(ScopeError::VariableRedefined, var->name);
            } else {
                current->add(var->name, var->type);
            }
            if (var->value) check_node(var->value.get());
        }
        else if (auto call = dynamic_cast<CallNode*>(node)) {
            if (global->find(call->name) != "function") {
                error(ScopeError::UndefinedFunction, call->name);
            }
            for (auto& arg : call->args) {
                check_node(arg.get());
            }
        }
        else if (auto name = dynamic_cast<NameNode*>(node)) {
            if (current->find(name->name).empty()) {
                error(ScopeError::UndeclaredVariable, name->name);
            }
        }
        else if (auto assign = dynamic_cast<AssignmentNode*>(node)) {
            check_node(assign->value.get());
            if (current->find(assign->name).empty()) {
                error(ScopeError::UndeclaredVariable, assign->name);
            }
        }
        else if (auto ret = dynamic_cast<ReturnNode*>(node)) {
            if (ret->value) check_node(ret->value.get());
        }
        else if (auto if_stmt = dynamic_cast<IfNode*>(node)) {
            check_node(if_stmt->condition.get());
            check_node(if_stmt->then_branch.get());
            if (if_stmt->else_branch) check_node(if_stmt->else_branch.get());
        }
        else if (auto while_stmt = dynamic_cast<WhileNode*>(node)) {
            check_node(while_stmt->condition.get());
            check_node(while_stmt->body.get());
        }
        else if (auto for_stmt = dynamic_cast<ForNode*>(node)) {
            enter_scope();
            if (for_stmt->initializer) check_node(for_stmt->initializer.get());
            if (for_stmt->condition) check_node(for_stmt->condition.get());
            if (for_stmt->increment) check_node(for_stmt->increment.get());
            if (for_stmt->body) check_node(for_stmt->body.get());
            leave_scope();
        }
        else if (auto binary = dynamic_cast<BinaryOpNode*>(node)) {
            check_node(binary->left.get());
            check_node(binary->right.get());
        }
        // LiteralNode doesn't need checking - always valid
    }
};

#endif