/**
 * Guard Headers
 */
#ifndef PARSER_H
#define PARSER_H

/**
 * Include std::vector used inside Balue to represent list
 * 
 * Memory: for smart pointers (unique_ptr)
 * 
 * Include fot std::runtime_error used as base for RuntimeError
 * 
 * Include for AST definitions
 */
#include <vector>
#include <memory>
#include <stdexcept>
#include "lexer.h"
#include "ast.h"

/**
 * Exception class for parsing errors
 */
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error("Error: " + message) {}
};

/**
 * Implements a parser following the BNF grammar specification
 */
class Parser {
private:
    std::vector<Token> tokens;
    size_t currentPos;

    const Token& currentToken();
    const Token& peekToken(int offset = 1);
    void advance();
    bool isAtEnd();
    bool match(TokenType type);
    bool check(TokenType type);
    Token consume(TokenType type, const std::string& message);
    
public:
    Parser(const std::vector<Token>& tokenStream);

    std::unique_ptr<Program> parseProgram();
    
private:
    std::unique_ptr<Program> parseProgram_();

    void parseStmts(std::vector<std::unique_ptr<Statement>>& statements);

    std::unique_ptr<Statement> parseStmt();

    std::unique_ptr<Statement> parseSimpleStmt();
    std::unique_ptr<Statement> parseAssignment();
    std::unique_ptr<Statement> parseListCreation();
    std::unique_ptr<Statement> parseListAppend();
    std::unique_ptr<Statement> parsePrintStatement();
    std::unique_ptr<Statement> parseBreakStatement();
    std::unique_ptr<Statement> parseContinueStatement();

    std::unique_ptr<Statement> parseCompoundStmt();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();

    std::unique_ptr<Block> parseBlock();

    std::unique_ptr<Expression> parseExpr();

    std::unique_ptr<Expression> parseJoin();

    std::unique_ptr<Expression> parseEquality();

    std::unique_ptr<Expression> parseRel();

    std::unique_ptr<Expression> parseNumExpr();

    std::unique_ptr<Expression> parseTerm();

    std::unique_ptr<Expression> parseUnary();

    std::unique_ptr<Expression> parseFactor();

    std::unique_ptr<Expression> parseLoc();
};

#endif // PARSER_H