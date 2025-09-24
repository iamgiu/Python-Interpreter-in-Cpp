#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <unordered_map>

// Tipi di token come da specifica del progetto
// Ogni token rappresenta un elemento lessicale del linguaggio Python semplificato
enum class TokenType {
    // Letterali
    NUM,           // numeri [1-9][0-9]* | 0
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
    
    // Token speciali per gestione struttura del codice
    NEWLINE,       // \n
    INDENT,        // aumento indentazione (generato automaticamente)
    DEDENT,        // diminuzione indentazione (generato automaticamente)
    ENDMARKER,     // EOF
    
    // Errore lessicale
    ERROR
};

// Struttura per rappresentare un token con informazioni di posizione
struct Token {
    TokenType type;        // Tipo del token
    std::string value;     // Valore testuale del token
    int line;             // Numero di linea nel codice sorgente
    int column;           // Numero di colonna nel codice sorgente
    
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

/**
 * Lexer (Analizzatore Lessicale)
 * 
 * Converte il codice sorgente in una sequenza di token seguendo le regole
 * lessicali specificate nel documento del progetto. Gestisce in particolare:
 * - Riconoscimento di numeri, identificatori e parole chiave
 * - Operatori aritmetici, relazionali e booleani  
 * - Gestione dell'indentazione con stack per generare INDENT/DEDENT
 * - Rilevamento e segnalazione di errori lessicali
 */
class Lexer {
private:
    std::string source;           // Codice sorgente da analizzare
    size_t pos;                   // Posizione corrente nel codice
    int line;                     // Linea corrente (inizia da 1)
    int column;                   // Colonna corrente (inizia da 1)
    std::stack<int> indentStack;  // Stack per gestire livelli di indentazione
    std::vector<Token> tokens;    // Token generati dall'analisi
    bool atLineStart;             // Flag per rilevare inizio di nuova linea
    
    // === METODI PRIVATI PER NAVIGAZIONE DEL CODICE ===
    
    /** Restituisce il carattere corrente (o '\0' se fine file) */
    char currentChar();
    
    /** Guarda avanti di 'offset' caratteri senza avanzare la posizione */
    char peekChar(int offset = 1);
    
    /** Avanza di una posizione aggiornando line e column */
    void advance();
    
    /** Salta spazi (ma non tabulazioni, usate per indentazione) */
    void skipWhitespace();
    
    // === HELPER PER RICONOSCIMENTO CARATTERI ===
    
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNum(char c);
    
    // === METODI PER PARSING DI TOKEN SPECIFICI ===
    
    /**
     * Costruisce un token NUM seguendo la regex: [1-9][0-9]* | 0
     * Gestisce correttamente il caso speciale dello zero
     */
    Token makeNumber();
    
    /**
     * Costruisce un token ID o parola chiave seguendo: [a-zA-Z][0-9a-zA-Z]*
     * Controlla se l'identificatore è una parola riservata
     */
    Token makeIdentifier();
    
    /**
     * Gestisce operatori che possono essere singoli o doppi
     * Es: = vs ==, < vs <=, / vs //
     */
    Token makeTwoCharOperator();
    
    /**
     * GESTIONE DELL'INDENTAZIONE (algoritmo da specifica)
     * 
     * Utilizza uno stack S inizializzato con 0. Per ogni linea:
     * - Se nt = top(S): nessun token
     * - Se nt > top(S): push(nt), genera INDENT
     * - Se nt < top(S): pop fino a nt, genera DEDENT per ogni pop
     * 
     * Richiede che nt sia sempre presente nello stack (indentazione consistente)
     */
    void handleIndentation();
    
    /**
     * Alla fine del file, genera DEDENT per ogni livello > 0 rimasto nello stack
     */
    void addDedentTokens();
    
public:
    /**
     * Costruttore: inizializza lo stack di indentazione con 0
     * come richiesto dalla specifica
     */
    Lexer(const std::string& sourceCode);
    ~Lexer();
    
    /**
     * Metodo principale: analizza tutto il codice sorgente e restituisce
     * la sequenza completa di token. Si ferma al primo errore lessicale.
     */
    std::vector<Token> tokenize();
    
    /** Utility per debug: stampa tutti i token generati */
    void printTokens() const;
};

#endif // LEXER_H