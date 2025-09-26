/**
 * Include for std::count, std::cerr used for printing and error messages
 * 
 * Include for std::ifstream to read source file from disk
 * 
 * Include for std::stringstream to easily read entire file content
 */
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * Include project headers for lexer, parser and interpreter
 */
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

/**
 * Reads the entire content of a file into a string
 */
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Cannot open file " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::string normalized;
    for (size_t i = 0; i < content.length(); i++) {
        if (content[i] == '\r') {
            if (i + 1 < content.length() && content[i + 1] == '\n') {
                normalized += '\n';
                i++;
            } else {
                normalized += '\n';
            }
        } else {
            normalized += content[i];
        }
    }
    
    return normalized;
}

/**
 * Expects exactly one argument: the path to the source file to execute
 * 
 * Performs lexical analysis, parsing an interpretation
 * 
 * Reports errors
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    
    try {
        std::string sourceCode = readFile(argv[1]);

        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();

        for (const auto& token : tokens) {
            if (token.type == TokenType::ERROR) {
                std::cerr << "Error: " << token.value << std::endl;
                return 1; 
            }
        }

        Parser parser(tokens);
        auto program = parser.parseProgram();
 
        Interpreter interpreter;
        interpreter.execute(*program);
        
    } catch (const ParseError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const RuntimeError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}