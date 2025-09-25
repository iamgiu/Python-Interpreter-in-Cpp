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

/**
 * AST node for numbers (42, 100, ...)
 * 
 * Inherits from Expression by polymorphism 
 * 
 * Constructor sets the integer value and dataType to INTEGER 
 */
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

/**
 * AST node for boolean (True, False)
 * 
 * Inherits from Expression by polymorphism
 * 
 * Constructor sets the boolean value and dataType to BOOLEAN
 */
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

/**
 * AST node for the variable name (x, Var, ...)
 * 
 * Inherits from Expression by polymorphism
 * 
 * Constructor that takes the name by const reference
 */
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n) : name(n) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return name;
    }
};

/**
 * AST node to access the list (x[5], arr[i], ...)
 * 
 * Inherits from Expression by polymorphism
 * 
 * Stores list name as string and index expression as smart pointer
 */
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

/**
* AST node for unary operations (-x, not x)
*
* Inherits from Expression by polymorphism
*
* Stores the operator type and operand
*/
class UnaryOperation : public Expression {
public:
    enum class Operator {
        MINUS,   
        NOT       
    };
    
    Operator op;
    std::unique_ptr<Expression> operand;
    
    UnaryOperation(Operator operation, std::unique_ptr<Expression> expr)
        : op(operation), operand(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override;
};

/**
 * AST node for binary operations (x + y, x == y, x and y, ...)
 * 
 * Inherits from Expression by polymorphism
 * 
 * Stores left operand, operator and right operand
 * 
 * +, -, *, //, <, >, >=, <=, ==, !?, and, or
 */
class BinaryOperation : public Expression {
public:
    enum class Operator {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        LESS,          
        LESS_EQUAL,     
        GREATER,        
        GREATER_EQUAL,  
        EQUAL,          
        NOT_EQUAL,      
        AND,    
        OR       
    };
    
    std::unique_ptr<Expression> left;
    Operator op;
    std::unique_ptr<Expression> right;
    
    BinaryOperation(std::unique_ptr<Expression> l, Operator operation, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(operation), right(std::move(r)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override;
};

/**
 * AST node representing a block of statements
 * 
 * Contains a vector of unique_ptr Statement
 */
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

/**
 * AST node for assignment (x = expr, var = 5, ...)
 * 
 * Stores variable name and value expression
 */
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

/**
 * AST node for list assignment (x[i] = expr, arr[1] = 30, ...)
 * 
 * Stores list name, index expression and value expression
 */
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

/**
 * AST node for list creation (x = list())
 * 
 * Stores the variable name
 */
class ListCreation : public Statement {
public:
    std::string variableName;
    
    ListCreation(const std::string& name) : variableName(name) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return variableName + " = list()";
    }
};

/**
 * AST node for appending to a list (x.append(10))
 * 
 * Stores list name and value expression to append
 */
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

/**
 * AST node for printing (print(x), print (10))
 * 
 * Stores the expression to print
 */
class PrintStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    
    PrintStatement(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "print(" + expression->toString() + ")";
    }
};

/**
 * AST node for break statement
 * 
 * Break in python allows you to exit a loop when an external condition is met
 */
class BreakStatement : public Statement {
public:
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "break";
    }
};

/**
 * AST node for continue statement
 * 
 * Continue in python used to end the current interation in a for loop (or while loop) and continues to the next interation
 */
class ContinueStatement : public Statement {
public:
    void accept(ASTVisitor& visitor) override;
    std::string toString() const override {
        return "continue";
    }
};

/**
 * AST node for if statement with optional elif and else clauses
 * 
 * If, elif, else are condizional statements used in python that help you to automatically excute different code based on a particular condition
 */
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
    std::unique_ptr<Block> elseBlock; 
    
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

/**
 * AST node fot while statement
 * 
 * The while loop in python can execute a set of statements as long as a condition is true
 */
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

/**
 * Root of the AST it is represents the whole program
 */
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

/**
 * Visitor pattern interface defing visit methods for each node type
 */
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