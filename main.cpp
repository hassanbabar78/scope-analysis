#include "scope_analyzer.h"
#include <iostream>

int main() {
    std::cout << "=== COMPLETE SCOPE ANALYSIS TEST WITH ALL CASES ===" << std::endl;
    
    ProgramNode program;
    
    // =============================================
    // TEST CASE 1: Valid Code - All Correct Usage
    // =============================================
    std::cout << "\n📝 TEST 1: Valid Code (Should PASS)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code: int MAX_SIZE = 100;
    std::cout << "✓ Adding: int MAX_SIZE = 100;" << std::endl;
    program.globals.emplace_back("int", "MAX_SIZE");
    program.globals.back().value = std::make_unique<LiteralNode>("int", "100");
    
    // Code: float PI = 3.14;
    std::cout << "✓ Adding: float PI = 3.14;" << std::endl;
    program.globals.emplace_back("float", "PI");
    program.globals.back().value = std::make_unique<LiteralNode>("float", "3.14");
    
    // Code: int calculate(int a, int b) { return a * b; }
    std::cout << "✓ Adding: int calculate(int a, int b) { return a * b; }" << std::endl;
    program.functions.emplace_back("int", "calculate");
    FunctionNode& calculate_func = program.functions.back();
    calculate_func.params.emplace_back("int", "a");
    calculate_func.params.emplace_back("int", "b");
    
    auto calculate_body = std::make_unique<BlockNode>();
    auto return_stmt = std::make_unique<ReturnNode>();
    auto multiply = std::make_unique<BinaryOpNode>("*");
    multiply->left = std::make_unique<NameNode>("a");
    multiply->right = std::make_unique<NameNode>("b");
    return_stmt->value = std::move(multiply);
    calculate_body->statements.push_back(std::move(return_stmt));
    calculate_func.body = std::move(calculate_body);
    
    // Code: int main() { int x = 5; int y = calculate(x, 10); return y; }
    std::cout << "✓ Adding: int main() { int x = 5; int y = calculate(x, 10); return y; }" << std::endl;
    program.functions.emplace_back("int", "main");
    FunctionNode& main_func = program.functions.back();
    
    auto main_body = std::make_unique<BlockNode>();
    
    // int x = 5;
    auto x_var = std::make_unique<VariableNode>();
    x_var->type = "int";
    x_var->name = "x";
    x_var->value = std::make_unique<LiteralNode>("int", "5");
    main_body->statements.push_back(std::move(x_var));
    
    // int y = calculate(x, 10);
    auto y_var = std::make_unique<VariableNode>();
    y_var->type = "int";
    y_var->name = "y";
    auto calculate_call = std::make_unique<CallNode>();
    calculate_call->name = "calculate";
    calculate_call->args.push_back(std::make_unique<NameNode>("x"));
    calculate_call->args.push_back(std::make_unique<LiteralNode>("int", "10"));
    y_var->value = std::move(calculate_call);
    main_body->statements.push_back(std::move(y_var));
    
    // return y;
    auto main_return = std::make_unique<ReturnNode>();
    main_return->value = std::make_unique<NameNode>("y");
    main_body->statements.push_back(std::move(main_return));
    
    main_func.body = std::move(main_body);
    
    // =============================================
    // TEST CASE 2: Error - Undeclared Variable
    // =============================================
    std::cout << "\n❌ TEST 2: Undeclared Variable (Should FAIL)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code: int result = unknown_var * 2;  // 'unknown_var' not declared!
    std::cout << "✗ Adding: int result = unknown_var * 2;  // ERROR: unknown_var not declared" << std::endl;
    program.globals.emplace_back("int", "result");
    auto error_expr = std::make_unique<BinaryOpNode>("*");
    error_expr->left = std::make_unique<NameNode>("unknown_var");  // This will cause error
    error_expr->right = std::make_unique<LiteralNode>("int", "2");
    program.globals.back().value = std::move(error_expr);
    
    // =============================================
    // TEST CASE 3: Error - Undefined Function
    // =============================================
    std::cout << "\n❌ TEST 3: Undefined Function (Should FAIL)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code: int value = unknown_func();  // 'unknown_func' not defined!
    std::cout << "✗ Adding: int value = unknown_func();  // ERROR: unknown_func not defined" << std::endl;
    program.globals.emplace_back("int", "value");
    auto error_call = std::make_unique<CallNode>();
    error_call->name = "unknown_func";  // This will cause error
    program.globals.back().value = std::move(error_call);
    
    // =============================================
    // TEST CASE 4: Error - Variable Redefinition
    // =============================================
    std::cout << "\n❌ TEST 4: Variable Redefinition (Should FAIL)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Add a function with duplicate variable
    std::cout << "✗ Adding function with: int x = 5; int x = 10;  // ERROR: x redefined" << std::endl;
    program.functions.emplace_back("void", "test_redefinition");
    FunctionNode& redef_func = program.functions.back();
    
    auto redef_body = std::make_unique<BlockNode>();
    
    // int x = 5;
    auto x1 = std::make_unique<VariableNode>();
    x1->type = "int";
    x1->name = "x";
    x1->value = std::make_unique<LiteralNode>("int", "5");
    redef_body->statements.push_back(std::move(x1));
    
    // int x = 10;  // ERROR: x already declared in same scope!
    auto x2 = std::make_unique<VariableNode>();
    x2->type = "int";
    x2->name = "x";  // This will cause error
    x2->value = std::make_unique<LiteralNode>("int", "10");
    redef_body->statements.push_back(std::move(x2));
    
    redef_func.body = std::move(redef_body);
    
    // =============================================
    // TEST CASE 5: Error - Function Redefinition
    // =============================================
    std::cout << "\n❌ TEST 5: Function Redefinition (Should FAIL)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code: void calculate() {}  // ERROR: calculate already defined!
    std::cout << "✗ Adding: void calculate() {}  // ERROR: calculate redefined" << std::endl;
    program.functions.emplace_back("void", "calculate");  // This will cause error
    
    // =============================================
    // TEST CASE 6: Control Structures (Valid)
    // =============================================
    std::cout << "\n📝 TEST 6: Control Structures (Should PASS)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code with if, while, for
    std::cout << "✓ Adding function with if, while, for loops" << std::endl;
    program.functions.emplace_back("void", "control_test");
    FunctionNode& control_func = program.functions.back();
    
    auto control_body = std::make_unique<BlockNode>();
    
    // if (MAX_SIZE > 0) { int temp = MAX_SIZE; }
    auto if_stmt = std::make_unique<IfNode>();
    auto if_condition = std::make_unique<BinaryOpNode>(">");
    if_condition->left = std::make_unique<NameNode>("MAX_SIZE");
    if_condition->right = std::make_unique<LiteralNode>("int", "0");
    if_stmt->condition = std::move(if_condition);
    
    auto then_block = std::make_unique<BlockNode>();
    auto temp_var = std::make_unique<VariableNode>();
    temp_var->type = "int";
    temp_var->name = "temp";
    temp_var->value = std::make_unique<NameNode>("MAX_SIZE");
    then_block->statements.push_back(std::move(temp_var));
    if_stmt->then_branch = std::move(then_block);
    
    control_body->statements.push_back(std::move(if_stmt));
    
    // while (true) { break; } - simplified
    auto while_loop = std::make_unique<WhileNode>();
    while_loop->condition = std::make_unique<LiteralNode>("bool", "true");
    while_loop->body = std::make_unique<BlockNode>();  // Empty body
    control_body->statements.push_back(std::move(while_loop));
    
    // for (int i = 0; i < 10; i++) { }
    auto for_loop = std::make_unique<ForNode>();
    auto for_init = std::make_unique<VariableNode>();
    for_init->type = "int";
    for_init->name = "i";
    for_init->value = std::make_unique<LiteralNode>("int", "0");
    for_loop->initializer = std::move(for_init);
    
    auto for_cond = std::make_unique<BinaryOpNode>("<");
    for_cond->left = std::make_unique<NameNode>("i");
    for_cond->right = std::make_unique<LiteralNode>("int", "10");
    for_loop->condition = std::move(for_cond);
    
    auto for_inc = std::make_unique<AssignmentNode>("i");
    auto inc_val = std::make_unique<BinaryOpNode>("+");
    inc_val->left = std::make_unique<NameNode>("i");
    inc_val->right = std::make_unique<LiteralNode>("int", "1");
    for_inc->value = std::move(inc_val);
    for_loop->increment = std::move(for_inc);
    
    for_loop->body = std::make_unique<BlockNode>();  // Empty body
    control_body->statements.push_back(std::move(for_loop));
    
    control_func.body = std::move(control_body);
    
    // =============================================
    // TEST CASE 7: Assignment Statement
    // =============================================
    std::cout << "\n📝 TEST 7: Assignment Statement (Should PASS)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code: x = y + 5; (in a function)
    std::cout << "✓ Adding assignment: x = y + 5;" << std::endl;
    program.functions.emplace_back("void", "assignment_test");
    FunctionNode& assign_func = program.functions.back();
    
    auto assign_body = std::make_unique<BlockNode>();
    
    // First declare y
    auto y_decl = std::make_unique<VariableNode>();
    y_decl->type = "int";
    y_decl->name = "y";
    y_decl->value = std::make_unique<LiteralNode>("int", "10");
    assign_body->statements.push_back(std::move(y_decl));
    
    // Then declare x
    auto x_decl = std::make_unique<VariableNode>();
    x_decl->type = "int";
    x_decl->name = "x";
    x_decl->value = std::make_unique<LiteralNode>("int", "0");
    assign_body->statements.push_back(std::move(x_decl));
    
    // Assignment: x = y + 5;
    auto assignment = std::make_unique<AssignmentNode>("x");
    auto assign_expr = std::make_unique<BinaryOpNode>("+");
    assign_expr->left = std::make_unique<NameNode>("y");
    assign_expr->right = std::make_unique<LiteralNode>("int", "5");
    assignment->value = std::move(assign_expr);
    assign_body->statements.push_back(std::move(assignment));
    
    assign_func.body = std::move(assign_body);
    
    // =============================================
    // TEST CASE 8: Shadowing (Valid)
    // =============================================
    std::cout << "\n📝 TEST 8: Variable Shadowing (Should PASS)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Code with shadowing
    std::cout << "✓ Adding shadowing: outer x and inner x (different scopes)" << std::endl;
    program.functions.emplace_back("void", "shadow_test");
    FunctionNode& shadow_func = program.functions.back();
    
    auto shadow_body = std::make_unique<BlockNode>();
    
    // Outer x
    auto outer_x = std::make_unique<VariableNode>();
    outer_x->type = "int";
    outer_x->name = "x";
    outer_x->value = std::make_unique<LiteralNode>("int", "1");
    shadow_body->statements.push_back(std::move(outer_x));
    
    // Inner block with shadowing x
    auto inner_block = std::make_unique<BlockNode>();
    auto inner_x = std::make_unique<VariableNode>();
    inner_x->type = "int";
    inner_x->name = "x";  // This shadows outer x - allowed!
    inner_x->value = std::make_unique<LiteralNode>("int", "2");
    inner_block->statements.push_back(std::move(inner_x));
    shadow_body->statements.push_back(std::move(inner_block));
    
    shadow_func.body = std::move(shadow_body);
    
    std::cout << "\n=== RUNNING SCOPE ANALYSIS ===" << std::endl;
    std::cout << "Testing all cases: valid code, errors, control structures..." << std::endl;
    
    ScopeAnalyzer analyzer;
    bool success = analyzer.check(&program);
    
    std::cout << "\n=== FINAL RESULTS ===" << std::endl;
    std::cout << "Total errors found: " << analyzer.errorCount() << std::endl;
    
    if (success) {
        std::cout << "✅ SUCCESS: No scope errors found!" << std::endl;
    } else {
        std::cout << "❌ FAILED: Scope analysis detected errors (as expected)" << std::endl;
        std::cout << "\nError Summary:" << std::endl;
        std::cout << "---------------" << std::endl;
        
        int error_num = 1;
        for (auto error : analyzer.getErrors()) {
            std::cout << error_num++ << ". ";
            switch(error) {
                case ScopeError::UndeclaredVariable:
                    std::cout << "Undeclared variable used";
                    break;
                case ScopeError::UndefinedFunction:
                    std::cout << "Undefined function called";
                    break;
                case ScopeError::VariableRedefined:
                    std::cout << "Variable redefined in same scope";
                    break;
                case ScopeError::FunctionRedefined:
                    std::cout << "Function redefined";
                    break;
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "\n=== EXPECTED BEHAVIOR ===" << std::endl;
    std::cout << "✓ Valid code should pass scope checking" << std::endl;
    std::cout << "✗ Errors should be caught and reported" << std::endl;
    std::cout << "✓ Control structures should work correctly" << std::endl;
    std::cout << "✓ Variable shadowing should be allowed" << std::endl;
    
    return 0;
}