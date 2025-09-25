/**
 * Guard Headers
 * 
 * Preprocess directives used in C and C++ to prevent the contents of a header file from being included more than once in a single compilation unit
 */
#ifndef AST_H
#define AST_H

/**
 * Include needed for AST implementation
 * 
 * Vector: for dynamic containers
 * 
 * Memory: for smart pointers (unique_ptr)
 * 
 * String: to handle strings
 */
#include <vector>
#include <memory>
#include <string>

/**
 * Advance Declaration so that in case I can use it before having declared it 
 */
class ASTVisitor;

/**
 * Enum class which defines the data types
 */
enum class DataType {
    INTEGER,
    BOOLEAN,
    LIST,
    UNDEFINED
};

/**
 * Defines the base class for all AST nodes:
 * 
 * ~ASTNode(): Virtual destructor for polymorphism
 * 
 * accept(ASTVisitor& visitor): each node must accept a visitor for operations
 * 
 *  std::string toString(): each node must be able to be converted to a string
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual std::string toString() const = 0;
};

/**
 * Stores the data type of the expression
 */
class Expression : public ASTNode {
public:
    DataType dataType = DataType::UNDEFINED;
};

/**
 * Base class for instructions
 * 
 * It is only used to categorize instructions
 */
class Statement : public ASTNode {
};

// Letterale numerico: 42
class NumberLiteral : public Expression {
public:
    int value;
    
    NumberLiteral(int val) : value(val) {
        dataType = DataType::INTEGER;
    }
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return std::to_string(value);
    }
};

// Letterale booleano: True, False
class BooleanLiteral : public Expression {
public:
    bool value;
    
    BooleanLiteral(bool val) : value(val) {
        dataType = DataType::BOOLEAN;
    }
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return value ? "True" : "False";
    }
};

// Identificatore: x
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n) : name(n) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return name;
    }
};

// Accesso a lista: x[i]
class ListAccess : public Expression {
public:
    std::string listName;
    std::unique_ptr<Expression> index;
    
    ListAccess(const std::string& name, std::unique_ptr<Expression> idx) 
        : listName(name), index(std::move(idx)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return listName + "[" + index->toString() + "]";
    }
};

// Operazioni unarie: -x, not x
class UnaryOperation : public Expression {
public:
    enum class Operator {
        MINUS,    // -
        NOT       // not
    };
    
    Operator op;
    std::unique_ptr<Expression> operand;
    
    UnaryOperation(Operator operation, std::unique_ptr<Expression> expr)
        : op(operation), operand(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override;
};

// Operazioni binarie: x + y, x == y, x and y, etc.
class BinaryOperation : public Expression {
public:
    enum class Operator {
        // Aritmetici
        ADD,      // +
        SUBTRACT, // -
        MULTIPLY, // *
        DIVIDE,   // //
        // Relazionali
        LESS,           // <
        LESS_EQUAL,     // <=
        GREATER,        // >
        GREATER_EQUAL,  // >=
        EQUAL,          // ==
        NOT_EQUAL,      // !=
        // Booleani
        AND,      // and
        OR        // or
    };
    
    std::unique_ptr<Expression> left;
    Operator op;
    std::unique_ptr<Expression> right;
    
    BinaryOperation(std::unique_ptr<Expression> l, Operator operation, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(operation), right(std::move(r)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override;
};

// ========== ISTRUZIONI ==========

// Blocco di istruzioni
class Block : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void addStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "Block(" + std::to_string(statements.size()) + " statements)";
    }
};

// Assegnazione: x = expr
class Assignment : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> value;
    
    Assignment(const std::string& name, std::unique_ptr<Expression> val)
        : variableName(name), value(std::move(val)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return variableName + " = " + value->toString();
    }
};

// Assegnazione a lista: x[i] = expr
class ListAssignment : public Statement {
public:
    std::string listName;
    std::unique_ptr<Expression> index;
    std::unique_ptr<Expression> value;
    
    ListAssignment(const std::string& name, std::unique_ptr<Expression> idx, std::unique_ptr<Expression> val)
        : listName(name), index(std::move(idx)), value(std::move(val)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return listName + "[" + index->toString() + "] = " + value->toString();
    }
};

// Creazione lista: x = list()
class ListCreation : public Statement {
public:
    std::string variableName;
    
    ListCreation(const std::string& name) : variableName(name) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return variableName + " = list()";
    }
};

// Append a lista: x.append(expr)
class ListAppend : public Statement {
public:
    std::string listName;
    std::unique_ptr<Expression> value;
    
    ListAppend(const std::string& name, std::unique_ptr<Expression> val)
        : listName(name), value(std::move(val)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return listName + ".append(" + value->toString() + ")";
    }
};

// Print: print(expr)
class PrintStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    
    PrintStatement(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "print(" + expression->toString() + ")";
    }
};

// Break
class BreakStatement : public Statement {
public:
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "break";
    }
};

// Continue
class ContinueStatement : public Statement {
public:
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "continue";
    }
};

// If statement con elif e else opzionali
class IfStatement : public Statement {
public:
    struct ElifClause {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Block> body;
        
        ElifClause(std::unique_ptr<Expression> cond, std::unique_ptr<Block> b)
            : condition(std::move(cond)), body(std::move(b)) {}
    };
    
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenBlock;
    std::vector<ElifClause> elifClauses;
    std::unique_ptr<Block> elseBlock;  // può essere nullptr
    
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> then)
        : condition(std::move(cond)), thenBlock(std::move(then)), elseBlock(nullptr) {}
    
    void addElif(std::unique_ptr<Expression> cond, std::unique_ptr<Block> body) {
        elifClauses.emplace_back(std::move(cond), std::move(body));
    }
    
    void setElse(std::unique_ptr<Block> elseBody) {
        elseBlock = std::move(elseBody);
    }
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override;
};

// While statement
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
    
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "while " + condition->toString() + ": ...";
    }
};

// Programma (root del AST)
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void addStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "Program(" + std::to_string(statements.size()) + " statements)";
    }
};

// ========== PATTERN VISITOR ==========

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Espressioni
    virtual void visit(NumberLiteral& node) = 0;
    virtual void visit(BooleanLiteral& node) = 0;
    virtual void visit(Identifier& node) = 0;
    virtual void visit(ListAccess& node) = 0;
    virtual void visit(UnaryOperation& node) = 0;
    virtual void visit(BinaryOperation& node) = 0;
    
    // Istruzioni
    virtual void visit(Assignment& node) = 0;
    virtual void visit(ListAssignment& node) = 0;
    virtual void visit(ListCreation& node) = 0;
    virtual void visit(ListAppend& node) = 0;
    virtual void visit(PrintStatement& node) = 0;
    virtual void visit(BreakStatement& node) = 0;
    virtual void visit(ContinueStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(Block& node) = 0;
    virtual void visit(Program& node) = 0;
};

#endif // AST_H