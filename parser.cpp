/**
 * Implementation of the Parser class
 */
#include "parser.h"

/**
 * Include for std::count, std::cerr used for printing and error messages
 */
#include <iostream>

/**
 * Initalizes the parser with a stram of tokens
 */
Parser::Parser(const std::vector<Token>& tokenStream) : tokens(tokenStream), currentPos(0) {
    if (tokens.empty() || tokens.back().type != TokenType::ENDMARKER) {
        tokens.push_back(Token(TokenType::ENDMARKER, "EOF", 0, 0));
    }
}

/**
 * Returns the current
 * 
 * Perform safety check
 */
const Token& Parser::currentToken() {
    if (currentPos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[currentPos];
}

/**
 * Return the token at a given offset from the current position
 * 
 * Perform safety check
 */
const Token& Parser::peekToken(int offset) {
    size_t pos = currentPos + offset;
    if (pos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos];
}

/**
 * Advances the current position to the next token
 */
void Parser::advance() {
    if (!isAtEnd()) {
        currentPos++;
    }
}

/**
 * Check if the parser has reached the end of the token stream
 */
bool Parser::isAtEnd() {
    return currentPos >= tokens.size() || currentToken().type == TokenType::ENDMARKER;
}

/**
 * Match the current token against a given type
 * 
 * if matche adavance and return true else return false
 */
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

/**
 * Check the type of the current token without adavancing
 */
bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return currentToken().type == type;
}

/**
 * Consume a token of a specific type
 * 
 * if the token matchm it is consumed and returned else throw a ParseError with a message
 */
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        Token token = currentToken();
        advance();
        return token;
    }
    throw ParseError(message);
}

/**
 * Entry point for parsing a program
 */
std::unique_ptr<Program> Parser::parseProgram() {
    return parseProgram_();
}

/**
 * Parse an entire program
 * 
 * Parse a list of statement, skip trailing DEDENT/NEWLINE and ensure the token stream end with ENDMARKER
 */
std::unique_ptr<Program> Parser::parseProgram_() {
    auto program = std::make_unique<Program>();
    
    parseStmts(program->statements);

    while (currentPos < tokens.size() && 
           (tokens[currentPos].type == TokenType::DEDENT || 
            tokens[currentPos].type == TokenType::NEWLINE)) {
        currentPos++;
    }

    if (currentPos >= tokens.size()) {
        throw ParseError("Unexpected end of token stream");
    }
    
    if (tokens[currentPos].type != TokenType::ENDMARKER) {
        throw ParseError("Expected ENDMARKER");
    }
    
    return program;
}

/**
 * Parse zero or more statement into a given vector
 * 
 * Skip empty NEWLINE and stop and ENDMARKER or DEDENT
 */
