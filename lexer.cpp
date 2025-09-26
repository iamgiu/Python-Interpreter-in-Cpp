/**
 * Implementation of the Lexer class
 * 
 * Include for std::count and std::endl used in PrintStatement visitor
 * 
 * Include for std::unordered_map used as variable envitoment 
 */
#include "lexer.h"
#include <iostream>
#include <unordered_map>

/**
 * Keyword Map
 */
static std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"list", TokenType::LIST},
    {"print", TokenType::PRINT},
    {"append", TokenType::APPEND},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"True", TokenType::TRUE},
    {"False", TokenType::FALSE}
};

/**
 * Initializes thhe source, current position, line/column number and sets the identation stack
 */
Lexer::Lexer(const std::string& sourceCode) 
    : source(sourceCode), pos(0), line(1), column(1), atLineStart(true) {
    indentStack.push(0);
}

/**
 * Lexer Destroyer
 */
Lexer::~Lexer() {}

/**
 * Returns the current character or '\0' if the end of the file has been reached
 */
char Lexer::currentChar() {
    if (pos >= source.length()) {
        return '\0'; 
    }
    return source[pos];
}

/**
 * Returns the character at a given offset from the current position without advancing the cursor
 */
char Lexer::peekChar(int offset) {
    size_t peekPos = pos + offset;
    if (peekPos >= source.length()) {
        return '\0';
    }
    return source[peekPos];
}

/**
 * Advances the source by one character, updating lines, columns and the atLineStart state
 */
void Lexer::advance() {
    if (pos < source.length()) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
            atLineStart = true;
        } else {
            column++;
            if (source[pos] != '\t' && source[pos] != ' ') {
                atLineStart = false;
            }
        }
        pos++;
    }
}

/**
 * Skip al consecutive spaces
 */
void Lexer::skipWhitespace() {
    while (currentChar() == ' ') {
        advance();
    }
}

/**
 * Checks if a character is a digit
 */
bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

/**
 * Checks if a character is an alphabetic letter
 */
bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * Check if a character is alphanumeric
 */
bool Lexer::isAlphaNum(char c) {
    return isAlpha(c) || isDigit(c);
}

/**
 * Recognized and creates a numeber:
 * - accepts zero alone
 * - otherwise it must begin with a digit from 1 to 9 and can have other digits after it
 */
Token Lexer::makeNumber() {
    std::string numStr;
    int startLine = line;
    int startColumn = column;
    
    char firstChar = currentChar();
    
    if (firstChar == '0') {
        numStr += firstChar;
        advance();

        if (isDigit(currentChar())) {
            return Token(TokenType::ERROR, "Numbers cannot start with 0 unless they are just 0", startLine, startColumn);
        }
        
        return Token(TokenType::NUM, numStr, startLine, startColumn);
    }

    if (firstChar >= '1' && firstChar <= '9') {
        numStr += firstChar;
        advance();

        while (isDigit(currentChar())) {
            numStr += currentChar();
            advance();
        }
        
        return Token(TokenType::NUM, numStr, startLine, startColumn);
    }
    
    return Token(TokenType::ERROR, "Invalid number format", startLine, startColumn);
}

/**
 * Recognized and creates an identifier or keyword:
 * - must begin with a letter (a-z or A-Z)
 * - can contain subsequent letters and numbers
 * - if the name matches a keyword, generates the keyword token
 * - else generates an identifier token
 */
Token Lexer::makeIdentifier() {
    std::string idStr;
    int startLine = line;
    int startColumn = column;

    if (!isAlpha(currentChar())) {
        return Token(TokenType::ERROR, "Invalid identifier", startLine, startColumn);
    }
    
    idStr += currentChar();
    advance();

    while (isAlphaNum(currentChar())) {
        idStr += currentChar();
        advance();
    }

    auto it = keywords.find(idStr);
    if (it != keywords.end()) {
        return Token(it->second, idStr, startLine, startColumn);
    }
    
    return Token(TokenType::ID, idStr, startLine, startColumn);
}

/**
 * Supports two character operators (==, !=, <=, >=, //) or single-character operators (=, <, >)
 */
Token Lexer::makeTwoCharOperator() {
    int startLine = line;
    int startColumn = column;
    char first = currentChar();
    char second = peekChar();
    
    std::string op;
    op += first;
    
    if (first == '=' && second == '=') {
        advance(); 
        advance(); 
        return Token(TokenType::EQUAL, "==", startLine, startColumn);
    }
    if (first == '!' && second == '=') {
        advance(); 
        advance(); 
        return Token(TokenType::NOT_EQUAL, "!=", startLine, startColumn);
    }
    if (first == '<' && second == '=') {
        advance(); 
        advance(); 
        return Token(TokenType::LESS_EQUAL, "<=", startLine, startColumn);
    }
    if (first == '>' && second == '=') {
        advance(); 
        advance(); 
        return Token(TokenType::GREATER_EQUAL, ">=", startLine, startColumn);
    }
    if (first == '/' && second == '/') {
        advance(); 
        advance(); 
        return Token(TokenType::DIVIDE, "//", startLine, startColumn);
    }
    
    // Operatore singolo
    advance();
    switch (first) {
        case '=': return Token(TokenType::ASSIGN, "=", startLine, startColumn);
        case '<': return Token(TokenType::LESS, "<", startLine, startColumn);
        case '>': return Token(TokenType::GREATER, ">", startLine, startColumn);
        default: return Token(TokenType::ERROR, "Unknown operator", startLine, startColumn);
    }
}

