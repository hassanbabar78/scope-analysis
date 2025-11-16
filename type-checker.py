from enum import Enum
from typing import List, Dict, Optional

# ============================================================================
# 1. TYPES AND ERRORS
# ============================================================================

class ErrorType(Enum):
    ErroneousVarDecl = 1
    FnCallParamCount = 2
    FnCallParamType = 3
    ErroneousReturnType = 4
    ExpressionTypeMismatch = 5
    ExpectedBooleanExpression = 6
    ErroneousBreak = 7
    NonBooleanCondStmt = 8
    EmptyExpression = 9
    AttemptedBoolOpOnNonBools = 10
    AttemptedBitOpOnNonNumeric = 11
    AttemptedShiftOnNonInt = 12
    AttemptedAddOpOnNonNumeric = 13
    AttemptedExponentiationOfNonNumeric = 14
    ReturnStmtNotFound = 15

class Type(Enum):
    INT = "int"
    FLOAT = "float"
    BOOL = "bool"
    STRING = "string"
    VOID = "void"
    UNKNOWN = "unknown"

# ============================================================================
# 2. SYMBOL TABLE
# ============================================================================

class SymTab:
    def __init__(self):
        self.scopes = [{}]
        self.funcs = {}
        self.loop_depth = 0
   
    def push_scope(self):
        self.scopes.append({})
   
    def pop_scope(self):
        if len(self.scopes) > 1:
            self.scopes.pop()
   
    def add_var(self, name: str, typ: Type):
        self.scopes[-1][name] = typ
   
    def get_var(self, name: str) -> Optional[Type]:
        for scope in reversed(self.scopes):
            if name in scope:
                return scope[name]
        return None
   
    def add_func(self, name: str, ret: Type, params: List[Type]):
        self.funcs[name] = {"ret": ret, "params": params}
   
    def get_func(self, name: str):
        return self.funcs.get(name)

# ============================================================================
# 3. TYPE CHECKER
# ============================================================================

