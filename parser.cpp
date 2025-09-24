#include "parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokenStream) : tokens(tokenStream), currentPos(0) {
    // Assicurati che ci sia sempre un ENDMARKER alla fine
    if (tokens.empty() || tokens.back().type != TokenType::ENDMARKER) {
        tokens.push_back(Token(TokenType::ENDMARKER, "EOF", 0, 0));
    }
}

const Token& Parser::currentToken() {
    // Controllo di sicurezza: se siamo fuori bounds, restituiamo l'ultimo token
    if (currentPos >= tokens.size()) {
        // Restituisce l'ultimo token (dovrebbe essere ENDMARKER)
        return tokens.back();
    }
    return tokens[currentPos];
}

const Token& Parser::peekToken(int offset) {
    size_t pos = currentPos + offset;
    // Controllo di sicurezza: se siamo fuori bounds, restituiamo l'ultimo token
    if (pos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos];
}

void Parser::advance() {
    if (!isAtEnd()) {
        currentPos++;
    }
}

bool Parser::isAtEnd() {
    return currentPos >= tokens.size() || currentToken().type == TokenType::ENDMARKER;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return currentToken().type == type;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        Token token = currentToken();
        advance();
        return token;
    }
    throw ParseError(message);
}

std::unique_ptr<Program> Parser::parseProgram() {
    return parseProgram_();
}

std::unique_ptr<Program> Parser::parseProgram_() {
    auto program = std::make_unique<Program>();
    
    parseStmts(program->statements);
    
    // Salta token finali (DEDENT e NEWLINE)
    while (currentPos < tokens.size() && 
           (tokens[currentPos].type == TokenType::DEDENT || 
            tokens[currentPos].type == TokenType::NEWLINE)) {
        currentPos++;
    }
    
    // Dovremmo essere all'ENDMARKER
    if (currentPos >= tokens.size()) {
        throw ParseError("Unexpected end of token stream");
    }
    
    if (tokens[currentPos].type != TokenType::ENDMARKER) {
        throw ParseError("Expected ENDMARKER");
    }
    
    return program;
}

