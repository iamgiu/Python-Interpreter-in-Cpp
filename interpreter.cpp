/**
 * Implementation of the Interpreter class
 */
#include "interpreter.h"

/**
 * Initializes inLoop flag to false
 */
Interpreter::Interpreter() : inLoop(false) {}

/**
 * Esecute the root program node
 * 
 * Try to use accept to traverse AST in case of errors it reports them
 */
void Interpreter::execute(Program& program) {
    try {
        program.accept(*this);
    } catch (const RuntimeError& e) {
        throw e;
    } catch (const BreakException&) {
        throw RuntimeError("'break' outside loop");
    } catch (const ContinueException&) {
        throw RuntimeError("'continue' outside loop");
    }
}

/**
 * Evaluate an Expression node and return its resulting Value
 */
Value Interpreter::evaluateExpression(Expression& expr) {
    expr.accept(*this);
    return currentValue;
}

/**
 * Execute a statement node
 */
void Interpreter::executeStatement(Statement& stmt) {
    stmt.accept(*this);
}

// ========== EXPRESSIONS ==========

/**
 * Visist NumberLiteral: store its interger value in currentValue
 */
void Interpreter::visit(NumberLiteral& node) {
    currentValue = Value(node.value);
}

/**
 * Visist BooleranLiteral: store its boolean value in currentValue
 */
void Interpreter::visit(BooleanLiteral& node) {
    currentValue = Value(node.value);
}

/**
 * Visist Identifier: look up variable in symbol table and store its value
 */
void Interpreter::visit(Identifier& node) {
    auto it = variables.find(node.name);
    if (it == variables.end()) {
        throw RuntimeError("Undefined variable '" + node.name + "'");
    }
    
    if (it->second.type == Value::UNDEFINED) {
        throw RuntimeError("Variable '" + node.name + "' is undefined");
    }
    
    currentValue = it->second;
}

/**
 * Visit ListAccess: evalute index, check bounds and store element value
 */
void Interpreter::visit(ListAccess& node) {
    auto it = variables.find(node.listName);
    if (it == variables.end()) {
        throw RuntimeError("Undefined variable '" + node.listName + "'");
    }
    
    if (it->second.type != Value::LIST) {
        throw RuntimeError("Variable '" + node.listName + "' is not a list");
    }
    
    Value indexValue = evaluateExpression(*node.index);
    if (indexValue.type != Value::INTEGER) {
        throw RuntimeError("List index must be an integer");
    }
    
    int index = indexValue.getInt();
    const auto& list = it->second.getList();
    
    if (index < 0) {
        throw RuntimeError("List index cannot be negative");
    }
    
    if (static_cast<size_t>(index) >= list.size()) {
        throw RuntimeError("List index out of range");
    }
    
    currentValue = list[index];
}

/**
 * Visit UnaryOperation: evalute operand and perform operation
 */
void Interpreter::visit(UnaryOperation& node) {
    Value operand = evaluateExpression(*node.operand);
    currentValue = performUnaryOperation(node.op, operand);
}

/**
 * Visit VinaryOperation: evalute operands and perform operation
 * 
 * I implement short-circuit evaluation for AND/OR separately from the rest of the binary operations because:
 * - THe specification explicitly requires this semantics 
 * - I avoid unnecessary evaluation of the second operand
 * - I maintain consistency with stanrdard Python
 */
void Interpreter::visit(BinaryOperation& node) {
    if (node.op == BinaryOperation::Operator::AND) {
        Value left = evaluateExpression(*node.left);
        if (left.type != Value::BOOLEAN) {
            throw RuntimeError("Logical AND requires boolean operands");
        }
        if (!left.getBool()) {
            currentValue = Value(false);
            return;
        }
        Value right = evaluateExpression(*node.right);
        if (right.type != Value::BOOLEAN) {
            throw RuntimeError("Logical AND requires boolean operands");
        }
        currentValue = Value(right.getBool());
        return;
    }
    
    if (node.op == BinaryOperation::Operator::OR) {
        Value left = evaluateExpression(*node.left);
        if (left.type != Value::BOOLEAN) {
            throw RuntimeError("Logical OR requires boolean operands");
        }
        if (left.getBool()) {
            currentValue = Value(true);
            return;
        }
        Value right = evaluateExpression(*node.right);
        if (right.type != Value::BOOLEAN) {
            throw RuntimeError("Logical OR requires boolean operands");
        }
        currentValue = Value(right.getBool());
        return;
    }

    Value left = evaluateExpression(*node.left);
    Value right = evaluateExpression(*node.right);
    currentValue = performBinaryOperation(left, node.op, right);
}

