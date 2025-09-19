#include "ast.h"

// Espressioni
void NumberLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BooleanLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void Identifier::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ListAccess::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void UnaryOperation::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BinaryOperation::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Istruzioni
void Assignment::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ListAssignment::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ListCreation::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ListAppend::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void PrintStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BreakStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ContinueStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void WhileStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void Block::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

std::string UnaryOperation::toString() const {
    std::string opStr;
    switch (op) {
        case Operator::MINUS: opStr = "-"; break;
        case Operator::NOT: opStr = "not "; break;
    }
    return opStr + operand->toString();
}

std::string BinaryOperation::toString() const {
    std::string opStr;
    switch (op) {
        case Operator::ADD: opStr = " + "; break;
        case Operator::SUBTRACT: opStr = " - "; break;
        case Operator::MULTIPLY: opStr = " * "; break;
        case Operator::DIVIDE: opStr = " // "; break;
        case Operator::LESS: opStr = " < "; break;
        case Operator::LESS_EQUAL: opStr = " <= "; break;
        case Operator::GREATER: opStr = " > "; break;
        case Operator::GREATER_EQUAL: opStr = " >= "; break;
        case Operator::EQUAL: opStr = " == "; break;
        case Operator::NOT_EQUAL: opStr = " != "; break;
        case Operator::AND: opStr = " and "; break;
        case Operator::OR: opStr = " or "; break;
    }
    return "(" + left->toString() + opStr + right->toString() + ")";
}

std::string IfStatement::toString() const {
    std::string result = "if " + condition->toString() + ": ...";
    
    for (const auto& elif : elifClauses) {
        result += " elif " + elif.condition->toString() + ": ...";
    }
    
    if (elseBlock) {
        result += " else: ...";
    }
    
    return result;
}