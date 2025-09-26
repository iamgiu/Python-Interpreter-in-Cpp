/**
 * Guard Headers
 */
#ifndef INTERPRETER_H
#define INTERPRETER_H

/**
 * Include for AST definitions
 * 
 * Include for std::unordered_map used as variable envitoment 
 * 
 * Include std::vector used inside Balue to represent list
 * 
 * Include for std::variant used in Value to store int, bool or list in one 
 * 
 * Include fot std::runtime_error used as base for RuntimeError
 * 
 * Include for std::count and std::endl used in PrintStatement visitor
 */
#include "ast.h"
#include <unordered_map>
#include <vector>
#include <variant>
#include <stdexcept>
#include <iostream>

/**
 * Expetion for runtime errors
 * 
 * Inherits from std::runtime_error and prefixes the message with "Error:"
 */
class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& message) : std::runtime_error("Error: " + message) {}
};

/**
 * Rapresents a value in the Interpreter (interger, boolean or list)
 */
class Value {
public:
    enum Type { INTEGER, BOOLEAN, LIST, UNDEFINED };
    
    Type type;
    std::variant<int, bool, std::vector<Value>> data;
    
    Value() : type(UNDEFINED) {}
    Value(int i) : type(INTEGER), data(i) {}
    Value(bool b) : type(BOOLEAN), data(b) {}
    Value(const std::vector<Value>& l) : type(LIST), data(l) {}

    int getInt() const {
        if (type != INTEGER) throw RuntimeError("Expected integer value");
        return std::get<int>(data);
    }
    
    bool getBool() const {
        if (type != BOOLEAN) throw RuntimeError("Expected boolean value");
        return std::get<bool>(data);
    }
    
    std::vector<Value>& getList() {
        if (type != LIST) throw RuntimeError("Expected list value");
        return std::get<std::vector<Value>>(data);
    }
    
    const std::vector<Value>& getList() const {
        if (type != LIST) throw RuntimeError("Expected list value");
        return std::get<std::vector<Value>>(data);
    }
    
    std::string toString() const {
        switch (type) {
            case INTEGER: return std::to_string(getInt());
            case BOOLEAN: return getBool() ? "True" : "False";
            case LIST: {
                const auto& list = getList();
                std::string result = "[";
                for (size_t i = 0; i < list.size(); i++) {
                    if (i > 0) result += ", ";
                    result += list[i].toString();
                }
                result += "]";
                return result;
            }
            case UNDEFINED: return "undefined";
        }
        return "unknown";
    }
};

/**
 * Exceptions used to implement break/continue control flow
 */
class BreakException : public std::exception {};
class ContinueException : public std::exception {};

/**
 * Interpreter class
 * 
 * Implements ASTVisitor to execute the program represented by the AST
 * Keep a variable enviroment and current value being evaluated
 * 
 * Private:
 * Symbol table for variables
 * Current value being computed
 * Flag to indicate if we are inside a loop
 * 
 * Public:
 * Exeutes the entire program
 * Visitor implementations for expressions
 * Visitor impelemntations for statements
 * 
 * Private:
 * Consider an expression and returns its value
 * Executes a single statement
 * Helper functions for unary and binary operations
 */
class Interpreter : public ASTVisitor {
private:
    std::unordered_map<std::string, Value> variables;
    
    Value currentValue;

    bool inLoop;
    
public:
    Interpreter();

    void execute(Program& program);

    void visit(NumberLiteral& node) override;
    void visit(BooleanLiteral& node) override;
    void visit(Identifier& node) override;
    void visit(ListAccess& node) override;
    void visit(UnaryOperation& node) override;
    void visit(BinaryOperation& node) override;
    
    void visit(Assignment& node) override;
    void visit(ListAssignment& node) override;
    void visit(ListCreation& node) override;
    void visit(ListAppend& node) override;
    void visit(PrintStatement& node) override;
    void visit(BreakStatement& node) override;
    void visit(ContinueStatement& node) override;
    void visit(IfStatement& node) override;
    void visit(WhileStatement& node) override;
    void visit(Block& node) override;
    void visit(Program& node) override;
    
private:
    Value evaluateExpression(Expression& expr);

    void executeStatement(Statement& stmt);

    Value performBinaryOperation(const Value& left, BinaryOperation::Operator op, const Value& right);
    Value performUnaryOperation(UnaryOperation::Operator op, const Value& operand);
};

#endif // INTERPRETER_H