void Parser::parseStmts(std::vector<std::unique_ptr<Statement>>& statements) {
    // <stmts> → <stmt> <stmts> | ε
    while (!isAtEnd() && !check(TokenType::ENDMARKER) && !check(TokenType::DEDENT)) {
        
        // Salta eventuali NEWLINE vuoti
        while (check(TokenType::NEWLINE)) {
            advance();
        }
        
        // Se dopo aver saltato i NEWLINE siamo alla fine, esci
        if (isAtEnd() || check(TokenType::ENDMARKER) || check(TokenType::DEDENT)) {
            break;
        }
        
        auto stmt = parseStmt();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
}

std::unique_ptr<Statement> Parser::parseStmt() {
    // <stmt> → <compound_stmt> | <simple_stmt>
    if (check(TokenType::IF) || check(TokenType::WHILE)) {
        return parseCompoundStmt();
    } else {
        return parseSimpleStmt();
    }
}

std::unique_ptr<Statement> Parser::parseSimpleStmt() {
    // Parsing delle simple statements
    if (check(TokenType::BREAK)) {
        return parseBreakStatement();
    } else if (check(TokenType::CONTINUE)) {
        return parseContinueStatement();
    } else if (check(TokenType::PRINT)) {
        return parsePrintStatement();
    } else if (check(TokenType::ID)) {
        
        // Controlla il secondo token per determinare il tipo di statement
        if (currentPos + 1 < tokens.size()) {
            TokenType secondToken = peekToken().type;
            
            if (secondToken == TokenType::ASSIGN) {
                // Controlla il terzo token per list creation
                if (currentPos + 2 < tokens.size()) {
                    TokenType thirdToken = peekToken(2).type;
                    
                    if (thirdToken == TokenType::LIST) {
                        return parseListCreation();
                    }
                }
                
                return parseAssignment();
                
            } else if (secondToken == TokenType::LBRACKET) {
                return parseAssignment(); // List assignment
            } else if (secondToken == TokenType::DOT) {
                return parseListAppend();
            }
        }
    }
    
    throw ParseError("Unexpected token in simple statement");
}

std::unique_ptr<Statement> Parser::parseAssignment() {
    // <loc> = <expr> newline
    if (check(TokenType::ID)) {
        std::string varName = currentToken().value;
        advance(); // consume ID
        
        if (check(TokenType::LBRACKET)) {
            // List assignment: x[i] = expr
            advance(); // consume [
            auto index = parseExpr();
            consume(TokenType::RBRACKET, "Expected ']'");
            consume(TokenType::ASSIGN, "Expected '='");
            auto value = parseExpr();
            consume(TokenType::NEWLINE, "Expected newline");
            
            return std::make_unique<ListAssignment>(varName, std::move(index), std::move(value));
        } else {
            // Regular assignment: x = expr
            consume(TokenType::ASSIGN, "Expected '='");
            auto value = parseExpr();
            consume(TokenType::NEWLINE, "Expected newline");
            
            return std::make_unique<Assignment>(varName, std::move(value));
        }
    }
    
    throw ParseError("Expected identifier in assignment");
}

std::unique_ptr<Statement> Parser::parseListCreation() {
    // <id> = list() newline
    std::string varName = consume(TokenType::ID, "Expected identifier").value;
    consume(TokenType::ASSIGN, "Expected '='");
    consume(TokenType::LIST, "Expected 'list'");
    consume(TokenType::LPAREN, "Expected '('");
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<ListCreation>(varName);
}

std::unique_ptr<Statement> Parser::parseListAppend() {
    // <id> . append ( <expr> ) newline
    std::string listName = consume(TokenType::ID, "Expected identifier").value;
    consume(TokenType::DOT, "Expected '.'");
    consume(TokenType::APPEND, "Expected 'append'");
    consume(TokenType::LPAREN, "Expected '('");
    auto value = parseExpr();
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<ListAppend>(listName, std::move(value));
}

std::unique_ptr<Statement> Parser::parsePrintStatement() {
    // print ( <expr> ) newline
    consume(TokenType::PRINT, "Expected 'print'");
    consume(TokenType::LPAREN, "Expected '('");
    auto expr = parseExpr();
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::NEWLINE, "Expected newline");
    
    return std::make_unique<PrintStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::parseBreakStatement() {
    // break newline
    consume(TokenType::BREAK, "Expected 'break'");
    consume(TokenType::NEWLINE, "Expected newline");
    return std::make_unique<BreakStatement>();
}

std::unique_ptr<Statement> Parser::parseContinueStatement() {
    // continue newline
    consume(TokenType::CONTINUE, "Expected 'continue'");
    consume(TokenType::NEWLINE, "Expected newline");
    return std::make_unique<ContinueStatement>();
}

std::unique_ptr<Statement> Parser::parseCompoundStmt() {
    // <compound_stmt> → <if_stmt> | <while_stmt>
    if (check(TokenType::IF)) {
        return parseIfStatement();
    } else if (check(TokenType::WHILE)) {
        return parseWhileStatement();
    }
    
    throw ParseError("Expected compound statement");
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    // <if_stmt> → if <expr> : <block>
    //           | if <expr> : <block> <elif_block>
    //           | if <expr> : <block> <else_block>
    
    consume(TokenType::IF, "Expected 'if'");
    auto condition = parseExpr();
    consume(TokenType::COLON, "Expected ':'");
    auto thenBlock = parseBlock();
    
    auto ifStmt = std::make_unique<IfStatement>(std::move(condition), std::move(thenBlock));
    
    // Parse elif clauses
    while (check(TokenType::ELIF)) {
        advance(); // consume 'elif'
        auto elifCondition = parseExpr();
        consume(TokenType::COLON, "Expected ':'");
        auto elifBlock = parseBlock();
        ifStmt->addElif(std::move(elifCondition), std::move(elifBlock));
    }
    
    // Parse optional else clause
    if (check(TokenType::ELSE)) {
        advance(); // consume 'else'
        consume(TokenType::COLON, "Expected ':'");
        auto elseBlock = parseBlock();
        ifStmt->setElse(std::move(elseBlock));
    }
    
    return std::move(ifStmt);
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    // <while_stmt> → while <expr> : <block>
    consume(TokenType::WHILE, "Expected 'while'");
    auto condition = parseExpr();
    consume(TokenType::COLON, "Expected ':'");
    auto body = parseBlock();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Block> Parser::parseBlock() {
    // <block> → newline indent <stmts> dedent
    consume(TokenType::NEWLINE, "Expected newline before block");
    consume(TokenType::INDENT, "Expected indentation");
    
    auto block = std::make_unique<Block>();
    parseStmts(block->statements);
    
    consume(TokenType::DEDENT, "Expected dedent to close block");
    
    return block;
}

std::unique_ptr<Expression> Parser::parseExpr() {
    // <expr> → <expr> or <join> | <join>
    auto expr = parseJoin();
    
    while (match(TokenType::OR)) {
        auto right = parseJoin();
        expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::OR, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseJoin() {
    // <join> → <join> and <equality> | <equality>
    auto expr = parseEquality();
    
    while (match(TokenType::AND)) {
        auto right = parseEquality();
        expr = std::make_unique<BinaryOperation>(std::move(expr), BinaryOperation::Operator::AND, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    // <equality> → <equality> == <rel> | <equality> != <rel> | <rel>
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

std::unique_ptr<Expression> Parser::parseRel() {
    // <rel> → <numexpr> < <numexpr> | <numexpr> <= <numexpr> | ... | <numexpr>
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

std::unique_ptr<Expression> Parser::parseNumExpr() {
    // <numexpr> → <numexpr> + <term> | <numexpr> - <term> | <term>
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

std::unique_ptr<Expression> Parser::parseTerm() {
    // <term> → <term> * <unary> | <term> // <unary> | <unary>
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

std::unique_ptr<Expression> Parser::parseUnary() {
    // <unary> → not <unary> | - <unary> | <factor>
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

std::unique_ptr<Expression> Parser::parseFactor() {
    // <factor> → ( <expr> ) | <loc> | num | True | False
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

std::unique_ptr<Expression> Parser::parseLoc() {
    // <loc> → id | id [ <expr> ]
    std::string name = consume(TokenType::ID, "Expected identifier").value;
    
    if (match(TokenType::LBRACKET)) {
        auto index = parseExpr();
        consume(TokenType::RBRACKET, "Expected ']'");
        return std::make_unique<ListAccess>(name, std::move(index));
    }
    
    return std::make_unique<Identifier>(name);
}