/**
 * Manages identation at the beginning of the line, generates IDENT/DEDENT tokens and detects mixed ot inconsistent identation errors
 */
void Lexer::handleIndentation() {
    if (!atLineStart) return;
    
    int indentChars = 0;
    bool mixedIndentation = false;
    char firstIndentChar = '\0';

    while (currentChar() == '\t' || currentChar() == ' ') {
        char c = currentChar();
        if (firstIndentChar == '\0') {
            firstIndentChar = c;
        } else if (firstIndentChar != c) {
            mixedIndentation = true;
        }
        indentChars++;
        advance();
    }

    if (currentChar() == '\n' || currentChar() == '\0') {
        return;
    }

    if (mixedIndentation) {
        tokens.push_back(Token(TokenType::ERROR, "IndentationError: inconsistent use of tabs and spaces in indentation", line, column));
        return;
    }

    int indentLevel;
    if (firstIndentChar == '\t' || indentChars == 0) {
        indentLevel = indentChars;
    } else {
        if (indentChars % 2 != 0) {
            tokens.push_back(Token(TokenType::ERROR, "IndentationError: unindent does not match any outer indentation level", line, column));
            return;
        }
        indentLevel = indentChars / 2; 
    }
    
    if (indentStack.empty()) {
        tokens.push_back(Token(TokenType::ERROR, "Internal error: empty indent stack", line, column));
        return;
    }
    
    int currentIndent = indentStack.top();
    
    if (indentLevel > currentIndent) {
        indentStack.push(indentLevel);
        tokens.push_back(Token(TokenType::INDENT, "", line, column));
    } else if (indentLevel < currentIndent) {
        while (!indentStack.empty() && indentStack.top() > indentLevel) {
            indentStack.pop();
            tokens.push_back(Token(TokenType::DEDENT, "", line, column));
        }
        if (indentStack.empty() || indentStack.top() != indentLevel) {
            tokens.push_back(Token(TokenType::ERROR, "IndentationError: unindent does not match any outer indentation level", line, column));
            return;
        }
    }
    
    atLineStart = false;
}

/**
 * At the end of the file generate as many DEDENT tokens as needed to reset the identation to level 0
 */
void Lexer::addDedentTokens() {
    while (!indentStack.empty() && indentStack.top() > 0) {
        indentStack.pop();
        tokens.push_back(Token(TokenType::DEDENT, "", line, column));
    }
}

/**
 * Read the source character by character
 * 
 * Generates tokens and manages newlines, identation, numbers, identifiers and operators
 */
std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    
    while (currentChar() != '\0') {
        char c = currentChar();

        if (atLineStart) {
            handleIndentation();
            if (!tokens.empty() && tokens.back().type == TokenType::ERROR) {
                return tokens;
            }
            c = currentChar();
        }

        if (c == '\0') break;

        if (c == '\n') {
            tokens.push_back(Token(TokenType::NEWLINE, "\\n", line, column));
            advance();
            continue;
        }

        if (c == ' ') {
            skipWhitespace();
            continue;
        }
        
        if (c >= '0' && c <= '9') {
            Token numToken = makeNumber();
            tokens.push_back(numToken);
            if (numToken.type == TokenType::ERROR) {
                return tokens;
            }
            continue;
        }
 
        if (isAlpha(c)) {
            Token idToken = makeIdentifier();
            tokens.push_back(idToken);
            if (idToken.type == TokenType::ERROR) {
                return tokens;
            }
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '/') {
            Token opToken = makeTwoCharOperator();
            tokens.push_back(opToken);
            if (opToken.type == TokenType::ERROR) {
                return tokens;
            }
            continue;
        }

        int startLine = line;
        int startColumn = column;
        advance();
        
        switch (c) {
            case '+': tokens.push_back(Token(TokenType::PLUS, "+", startLine, startColumn)); break;
            case '-': tokens.push_back(Token(TokenType::MINUS, "-", startLine, startColumn)); break;
            case '*': tokens.push_back(Token(TokenType::MULTIPLY, "*", startLine, startColumn)); break;
            case '(': tokens.push_back(Token(TokenType::LPAREN, "(", startLine, startColumn)); break;
            case ')': tokens.push_back(Token(TokenType::RPAREN, ")", startLine, startColumn)); break;
            case '[': tokens.push_back(Token(TokenType::LBRACKET, "[", startLine, startColumn)); break;
            case ']': tokens.push_back(Token(TokenType::RBRACKET, "]", startLine, startColumn)); break;
            case ':': tokens.push_back(Token(TokenType::COLON, ":", startLine, startColumn)); break;
            case '.': tokens.push_back(Token(TokenType::DOT, ".", startLine, startColumn)); break;
            case ',': tokens.push_back(Token(TokenType::COMMA, ",", startLine, startColumn)); break;
            default: 
                tokens.push_back(Token(TokenType::ERROR, "Unexpected character", startLine, startColumn));
                return tokens; 
        }
    }

    addDedentTokens();
    tokens.push_back(Token(TokenType::ENDMARKER, "EOF", line, column));
    
    return tokens;
}

/**
 * Prints all generates tokens useful for debugging
 */
void Lexer::printTokens() const {
    for (const auto& token : tokens) {
        std::cout << "Token(" << static_cast<int>(token.type) 
                  << ", \"" << token.value << "\", " 
                  << token.line << ":" << token.column << ")" << std::endl;
    }
}