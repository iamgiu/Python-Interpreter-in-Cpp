#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include <stdexcept>
#include "lexer.h"
#include "ast.h"

// Eccezione per errori di parsing
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error("Error: " + message) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t currentPos;
    
    // Utility methods per navigare i token
    const Token& currentToken();
    const Token& peekToken(int offset = 1);
    void advance();
    bool isAtEnd();
    bool match(TokenType type);
    bool check(TokenType type);
    Token consume(TokenType type, const std::string& message);
    
public:
    Parser(const std::vector<Token>& tokenStream);
    
    // Funzione principale di parsing
    std::unique_ptr<Program> parseProgram();
    
private:
    // Metodi per parsare le varie regole della grammatica
    // Seguono la struttura BNF del documento
    
    // <program> → <stmts> endmarker
    std::unique_ptr<Program> parseProgram_();
    
    // <stmts> → <stmt> <stmts> | ε
    void parseStmts(std::vector<std::unique_ptr<Statement>>& statements);
    
    // <stmt> → <compound_stmt> | <simple_stmt>
    std::unique_ptr<Statement> parseStmt();
    
    // Simple statements
    std::unique_ptr<Statement> parseSimpleStmt();
    std::unique_ptr<Statement> parseAssignment();
    std::unique_ptr<Statement> parseListCreation();
    std::unique_ptr<Statement> parseListAppend();
    std::unique_ptr<Statement> parsePrintStatement();
    std::unique_ptr<Statement> parseBreakStatement();
    std::unique_ptr<Statement> parseContinueStatement();
    
    // Compound statements
    std::unique_ptr<Statement> parseCompoundStmt();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    
    // <block> → newline indent <stmts> dedent
    std::unique_ptr<Block> parseBlock();
    
    // Expression parsing con precedenza
    // <expr> → <expr> or <join> | <join>
    std::unique_ptr<Expression> parseExpr();
    
    // <join> → <join> and <equality> | <equality>
    std::unique_ptr<Expression> parseJoin();
    
    // <equality> → <equality> == <rel> | <equality> != <rel> | <rel>
    std::unique_ptr<Expression> parseEquality();
    
    // <rel> → <numexpr> < <numexpr> | ... | <numexpr>
    std::unique_ptr<Expression> parseRel();
    
    // <numexpr> → <numexpr> + <term> | <numexpr> - <term> | <term>
    std::unique_ptr<Expression> parseNumExpr();
    
    // <term> → <term> * <unary> | <term> // <unary> | <unary>
    std::unique_ptr<Expression> parseTerm();
    
    // <unary> → not <unary> | - <unary> | <factor>
    std::unique_ptr<Expression> parseUnary();
    
    // <factor> → ( <expr> ) | <loc> | num | True | False
    std::unique_ptr<Expression> parseFactor();
    
    // <loc> → id | id [ <expr> ]
    std::unique_ptr<Expression> parseLoc();
};

#endif // PARSER_H