class TypeChecker:
    def __init__(self):
        self.sym = SymTab()
        self.errors = []
        self.current_return_type = None
        self.has_return = False
   
    def error(self, err: ErrorType, line: int, msg: str = ""):
        self.errors.append({
            "type": err,
            "line": line,
            "msg": f"{err.name}: {msg}"
        })
   
    def print_errors(self):
        if not self.errors:
            print("✓ No type errors")
            return
        print(f"Found {len(self.errors)} error(s):")
        for e in self.errors:
            print(f"  Line {e['line']}: {e['msg']}")
   
    # Type helpers
    def is_numeric(self, t: Type) -> bool:
        return t in [Type.INT, Type.FLOAT]
   
    def is_boolean(self, t: Type) -> bool:
        return t == Type.BOOL
   
    def is_integer(self, t: Type) -> bool:
        return t == Type.INT
   
    def str_to_type(self, s: str) -> Type:
        return {
            "int": Type.INT,
            "float": Type.FLOAT,
            "bool": Type.BOOL,
            "string": Type.STRING,
            "void": Type.VOID
        }.get(s, Type.UNKNOWN)
   
    # Expression checking
    def check_expr(self, expr) -> Type:
        if not expr:
            self.error(ErrorType.EmptyExpression, 0, "Empty expression")
            return Type.UNKNOWN
       
        typ = expr.get("type", "")
        line = expr.get("line", 0)
       
        if typ == "literal":
            val = expr.get("value")
            if isinstance(val, bool): return Type.BOOL
            elif isinstance(val, int): return Type.INT
            elif isinstance(val, float): return Type.FLOAT
            elif isinstance(val, str): return Type.STRING
            return Type.UNKNOWN
       
        elif typ == "variable":
            name = expr.get("name")
            t = self.sym.get_var(name)
            if t is None:
                self.error(ErrorType.ExpressionTypeMismatch, line, f"Undefined: {name}")
                return Type.UNKNOWN
            return t
       
        elif typ == "binary_op":
            return self.check_binary_op(expr)
       
        elif typ == "unary_op":
            return self.check_unary_op(expr)
       
        elif typ == "call":
            return self.check_call(expr)
       
        return Type.UNKNOWN
   
    def check_binary_op(self, expr) -> Type:
        op = expr.get("operator")
        left = expr.get("left")
        right = expr.get("right")
        line = expr.get("line", 0)
       
        left_type = self.check_expr(left)
        right_type = self.check_expr(right)
       
        # Arithmetic
        if op in ["+", "-", "*", "/"]:
            if not (self.is_numeric(left_type) and self.is_numeric(right_type)):
                self.error(ErrorType.AttemptedAddOpOnNonNumeric, line, f"'{op}' needs numbers")
                return Type.UNKNOWN
            if left_type == Type.FLOAT or right_type == Type.FLOAT:
                return Type.FLOAT
            return Type.INT
       
        # Comparison
        elif op in ["==", "!=", "<", "<=", ">", ">="]:
            if left_type != right_type:
                self.error(ErrorType.ExpressionTypeMismatch, line, f"Can't compare {left_type.value} and {right_type.value}")
                return Type.UNKNOWN
            return Type.BOOL
       
        # Logical
        elif op in ["&&", "||"]:
            if not (self.is_boolean(left_type) and self.is_boolean(right_type)):
                self.error(ErrorType.AttemptedBoolOpOnNonBools, line, f"'{op}' needs booleans")
                return Type.UNKNOWN
            return Type.BOOL
       
        # Bitwise
        elif op in ["&", "|", "^"]:
            if not (self.is_integer(left_type) and self.is_integer(right_type)):
                self.error(ErrorType.AttemptedBitOpOnNonNumeric, line, f"'{op}' needs integers")
                return Type.UNKNOWN
            return Type.INT
       
        # Shift
        elif op in ["<<", ">>"]:
            if not (self.is_integer(left_type) and self.is_integer(right_type)):
                self.error(ErrorType.AttemptedShiftOnNonInt, line, f"'{op}' needs integers")
                return Type.UNKNOWN
            return Type.INT
       
        # Power
        elif op == "**":
            if not (self.is_numeric(left_type) and self.is_numeric(right_type)):
                self.error(ErrorType.AttemptedExponentiationOfNonNumeric, line, f"'**' needs numbers")
                return Type.UNKNOWN
            if left_type == Type.FLOAT or right_type == Type.FLOAT:
                return Type.FLOAT
            return Type.INT
       
        return Type.UNKNOWN
   
    def check_unary_op(self, expr) -> Type:
        op = expr.get("operator")
        operand = expr.get("operand")
        line = expr.get("line", 0)
       
        op_type = self.check_expr(operand)
       
        if op == "-":
            if not self.is_numeric(op_type):
                self.error(ErrorType.AttemptedAddOpOnNonNumeric, line, f"'-' needs number")
                return Type.UNKNOWN
            return op_type
       
        elif op == "!":
            if not self.is_boolean(op_type):
                self.error(ErrorType.AttemptedBoolOpOnNonBools, line, f"'!' needs boolean")
                return Type.UNKNOWN
            return Type.BOOL
       
        elif op == "~":
            if not self.is_integer(op_type):
                self.error(ErrorType.AttemptedBitOpOnNonNumeric, line, f"'~' needs integer")
                return Type.UNKNOWN
            return Type.INT
       
        return Type.UNKNOWN
   
    def check_call(self, expr) -> Type:
        name = expr.get("name")
        args = expr.get("arguments", [])
        line = expr.get("line", 0)
       
        func = self.sym.get_func(name)
        if not func:
            self.error(ErrorType.ExpressionTypeMismatch, line, f"Unknown function: {name}")
            return Type.UNKNOWN
       
        if len(args) != len(func["params"]):
            self.error(ErrorType.FnCallParamCount, line, f"{name} needs {len(func['params'])} args, got {len(args)}")
            return func["ret"]
       
        for i, (arg, expected) in enumerate(zip(args, func["params"])):
            arg_type = self.check_expr(arg)
            if arg_type != expected:
                self.error(ErrorType.FnCallParamType, line, f"Arg {i+1} of {name}: expected {expected.value}, got {arg_type.value}")
       
        return func["ret"]
   
    # Statement checking
    def check_stmt(self, stmt):
        typ = stmt.get("type", "")
       
        if typ == "variable_declaration":
            self.check_var_decl(stmt)
        elif typ == "assignment":
            self.check_assignment(stmt)
        elif typ == "if_statement":
            self.check_if(stmt)
        elif typ == "while_statement":
            self.check_while(stmt)
        elif typ == "for_statement":
            self.check_for(stmt)
        elif typ == "return_statement":
            self.check_return(stmt)
        elif typ == "break_statement":
            self.check_break(stmt)
        elif typ == "block":
            self.check_block(stmt)
        elif typ == "function_declaration":
            self.check_function(stmt)
   
    def check_var_decl(self, stmt):
        name = stmt.get("name")
        decl_type = self.str_to_type(stmt.get("data_type", ""))
        value = stmt.get("value")
        line = stmt.get("line", 0)
       
        self.sym.add_var(name, decl_type)
       
        if value:
            val_type = self.check_expr(value)
            if val_type != decl_type:
                self.error(ErrorType.ErroneousVarDecl, line, f"{name}: declared {decl_type.value}, got {val_type.value}")
   
    def check_assignment(self, stmt):
        name = stmt.get("name")
        value = stmt.get("value")
        line = stmt.get("line", 0)
       
        var_type = self.sym.get_var(name)
        if var_type is None:
            self.error(ErrorType.ExpressionTypeMismatch, line, f"Undefined: {name}")
            return
       
        val_type = self.check_expr(value)
        if var_type != val_type:
            self.error(ErrorType.ExpressionTypeMismatch, line, f"{name}: expected {var_type.value}, got {val_type.value}")
   
    def check_if(self, stmt):
        condition = stmt.get("condition")
        line = stmt.get("line", 0)
       
        cond_type = self.check_expr(condition)
        if not self.is_boolean(cond_type):
            self.error(ErrorType.NonBooleanCondStmt, line, f"If needs bool, got {cond_type.value}")
       
        self.sym.push_scope()
        self.check_stmt(stmt.get("then_block"))
        self.sym.pop_scope()
       
        else_block = stmt.get("else_block")
        if else_block:
            self.sym.push_scope()
            self.check_stmt(else_block)
            self.sym.pop_scope()
   
    def check_while(self, stmt):
        condition = stmt.get("condition")
        line = stmt.get("line", 0)
       
        cond_type = self.check_expr(condition)
        if not self.is_boolean(cond_type):
            self.error(ErrorType.NonBooleanCondStmt, line, f"While needs bool, got {cond_type.value}")
       
        self.sym.push_scope()
        self.sym.loop_depth += 1
        self.check_stmt(stmt.get("body"))
        self.sym.loop_depth -= 1
        self.sym.pop_scope()
   
    def check_for(self, stmt):
        init = stmt.get("init")
        condition = stmt.get("condition")
        update = stmt.get("update")
        line = stmt.get("line", 0)
       
        self.sym.push_scope()
        self.sym.loop_depth += 1
       
        if init:
            self.check_stmt(init)
       
        if condition:
            cond_type = self.check_expr(condition)
            if not self.is_boolean(cond_type):
                self.error(ErrorType.NonBooleanCondStmt, line, f"For needs bool, got {cond_type.value}")
       
        if update:
            self.check_stmt(update)
       
        self.check_stmt(stmt.get("body"))
       
        self.sym.loop_depth -= 1
        self.sym.pop_scope()
   
    def check_return(self, stmt):
        value = stmt.get("value")
        line = stmt.get("line", 0)
       
        self.has_return = True
       
        if self.current_return_type == Type.VOID:
            if value:
                self.error(ErrorType.ErroneousReturnType, line, f"Void function can't return value")
            return
       
        if value is None:
            self.error(ErrorType.ErroneousReturnType, line, f"Missing return value")
            return
       
        ret_type = self.check_expr(value)
        if ret_type != self.current_return_type:
            self.error(ErrorType.ErroneousReturnType, line, f"Expected {self.current_return_type.value}, got {ret_type.value}")
   
    def check_break(self, stmt):
        line = stmt.get("line", 0)
       
        if self.sym.loop_depth == 0:
            self.error(ErrorType.ErroneousBreak, line, "Break outside loop")
   
    def check_block(self, stmt):
        statements = stmt.get("statements", [])
       
        self.sym.push_scope()
       
        for s in statements:
            self.check_stmt(s)
       
        self.sym.pop_scope()
   
    def check_function(self, stmt):
        name = stmt.get("name")
        ret_type = self.str_to_type(stmt.get("return_type", ""))
        params = stmt.get("parameters", [])
        body = stmt.get("body")
        line = stmt.get("line", 0)
       
        param_types = []
        for p in params:
            param_types.append(self.str_to_type(p.get("data_type", "")))
       
        self.sym.add_func(name, ret_type, param_types)
       
        old_return = self.current_return_type
        old_has_return = self.has_return
        self.current_return_type = ret_type
        self.has_return = False
       
        self.sym.push_scope()
       
        for p in params:
            self.sym.add_var(p.get("name"), self.str_to_type(p.get("data_type", "")))
       
        self.check_stmt(body)
       
        if ret_type != Type.VOID and not self.has_return:
            self.error(ErrorType.ReturnStmtNotFound, line, f"Function {name} needs return")
       
        self.sym.pop_scope()
        self.current_return_type = old_return
        self.has_return = old_has_return
   
    # Main entry
    def check_program(self, program):
        for stmt in program.get("statements", []):
            self.check_stmt(stmt)
       
        for func in program.get("functions", []):
            self.check_stmt(func)

