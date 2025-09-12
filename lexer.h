#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <unordered_map>

// Tipi di token come da specifica
enum class TokenType {
    // Letterali
    NUM,           // numeri [1-9][0-9]*
    ID,            // identificatori [a-zA-Z][0-9a-zA-Z]*
    TRUE,          // True
    FALSE,         // False
    
    // Operatori aritmetici
    PLUS,          // +
    MINUS,         // -
    MULTIPLY,      // *
    DIVIDE,        // //
    
    // Operatori relazionali
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=
    EQUAL,         // ==
    NOT_EQUAL,     // !=
    
    // Operatori booleani
    AND,           // and
    OR,            // or
    NOT,           // not
    
    // Parole chiave
    IF,            // if
    ELIF,          // elif
    ELSE,          // else
    WHILE,         // while
    BREAK,         // break
    CONTINUE,      // continue
    LIST,          // list
    PRINT,         // print
    APPEND,        // append
    
    // Punteggiatura
    ASSIGN,        // =
    LPAREN,        // (
    RPAREN,        // )
    LBRACKET,      // [
    RBRACKET,      // ]
    COLON,         // :
    DOT,           // .
    COMMA,         // ,
    
    // Token speciali
    NEWLINE,       // \n
    INDENT,        // aumento indentazione
    DEDENT,        // diminuzione indentazione
    ENDMARKER,     // EOF
    
    // Errore
    ERROR
};

// Struttura per rappresentare un token
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
private:
    std::string source;           // Codice sorgente
    size_t pos;                   // Posizione corrente
    int line;                     // Linea corrente
    int column;                   // Colonna corrente
    std::stack<int> indentStack;  // Stack per indentazione
    std::vector<Token> tokens;    // Token generati
    bool atLineStart;             // Flag per inizio linea
    
    // Metodi privati
    char currentChar();
    char peekChar(int offset = 1);
    void advance();
    void skipWhitespace();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNum(char c);
    
    Token makeNumber();
    Token makeIdentifier();
    Token makeTwoCharOperator();
    void handleIndentation();
    void addDedentTokens();
    
public:
    Lexer(const std::string& sourceCode);
    ~Lexer();
    
    std::vector<Token> tokenize();
    void printTokens() const; // Per debug
};

#endif // LEXER_H