// ========== INSTRUCTIONS ==========

/**
 *  Visit Assigment: evalute value and assing to variable
 */
void Interpreter::visit(Assignment& node) {
    Value value = evaluateExpression(*node.value);
    variables[node.variableName] = value;
}

/**
 * Visit ListAssigment: set element ad index to evaluted value
 */
void Interpreter::visit(ListAssignment& node) {
    auto it = variables.find(node.listName);
    if (it == variables.end()) {
        throw RuntimeError("Undefined variable '" + node.listName + "'");
    }
    
    if (it->second.type != Value::LIST) {
        throw RuntimeError("Variable '" + node.listName + "' is not a list");
    }
    
    Value indexValue = evaluateExpression(*node.index);
    if (indexValue.type != Value::INTEGER) {
        throw RuntimeError("List index must be an integer");
    }
    
    int index = indexValue.getInt();
    auto& list = it->second.getList();
    
    if (index < 0 || index >= static_cast<int>(list.size())) {
        throw RuntimeError("List index out of range");
    }
    
    Value value = evaluateExpression(*node.value);
    list[index] = value;
}

/**
 * Visit ListCreation: create an empty list and assing to variable
 */
void Interpreter::visit(ListCreation& node) {
    variables[node.variableName] = Value(std::vector<Value>());
}

/**
 * Visit ListAppend: evaluate value and append to list 
 */
void Interpreter::visit(ListAppend& node) {
    auto it = variables.find(node.listName);
    if (it == variables.end()) {
        throw RuntimeError("Undefined variable '" + node.listName + "'");
    }
    
    if (it->second.type != Value::LIST) {
        throw RuntimeError("Variable '" + node.listName + "' is not a list");
    }
    
    Value value = evaluateExpression(*node.value);
    it->second.getList().push_back(value);
}

/**
 * Visit PrintStatement: evalute expression and print result
 */
void Interpreter::visit(PrintStatement& node) {
    Value value = evaluateExpression(*node.expression);
    std::cout << value.toString() << std::endl;
}

/**
 * Visit BreakStatement: throw BreakException on exit loop
 */
void Interpreter::visit(BreakStatement& node) {
    if (!inLoop) {
        throw RuntimeError("'break' outside loop");
    }
    throw BreakException();
}

/**
 * Visit ContinueStatement: throw ContinueException to skip to next item
 */
void Interpreter::visit(ContinueStatement& node) {
    if (!inLoop) {
        throw RuntimeError("'continue' outside loop");
    }
    throw ContinueException();
}

/**
 * Visit IfStatement: evalute conditions and execute matching branch
 */
void Interpreter::visit(IfStatement& node) {
    Value condition = evaluateExpression(*node.condition);

    if (condition.type != Value::BOOLEAN) {
        throw RuntimeError("if condition must be boolean");
    }
    
    if (condition.getBool()) {
        executeStatement(*node.thenBlock);
        return;
    }
    
    for (const auto& elif : node.elifClauses) {
        Value elifCondition = evaluateExpression(*elif.condition);
        if (elifCondition.type != Value::BOOLEAN) {
            throw RuntimeError("elif condition must be boolean");
        }
        if (elifCondition.getBool()) {
            executeStatement(*elif.body);
            return;
        }
    }
    
    if (node.elseBlock) {
        executeStatement(*node.elseBlock);
    }
}

/**
 * Visit WhileStatement: repeatedly execute body while condition in true
 */