# ============================================================================
# 4. TEST
# ============================================================================

def create_test():
    """Test program with type errors"""
    return {
        "functions": [
            {
                "type": "function_declaration",
                "name": "main",
                "return_type": "int",
                "parameters": [],
                "body": {
                    "type": "block",
                    "statements": [
                        {
                            "type": "variable_declaration",
                            "name": "x",
                            "data_type": "int",
                            "value": {"type": "literal", "value": 5},
                            "line": 2
                        },
                        {
                            "type": "variable_declaration",
                            "name": "y",
                            "data_type": "bool",
                            "value": {"type": "literal", "value": "hello"},
                            "line": 3
                        },
                        {
                            "type": "assignment",
                            "name": "x",
                            "value": {"type": "literal", "value": "text"},
                            "line": 4
                        },
                        {
                            "type": "if_statement",
                            "condition": {
                                "type": "binary_op",
                                "operator": "+",
                                "left": {"type": "variable", "name": "x"},
                                "right": {"type": "literal", "value": 1}
                            },
                            "then_block": {"type": "block", "statements": []},
                            "line": 5
                        },
                        {
                            "type": "break_statement",
                            "line": 6
                        },
                        {
                            "type": "return_statement",
                            "value": {"type": "literal", "value": "done"},
                            "line": 7
                        }
                    ]
                },
                "line": 1
            }
        ]
    }

def main():
    print("=" * 50)
    print("TYPE CHECKER TEST")
    print("=" * 50)
   
    program = create_test()
    checker = TypeChecker()
    checker.check_program(program)
    checker.print_errors()
   
    print("=" * 50)
    print(f"Total errors: {len(checker.errors)}")

if __name__ == "__main__":
    main()