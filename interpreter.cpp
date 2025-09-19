#include "interpreter.h"

Interpreter::Interpreter() : inLoop(false) {}

void Interpreter::execute(Program& program) {
    try {
        program.accept(*this);
    } catch (const RuntimeError& e) {
        throw e; // Rilancia per gestione in main
    } catch (const BreakException&) {
        throw RuntimeError("'break' outside loop");
    } catch (const ContinueException&) {
        throw RuntimeError("'continue' outside loop");
    }
}

Value Interpreter::evaluateExpression(Expression& expr) {
    expr.accept(*this);
    return currentValue;
}

void Interpreter::executeStatement(Statement& stmt) {
    stmt.accept(*this);
}

// ========== ESPRESSIONI ==========

void Interpreter::visit(NumberLiteral& node) {
    currentValue = Value(node.value);
}

void Interpreter::visit(BooleanLiteral& node) {
    currentValue = Value(node.value);
}

void Interpreter::visit(Identifier& node) {
    auto it = variables.find(node.name);
    if (it == variables.end()) {
        throw RuntimeError("Undefined variable '" + node.name + "'");
    }
    
    // Controllo di sicurezza aggiuntivo
    if (it->second.type == Value::UNDEFINED) {
        throw RuntimeError("Variable '" + node.name + "' is undefined");
    }
    
    currentValue = it->second;
}

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
    
    // Controllo migliorato degli indici
    if (index < 0) {
        throw RuntimeError("List index cannot be negative");
    }
    
    if (static_cast<size_t>(index) >= list.size()) {
        throw RuntimeError("List index out of range (index: " + 
                         std::to_string(index) + ", size: " + 
                         std::to_string(list.size()) + ")");
    }
    
    currentValue = list[index];
}

void Interpreter::visit(UnaryOperation& node) {
    Value operand = evaluateExpression(*node.operand);
    currentValue = performUnaryOperation(node.op, operand);
}

void Interpreter::visit(BinaryOperation& node) {
    // Gestione short-circuit per operatori logici
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
    
    // Per tutti gli altri operatori, valuta entrambi gli operandi
    Value left = evaluateExpression(*node.left);
    Value right = evaluateExpression(*node.right);
    currentValue = performBinaryOperation(left, node.op, right);
}

// ========== ISTRUZIONI ==========

void Interpreter::visit(Assignment& node) {
    Value value = evaluateExpression(*node.value);
    variables[node.variableName] = value;
}

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

void Interpreter::visit(ListCreation& node) {
    variables[node.variableName] = Value(std::vector<Value>());
}

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

void Interpreter::visit(PrintStatement& node) {
    Value value = evaluateExpression(*node.expression);
    std::cout << value.toString() << std::endl;
}

void Interpreter::visit(BreakStatement& node) {
    if (!inLoop) {
        throw RuntimeError("'break' outside loop");
    }
    throw BreakException();
}

void Interpreter::visit(ContinueStatement& node) {
    if (!inLoop) {
        throw RuntimeError("'continue' outside loop");
    }
    throw ContinueException();
}

void Interpreter::visit(IfStatement& node) {
    Value condition = evaluateExpression(*node.condition);
    
    // Usa isTruthy() invece di controllare esplicitamente il tipo booleano
    if (condition.isTruthy()) {
        executeStatement(*node.thenBlock);
        return;
    }
    
    // Controlla elif clauses
    for (const auto& elif : node.elifClauses) {
        Value elifCondition = evaluateExpression(*elif.condition);
        if (elifCondition.isTruthy()) {
            executeStatement(*elif.body);
            return;
        }
    }
    
    // Esegui else block se presente
    if (node.elseBlock) {
        executeStatement(*node.elseBlock);
    }
}

void Interpreter::visit(WhileStatement& node) {
    bool wasInLoop = inLoop;
    inLoop = true;
    
    try {
        while (true) {
            Value condition = evaluateExpression(*node.condition);
            
            // Usa isTruthy() invece del controllo di tipo
            if (!condition.isTruthy()) {
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

void Interpreter::visit(Block& node) {
    for (auto& stmt : node.statements) {
        executeStatement(*stmt);
    }
}

void Interpreter::visit(Program& node) {
    for (auto& stmt : node.statements) {
        executeStatement(*stmt);
    }
}

// ========== HELPER METHODS ==========

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

Value Interpreter::performBinaryOperation(const Value& left, BinaryOperation::Operator op, const Value& right) {
    switch (op) {
        // Operatori aritmetici - richiedono interi
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
            
        // Operatori relazionali - richiedono interi
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
            
        // Operatori di uguaglianza - richiedono stesso tipo
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
            
        // Operatori logici - gestiti già in visit(BinaryOperation)
        case BinaryOperation::Operator::AND:
        case BinaryOperation::Operator::OR:
            throw RuntimeError("Logical operators should be handled in visit(BinaryOperation)");
    }
    throw RuntimeError("Unknown binary operator");
}