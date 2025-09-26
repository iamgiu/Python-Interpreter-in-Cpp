/**
 * Guard Headers
 */
#ifndef LEXER_H
#define LEXER_H

/**
 * Include fot std::string used to store source code and token values
 * 
 * Include for std::vector used to store the list of generated tokens
 * 
 * Include for std::stack used to manage identation level
 * 
 * Include for std::ifstream if needed to read from file
 * 
 * Include for std::unordered_map used for keyword lookup in makeIdentifier
 */
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <unordered_map>

/**
 * Token types enumeration
 * 
 * Each tolen represent a lexical element of simplified Python-like language
 */
enum class TokenType {
    NUM,           // number literal [1-9][0-9]* | 0
    ID,            // identifier [a-zA-Z][0-9a-zA-Z]*
    TRUE,          // True
    FALSE,         // False
    
    PLUS,          // +
    MINUS,         // -
    MULTIPLY,      // *
    DIVIDE,        // //
    
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=
    EQUAL,         // ==
    NOT_EQUAL,     // !=
    
    AND,           // and
    OR,            // or
    NOT,           // not
    
    IF,            // if
    ELIF,          // elif
    ELSE,          // else
    WHILE,         // while
    BREAK,         // break
    CONTINUE,      // continue
    LIST,          // list
    PRINT,         // print
    APPEND,        // append
    
    ASSIGN,        // =
    LPAREN,        // (
    RPAREN,        // )
    LBRACKET,      // [
    RBRACKET,      // ]
    COLON,         // :
    DOT,           // .
    COMMA,         // ,
    
    NEWLINE,       // \n
    INDENT,        // identation level increases
    DEDENT,        // identation level decreases
    ENDMARKER,     // EOF
    
    ERROR
};

/**
 * Represents a single token with type, text value and position info
 */
struct Token {
    TokenType type;      
    std::string value;   
    int line;          
    int column;
    
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

/**
 * Lexical Analyzer
 * 
 * Converts the input source code into a sequence of tokens following the lexical rules
 */
class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int column;
    std::stack<int> indentStack;
    std::vector<Token> tokens;
    bool atLineStart;

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

    void printTokens() const;
};

#endif // LEXER_H