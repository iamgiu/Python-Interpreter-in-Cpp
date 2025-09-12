#include "lexer.h"
#include <iostream>
#include <unordered_map>

// Mappa delle parole chiave
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

Lexer::Lexer(const std::string& sourceCode) 
    : source(sourceCode), pos(0), line(1), column(1), atLineStart(true) {
    // Inizializza lo stack con 0 (come da specifica)
    indentStack.push(0);
}

Lexer::~Lexer() {
    // Niente da fare per ora
}

char Lexer::currentChar() {
    if (pos >= source.length()) {
        return '\0'; // EOF
    }
    return source[pos];
}

char Lexer::peekChar(int offset) {
    size_t peekPos = pos + offset;
    if (peekPos >= source.length()) {
        return '\0';
    }
    return source[peekPos];
}

void Lexer::advance() {
    if (pos < source.length()) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
            atLineStart = true;
        } else {
            column++;
            if (source[pos] != '\t') {
                atLineStart = false;
            }
        }
        pos++;
    }
}

void Lexer::skipWhitespace() {
    while (currentChar() == ' ') {
        advance();
    }
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isAlphaNum(char c) {
    return isAlpha(c) || isDigit(c);
}

Token Lexer::makeNumber() {
    std::string numStr;
    int startLine = line;
    int startColumn = column;
    
    // Primo carattere deve essere [1-9] secondo la specifica
    if (currentChar() < '1' || currentChar() > '9') {
        return Token(TokenType::ERROR, "Invalid number", startLine, startColumn);
    }
    
    numStr += currentChar();
    advance();
    
    // Poi può seguire [0-9]*
    while (isDigit(currentChar())) {
        numStr += currentChar();
        advance();
    }
    
    return Token(TokenType::NUM, numStr, startLine, startColumn);
}

Token Lexer::makeIdentifier() {
    std::string idStr;
    int startLine = line;
    int startColumn = column;
    
    // Primo carattere deve essere [a-zA-Z]
    if (!isAlpha(currentChar())) {
        return Token(TokenType::ERROR, "Invalid identifier", startLine, startColumn);
    }
    
    idStr += currentChar();
    advance();
    
    // Poi può seguire [0-9a-zA-Z]*
    while (isAlphaNum(currentChar())) {
        idStr += currentChar();
        advance();
    }
    
    // Controlla se è una parola chiave
    auto it = keywords.find(idStr);
    if (it != keywords.end()) {
        return Token(it->second, idStr, startLine, startColumn);
    }
    
    return Token(TokenType::ID, idStr, startLine, startColumn);
}

Token Lexer::makeTwoCharOperator() {
    int startLine = line;
    int startColumn = column;
    char first = currentChar();
    char second = peekChar();
    
    std::string op;
    op += first;
    
    if (first == '=' && second == '=') {
        advance(); // primo =
        advance(); // secondo =
        return Token(TokenType::EQUAL, "==", startLine, startColumn);
    }
    if (first == '!' && second == '=') {
        advance(); // !
        advance(); // =
        return Token(TokenType::NOT_EQUAL, "!=", startLine, startColumn);
    }
    if (first == '<' && second == '=') {
        advance(); // <
        advance(); // =
        return Token(TokenType::LESS_EQUAL, "<=", startLine, startColumn);
    }
    if (first == '>' && second == '=') {
        advance(); // >
        advance(); // =
        return Token(TokenType::GREATER_EQUAL, ">=", startLine, startColumn);
    }
    if (first == '/' && second == '/') {
        advance(); // primo /
        advance(); // secondo /
        return Token(TokenType::DIVIDE, "//", startLine, startColumn);
    }
    
    // Operatore singolo
    advance();
    switch (first) {
        case '=': return Token(TokenType::ASSIGN, "=", startLine, startColumn);
        case '<': return Token(TokenType::LESS, "<", startLine, startColumn);
        case '>': return Token(TokenType::GREATER, ">", startLine, startColumn);
        default: return Token(TokenType::ERROR, op, startLine, startColumn);
    }
}

void Lexer::handleIndentation() {
    if (!atLineStart) return;
    
    int indentLevel = 0;
    
    // Conta le tabulazioni all'inizio della linea
    while (currentChar() == '\t') {
        indentLevel++;
        advance();
    }
    
    // Se la linea è vuota o è un commento, ignora l'indentazione
    if (currentChar() == '\n' || currentChar() == '\0') {
        return;
    }
    
    int currentIndent = indentStack.top();
    
    if (indentLevel > currentIndent) {
        // Aumento di indentazione - genera INDENT
        indentStack.push(indentLevel);
        tokens.push_back(Token(TokenType::INDENT, "", line, column));
    } else if (indentLevel < currentIndent) {
        // Diminuzione di indentazione - genera DEDENT
        while (!indentStack.empty() && indentStack.top() > indentLevel) {
            indentStack.pop();
            tokens.push_back(Token(TokenType::DEDENT, "", line, column));
        }
        
        // Controlla che l'indentazione sia consistente
        if (indentStack.empty() || indentStack.top() != indentLevel) {
            tokens.push_back(Token(TokenType::ERROR, "Inconsistent indentation", line, column));
            return;
        }
    }
    // Se indentLevel == currentIndent, non genera nessun token
    
    atLineStart = false;
}

void Lexer::addDedentTokens() {
    // Al termine del file, genera DEDENT per tutti i livelli > 0
    while (!indentStack.empty() && indentStack.top() > 0) {
        indentStack.pop();
        tokens.push_back(Token(TokenType::DEDENT, "", line, column));
    }
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    
    while (currentChar() != '\0') {
        // Gestisci indentazione all'inizio di ogni linea
        if (atLineStart) {
            handleIndentation();
        }
        
        char c = currentChar();
        
        if (c == ' ') {
            skipWhitespace();
            continue;
        }
        
        if (c == '\n') {
            tokens.push_back(Token(TokenType::NEWLINE, "\\n", line, column));
            advance();
            continue;
        }
        
        if (c == '\t') {
            advance(); // Le tabulazioni sono gestite in handleIndentation
            continue;
        }
        
        // Numeri [1-9][0-9]*
        if (c >= '1' && c <= '9') {
            tokens.push_back(makeNumber());
            continue;
        }
        
        // Identificatori e parole chiave [a-zA-Z][0-9a-zA-Z]*
        if (isAlpha(c)) {
            tokens.push_back(makeIdentifier());
            continue;
        }
        
        // Operatori che possono essere doppi
        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '/') {
            tokens.push_back(makeTwoCharOperator());
            continue;
        }
        
        // Operatori e punteggiatura singoli
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
                tokens.push_back(Token(TokenType::ERROR, std::string(1, c), startLine, startColumn));
                break;
        }
    }
    
    // Aggiungi DEDENT finali e ENDMARKER
    addDedentTokens();
    tokens.push_back(Token(TokenType::ENDMARKER, "EOF", line, column));
    
    return tokens;
}

void Lexer::printTokens() const {
    for (const auto& token : tokens) {
        std::cout << "Token(" << static_cast<int>(token.type) 
                  << ", \"" << token.value << "\", " 
                  << token.line << ":" << token.column << ")" << std::endl;
    }
}