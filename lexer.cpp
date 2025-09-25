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
            if (source[pos] != '\t' && source[pos] != ' ') {
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
    
    // CORREZIONE: Rispetta la regex num → [1-9][0-9]* | 0
    // Gestisce lo zero come caso speciale
    if (firstChar == '0') {
        numStr += firstChar;
        advance();
        
        // Zero deve essere un numero a se stante
        if (isDigit(currentChar())) {
            return Token(TokenType::ERROR, "Numbers cannot start with 0 unless they are just 0", startLine, startColumn);
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
    return Token(TokenType::ERROR, "Invalid number format", startLine, startColumn);
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
        default: return Token(TokenType::ERROR, "Unknown operator", startLine, startColumn);
    }
}

void Lexer::handleIndentation() {
    if (!atLineStart) return;
    
    int indentChars = 0;
    bool mixedIndentation = false;
    char firstIndentChar = '\0';
    
    // Conta caratteri di indentazione e verifica consistenza
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
    
    // Se la linea è vuota, ignora l'indentazione
    if (currentChar() == '\n' || currentChar() == '\0') {
        return;
    }
    
    // Errore per indentazione mista nella stessa linea
    if (mixedIndentation) {
        tokens.push_back(Token(TokenType::ERROR, "IndentationError: inconsistent use of tabs and spaces in indentation", line, column));
        return;
    }
    
    // Calcola il livello di indentazione
    int indentLevel;
    if (firstIndentChar == '\t' || indentChars == 0) {
        // Tab o nessuna indentazione: 1 tab = 1 livello
        indentLevel = indentChars;
    } else {
        // Spazi: converti in livelli (più tollerante)
        if (indentChars % 2 != 0) {
            tokens.push_back(Token(TokenType::ERROR, "IndentationError: unindent does not match any outer indentation level", line, column));
            return;
        }
        indentLevel = indentChars / 2; // 2 spazi = 1 livello base
    }
    
    // SAFETY CHECK: assicurati che lo stack non sia vuoto
    if (indentStack.empty()) {
        tokens.push_back(Token(TokenType::ERROR, "Internal error: empty indent stack", line, column));
        return;
    }
    
    int currentIndent = indentStack.top();
    
    if (indentLevel > currentIndent) {
        // Aumento indentazione - genera INDENT
        indentStack.push(indentLevel);
        tokens.push_back(Token(TokenType::INDENT, "", line, column));
    } else if (indentLevel < currentIndent) {
        // Diminuzione indentazione - genera DEDENT
        while (!indentStack.empty() && indentStack.top() > indentLevel) {
            indentStack.pop();
            tokens.push_back(Token(TokenType::DEDENT, "", line, column));
        }
        
        // Verifica che il livello sia presente nello stack
        if (indentStack.empty() || indentStack.top() != indentLevel) {
            tokens.push_back(Token(TokenType::ERROR, "IndentationError: unindent does not match any outer indentation level", line, column));
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
            // Controlla errori di indentazione immediatamente
            if (!tokens.empty() && tokens.back().type == TokenType::ERROR) {
                return tokens; // Ferma al primo errore
            }
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
        
        // Numeri [0-9] - include ora anche lo zero
        if (c >= '0' && c <= '9') {
            Token numToken = makeNumber();
            tokens.push_back(numToken);
            if (numToken.type == TokenType::ERROR) {
                return tokens;
            }
            continue;
        }
        
        // Identificatori e parole chiave [a-zA-Z][0-9a-zA-Z]*
        if (isAlpha(c)) {
            Token idToken = makeIdentifier();
            tokens.push_back(idToken);
            if (idToken.type == TokenType::ERROR) {
                return tokens;
            }
            continue;
        }
        
        // Operatori che possono essere doppi
        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '/') {
            Token opToken = makeTwoCharOperator();
            tokens.push_back(opToken);
            if (opToken.type == TokenType::ERROR) {
                return tokens;
            }
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