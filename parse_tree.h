#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include <string>
#include <vector>
#include <memory>

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct VariableNode : ASTNode {
    std::string type;
    std::string name;
    std::unique_ptr<ASTNode> value;
    
    VariableNode() = default;
    VariableNode(const std::string& t, const std::string& n) : type(t), name(n) {}
};

struct FunctionNode : ASTNode {
    std::string return_type;
    std::string name;
    std::vector<VariableNode> params;
    std::unique_ptr<ASTNode> body;
    
    FunctionNode() = default;
    FunctionNode(const std::string& ret, const std::string& n) : return_type(ret), name(n) {}
};

struct BlockNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
};

struct CallNode : ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
};

struct NameNode : ASTNode {
    std::string name;
    NameNode(const std::string& n) : name(n) {}
};

struct LiteralNode : ASTNode {
    std::string type;
    std::string value;
    LiteralNode(const std::string& t, const std::string& v) : type(t), value(v) {}
};

struct BinaryOpNode : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryOpNode(const std::string& o) : op(o) {}
};

struct AssignmentNode : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> value;
    AssignmentNode(const std::string& n) : name(n) {}
};

struct ReturnNode : ASTNode {
    std::unique_ptr<ASTNode> value;
};

struct IfNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> then_branch;
    std::unique_ptr<ASTNode> else_branch;
};

struct WhileNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;
};

struct ForNode : ASTNode {
    std::unique_ptr<ASTNode> initializer;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> increment;
    std::unique_ptr<ASTNode> body;
};

struct ProgramNode : ASTNode {
    std::vector<FunctionNode> functions;
    std::vector<VariableNode> globals;
};

#endif