void Interpreter::visit(WhileStatement& node) {
    bool wasInLoop = inLoop;
    inLoop = true;
    
    try {
        while (true) {
            Value condition = evaluateExpression(*node.condition);
            
            if (condition.type != Value::BOOLEAN) {
                throw RuntimeError("while condition must be boolean");
            }
            
            if (!condition.getBool()) {
                break;
            }
            
            try {
                executeStatement(*node.body);
            } catch (const ContinueException&) {
                continue;
            } catch (const BreakException&) {
                break;
            }
        }
    } catch (...) {
        inLoop = wasInLoop;
        throw;
    }
    
    inLoop = wasInLoop;
}

/**
 * Visit Block: execute all contained statements
 */
void Interpreter::visit(Block& node) {
    for (auto& stmt : node.statements) {
        executeStatement(*stmt);
    }
}

/**
 * Visit Program: execute all top-level statements
 */
void Interpreter::visit(Program& node) {
    for (auto& stmt : node.statements) {
        executeStatement(*stmt);
    }
}

// ========== HELPER METHODS ==========

/**
 * Perform a unary operation (- or not) and return the resulting Value
 */
Value Interpreter::performUnaryOperation(UnaryOperation::Operator op, const Value& operand) {
    switch (op) {
        case UnaryOperation::Operator::MINUS:
            if (operand.type != Value::INTEGER) {
                throw RuntimeError("Unary minus requires integer operand");
            }
            return Value(-operand.getInt());
            
        case UnaryOperation::Operator::NOT:
            if (operand.type != Value::BOOLEAN) {
                throw RuntimeError("Logical not requires boolean operand");
            }
            return Value(!operand.getBool());
    }
    throw RuntimeError("Unknown unary operator");
}

/**
 * Perform a binary operation (+, -, *, /, <, ==, ...) and return the result
 */
Value Interpreter::performBinaryOperation(const Value& left, BinaryOperation::Operator op, const Value& right) {
    switch (op) {
        case BinaryOperation::Operator::ADD:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Addition requires integer operands");
            }
            return Value(left.getInt() + right.getInt());
            
        case BinaryOperation::Operator::SUBTRACT:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Subtraction requires integer operands");
            }
            return Value(left.getInt() - right.getInt());
            
        case BinaryOperation::Operator::MULTIPLY:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Multiplication requires integer operands");
            }
            return Value(left.getInt() * right.getInt());
            
        case BinaryOperation::Operator::DIVIDE:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Division requires integer operands");
            }
            if (right.getInt() == 0) {
                throw RuntimeError("Division by zero");
            }
            return Value(left.getInt() / right.getInt());

        case BinaryOperation::Operator::LESS:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Comparison requires integer operands");
            }
            return Value(left.getInt() < right.getInt());
            
        case BinaryOperation::Operator::LESS_EQUAL:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Comparison requires integer operands");
            }
            return Value(left.getInt() <= right.getInt());
            
        case BinaryOperation::Operator::GREATER:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Comparison requires integer operands");
            }
            return Value(left.getInt() > right.getInt());
            
        case BinaryOperation::Operator::GREATER_EQUAL:
            if (left.type != Value::INTEGER || right.type != Value::INTEGER) {
                throw RuntimeError("Comparison requires integer operands");
            }
            return Value(left.getInt() >= right.getInt());
            
        case BinaryOperation::Operator::EQUAL:
            if (left.type != right.type) {
                throw RuntimeError("Equality comparison requires same types");
            }
            if (left.type == Value::INTEGER) {
                return Value(left.getInt() == right.getInt());
            } else if (left.type == Value::BOOLEAN) {
                return Value(left.getBool() == right.getBool());
            }
            throw RuntimeError("Cannot compare lists");
            
        case BinaryOperation::Operator::NOT_EQUAL:
            if (left.type != right.type) {
                throw RuntimeError("Equality comparison requires same types");
            }
            if (left.type == Value::INTEGER) {
                return Value(left.getInt() != right.getInt());
            } else if (left.type == Value::BOOLEAN) {
                return Value(left.getBool() != right.getBool());
            }
            throw RuntimeError("Cannot compare lists");
            
        case BinaryOperation::Operator::AND:
        case BinaryOperation::Operator::OR:
            throw RuntimeError("Logical operators should be handled in visit(BinaryOperation)");
    }
    throw RuntimeError("Unknown binary operator");
}