void Parser::parseStmts(std::vector<std::unique_ptr<Statement>>& statements) {
    while (!isAtEnd() && !check(TokenType::ENDMARKER) && !check(TokenType::DEDENT)) {

        while (check(TokenType::NEWLINE)) {
            advance();
        }
        
        if (isAtEnd() || check(TokenType::ENDMARKER) || check(TokenType::DEDENT)) {
            break;
        }
        
        auto stmt = parseStmt();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
}

/**
 * Parse a single statement 
 * 
 * Distinguishes between compound statements (if, while) and simple statements
 */
std::unique_ptr<Statement> Parser::parseStmt() {
    if (check(TokenType::IF) || check(TokenType::WHILE)) {
        return parseCompoundStmt();
    } else {
        return parseSimpleStmt();
    }
}

/**
 * Parse a single statement
 * 
 * Gandles assignment, list creation, list append, print, brake, continue
 */
std::unique_ptr<Statement> Parser::parseSimpleStmt() {
    if (check(TokenType::BREAK)) {
        return parseBreakStatement();
    } else if (check(TokenType::CONTINUE)) {
        return parseContinueStatement();
    } else if (check(TokenType::PRINT)) {
        return parsePrintStatement();
    } else if (check(TokenType::ID)) {

        if (currentPos + 1 < tokens.size()) {
            TokenType secondToken = peekToken().type;
            
            if (secondToken == TokenType::ASSIGN) {
                if (currentPos + 2 < tokens.size()) {
                    TokenType thirdToken = peekToken(2).type;
                    
                    if (thirdToken == TokenType::LIST) {
                        return parseListCreation();
                    }
                }
                
                return parseAssignment();
                
            } else if (secondToken == TokenType::LBRACKET) {
                return parseAssignment();
            } else if (secondToken == TokenType::DOT) {
                return parseListAppend();
            }
        }
    }
    
    throw ParseError("Unexpected token in simple statement");
}

/**
 * Parse a regular or list assignment statement
 */
std::unique_ptr<Statement> Parser::parseAssignment() {
    if (check(TokenType::ID)) {
        std::string varName = currentToken().value;
        advance();
        
        if (check(TokenType::LBRACKET)) {
            advance();
            auto index = parseExpr();
            consume(TokenType::RBRACKET, "Expected ']'");
            consume(TokenType::ASSIGN, "Expected '='");
            auto value = parseExpr();
            consume(TokenType::NEWLINE, "Expected newline");
            
            return std::make_unique<ListAssignment>(varName, std::move(index), std::move(value));
        } else {
            consume(TokenType::ASSIGN, "Expected '='");
            auto value = parseExpr();
            consume(TokenType::NEWLINE, "Expected newline");
            
            return std::make_unique<Assignment>(varName, std::move(value));
        }
    }
    
    throw ParseError("Expected identifier in assignment");
}

/**
 * Parse list creation
 */
std::unique_ptr<Statement> Parser::parseListCreation() {
    std::string varName = consume(TokenType::ID, "Expected identifier").value;
    consume(TokenType::ASSIGN, "Expected '='");
    consume(TokenType::LIST, "Expected 'list'");
    consume(TokenType::LPAREN, "Expected '('");
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<ListCreation>(varName);
}

/**
 * Parse list append
 */
std::unique_ptr<Statement> Parser::parseListAppend() {
    std::string listName = consume(TokenType::ID, "Expected identifier").value;
    consume(TokenType::DOT, "Expected '.'");
    consume(TokenType::APPEND, "Expected 'append'");
    consume(TokenType::LPAREN, "Expected '('");
    auto value = parseExpr();
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<ListAppend>(listName, std::move(value));
}

/**
 * Parse print statement
 */
std::unique_ptr<Statement> Parser::parsePrintStatement() {
    consume(TokenType::PRINT, "Expected 'print'");
    consume(TokenType::LPAREN, "Expected '('");
    auto expr = parseExpr();
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<PrintStatement>(std::move(expr));
}

/**
 * Parse break statement
 */
std::unique_ptr<Statement> Parser::parseBreakStatement() {
    consume(TokenType::BREAK, "Expected 'break'");
    consume(TokenType::NEWLINE, "Expected newline");
    return std::make_unique<BreakStatement>();
}

/**
 * Parse continue statement
 */
std::unique_ptr<Statement> Parser::parseContinueStatement() {
    consume(TokenType::CONTINUE, "Expected 'continue'");
    consume(TokenType::NEWLINE, "Expected newline");
    return std::make_unique<ContinueStatement>();
}

/**
 * Parse compound statment
 */
std::unique_ptr<Statement> Parser::parseCompoundStmt() {
    if (check(TokenType::IF)) {
        return parseIfStatement();
    } else if (check(TokenType::WHILE)) {
        return parseWhileStatement();
    }
    
    throw ParseError("Expected compound statement");
}

/**
 * Parse if statment with optional elif/else block
 */
std::unique_ptr<Statement> Parser::parseIfStatement() {
    consume(TokenType::IF, "Expected 'if'");
    auto condition = parseExpr();
    consume(TokenType::COLON, "Expected ':'");
    auto thenBlock = parseBlock();
    
    auto ifStmt = std::make_unique<IfStatement>(std::move(condition), std::move(thenBlock));

    while (check(TokenType::ELIF)) {
        advance();
        auto elifCondition = parseExpr();
        consume(TokenType::COLON, "Expected ':'");
        auto elifBlock = parseBlock();
        ifStmt->addElif(std::move(elifCondition), std::move(elifBlock));
    }

    if (check(TokenType::ELSE)) {
        advance();
        consume(TokenType::COLON, "Expected ':'");
        auto elseBlock = parseBlock();
        ifStmt->setElse(std::move(elseBlock));
    }
    
    return std::move(ifStmt);
}

/**
 * Parse while statement
 */
std::unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenType::WHILE, "Expected 'while'");
    auto condition = parseExpr();
    consume(TokenType::COLON, "Expected ':'");
    auto body = parseBlock();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

/**
 * Parse a block: newline + IDENT + statement + DEDENT
 */
