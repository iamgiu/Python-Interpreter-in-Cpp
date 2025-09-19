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
            if (source[pos] != '\t' && source[pos] != ' ') { // Aggiungi anche spazi
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
    
    char firstChar = currentChar();
    
    // Se il primo carattere è '0', deve essere l'unico carattere del numero
    if (firstChar == '0') {
        numStr += firstChar;
        advance();
        
        // Se dopo lo 0 c'è un'altra cifra, è un errore
        if (isDigit(currentChar())) {
            return Token(TokenType::ERROR, "Invalid number starting with 0", startLine, startColumn);
        }
        
        return Token(TokenType::NUM, numStr, startLine, startColumn);
    }
    
    // Per numeri che iniziano con [1-9]
    if (firstChar >= '1' && firstChar <= '9') {
        numStr += firstChar;
        advance();
        
        // Poi può seguire [0-9]*
        while (isDigit(currentChar())) {
            numStr += currentChar();
            advance();
        }
        
        return Token(TokenType::NUM, numStr, startLine, startColumn);
    }
    
    // Se arriviamo qui, c'è un errore
    return Token(TokenType::ERROR, "Invalid number", startLine, startColumn);
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
    
    // Conta tabulazioni E spazi all'inizio della linea
    while (currentChar() == '\t' || currentChar() == ' ') {
        if (currentChar() == '\t') {
            indentLevel++;
            advance();
        } else {
            // Conta 4 spazi come 1 livello di indentazione
            int spaces = 0;
            while (currentChar() == ' ' && spaces < 4) {
                spaces++;
                advance();
            }
            if (spaces == 4) {
                indentLevel++;
            } else if (spaces > 0) {
                // Se ci sono spazi ma non multipli di 4, è un errore
                tokens.push_back(Token(TokenType::ERROR, "Inconsistent indentation: spaces must be multiple of 4", line, column));
                return;
            }
        }
    }
    
    // Se la linea è vuota o è un commento, ignora l'indentazione
    if (currentChar() == '\n' || currentChar() == '\0') {
        return;
    }
    
    // CONTROLLO DI SICUREZZA: assicurati che lo stack non sia vuoto
    if (indentStack.empty()) {
        tokens.push_back(Token(TokenType::ERROR, "Internal error: empty indent stack", line, column));
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
        
        // Controllo che l'indentazione sia consistente
        // CONTROLLO DI SICUREZZA: verifica che lo stack non sia vuoto
        if (indentStack.empty()) {
            tokens.push_back(Token(TokenType::ERROR, "Inconsistent indentation: empty stack", line, column));
            return;
        }
        
        if (indentStack.top() != indentLevel) {
            tokens.push_back(Token(TokenType::ERROR, "Inconsistent indentation", line, column));
            return;
        }
    }
    
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
        char c = currentChar();
        
        // Gestisci indentazione all'inizio di ogni linea
        if (atLineStart) {
            handleIndentation();
            // Dopo handleIndentation, riprendi dal carattere corrente
            c = currentChar();
        }
        
        // Fine del file
        if (c == '\0') break;
        
        // Newline
        if (c == '\n') {
            tokens.push_back(Token(TokenType::NEWLINE, "\\n", line, column));
            advance();
            continue;
        }
        
        // Spazi (solo dopo l'indentazione)
        if (c == ' ') {
            skipWhitespace();
            continue;
        }
        
        // Tabulazioni (solo dopo l'indentazione) - RIMUOVI QUESTO BLOCCO
        // Non dovrebbero esserci tab qui se handleIndentation ha fatto il suo lavoro
        
        // Numeri [0-9]
        if (c >= '0' && c <= '9') {
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
                tokens.push_back(Token(TokenType::ERROR, "Unexpected character", startLine, startColumn));
                return tokens; // Ferma subito in caso di errore
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