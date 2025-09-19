#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>
#include <vector>
#include <variant>
#include <stdexcept>
#include <iostream>

// Eccezione per errori di runtime
class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& message) : std::runtime_error("Error: " + message) {}
};

// Rappresenta un valore nel nostro interprete
class Value {
public:
    enum Type { INTEGER, BOOLEAN, LIST, UNDEFINED };
    
    Type type;
    std::variant<int, bool, std::vector<Value>> data;
    
    Value() : type(UNDEFINED) {}
    Value(int i) : type(INTEGER), data(i) {}
    Value(bool b) : type(BOOLEAN), data(b) {}
    Value(const std::vector<Value>& l) : type(LIST), data(l) {}
    
    // Getters con controllo di tipo
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
    
    // Conversione a stringa per il print
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
    
    // Controllo se il valore è "truthy" (per if/while)
    bool isTruthy() const {
        switch (type) {
            case BOOLEAN: return getBool();
            case INTEGER: return getInt() != 0;
            case LIST: return !getList().empty();
            case UNDEFINED: return false;
        }
        return false;
    }
};

// Eccezione speciale per break/continue
class BreakException : public std::exception {};
class ContinueException : public std::exception {};

class Interpreter : public ASTVisitor {
private:
    // Environment per le variabili
    std::unordered_map<std::string, Value> variables;
    
    // Stack per il valore corrente durante la valutazione
    Value currentValue;
    
    // Flag per controllare se siamo in un ciclo
    bool inLoop;
    
public:
    Interpreter();
    
    // Esegue il programma
    void execute(Program& program);
    
    // Implementazioni del visitor pattern
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
    // Valuta un'espressione e restituisce il valore
    Value evaluateExpression(Expression& expr);
    
    // Esegue una statement
    void executeStatement(Statement& stmt);
    
    // Helper per operazioni binarie
    Value performBinaryOperation(const Value& left, BinaryOperation::Operator op, const Value& right);
    Value performUnaryOperation(UnaryOperation::Operator op, const Value& operand);
};

#endif // INTERPRETER_H