std::unique_ptr<Block> Parser::parseBlock() {
    consume(TokenType::NEWLINE, "Expected newline before block");
    consume(TokenType::INDENT, "Expected indentation");
    
    auto block = std::make_unique<Block>();
    parseStmts(block->statements);
    
    consume(TokenType::DEDENT, "Expected dedent to close block");
    
    return block;
}

/**
 * Parse logical OR operations
 */
std::unique_ptr<Expression> Parser::parseExpr() {
    auto expr = parseJoin();
    
    while (match(TokenType::OR)) {
        auto right = parseJoin();
        expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::OR, std::move(right));
    }
    
    return expr;
}

/**
 * Parse logical AND operations
 */
std::unique_ptr<Expression> Parser::parseJoin() {
    auto expr = parseEquality();
    
    while (match(TokenType::AND)) {
        auto right = parseEquality();
        expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::AND, std::move(right));
    }
    
    return expr;
}

/**
 * Parse equality operations
 */
std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseRel();
    
    while (true) {
        if (match(TokenType::EQUAL)) {
            auto right = parseRel();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::EQUAL, std::move(right));
        } else if (match(TokenType::NOT_EQUAL)) {
            auto right = parseRel();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::NOT_EQUAL, std::move(right));
        } else {
            break;
        }
    }
    
    return expr;
}

/**
 * Parse relational operations
 */
std::unique_ptr<Expression> Parser::parseRel() {
    auto expr = parseNumExpr();
    
    if (match(TokenType::LESS)) {
        auto right = parseNumExpr();
        return std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::LESS, std::move(right));
    } else if (match(TokenType::LESS_EQUAL)) {
        auto right = parseNumExpr();
        return std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::LESS_EQUAL, std::move(right));
    } else if (match(TokenType::GREATER)) {
        auto right = parseNumExpr();
        return std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::GREATER, std::move(right));
    } else if (match(TokenType::GREATER_EQUAL)) {
        auto right = parseNumExpr();
        return std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::GREATER_EQUAL, std::move(right));
    }
    
    return expr;
}

/**
 * Parse numeric expressions
 */
std::unique_ptr<Expression> Parser::parseNumExpr() {
    auto expr = parseTerm();
    
    while (true) {
        if (match(TokenType::PLUS)) {
            auto right = parseTerm();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::ADD, std::move(right));
        } else if (match(TokenType::MINUS)) {
            auto right = parseTerm();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::SUBTRACT, std::move(right));
        } else {
            break;
        }
    }
    
    return expr;
}

/**
 * Parse terms (multiplication and division)
 */
std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseUnary();
    
    while (true) {
        if (match(TokenType::MULTIPLY)) {
            auto right = parseUnary();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::MULTIPLY, std::move(right));
        } else if (match(TokenType::DIVIDE)) {
            auto right = parseUnary();
            expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::DIVIDE, std::move(right));
        } else {
            break;
        }
    }
    
    return expr;
}

/**
 * Parse unary expressions
 */
std::unique_ptr<Expression> Parser::parseUnary() {
    if (match(TokenType::NOT)) {
        auto operand = parseUnary();
        return std::make_unique<UnaryOperation>(UnaryOperation::Operator::NOT, std::move(operand));
    }
    
    if (match(TokenType::MINUS)) {
        auto operand = parseUnary();
        return std::make_unique<UnaryOperation>(UnaryOperation::Operator::MINUS, std::move(operand));
    }
    
    return parseFactor();
}

/**
 * Parse factors (primary expressions)
 */
std::unique_ptr<Expression> Parser::parseFactor() {
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (check(TokenType::NUM)) {
        int value = std::stoi(currentToken().value);
        advance();
        return std::make_unique<NumberLiteral>(value);
    }
    
    if (match(TokenType::TRUE)) {
        return std::make_unique<BooleanLiteral>(true);
    }
    
    if (match(TokenType::FALSE)) {
        return std::make_unique<BooleanLiteral>(false);
    }
    
    if (check(TokenType::ID)) {
        return parseLoc();
    }
    
    throw ParseError("Expected expression");
}

/**
 * Parse location expressions (variables and list access)
 */
std::unique_ptr<Expression> Parser::parseLoc() {
    std::string name = consume(TokenType::ID, "Expected identifier").value;
    
    if (match(TokenType::LBRACKET)) {
        auto index = parseExpr();
        consume(TokenType::RBRACKET, "Expected ']'");
        return std::make_unique<ListAccess>(name, std::move(index));
    }
    
    return std::make_unique<Identifier>(name);
}