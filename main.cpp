// Modifica main.cpp per aggiungere debug output

#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Cannot open file " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Debug: stampa il contenuto letto
    std::cerr << "DEBUG: File content length: " << content.length() << std::endl;
    std::cerr << "DEBUG: File content: [" << content << "]" << std::endl;
    
    // Normalizza i terminatori di linea
    std::string normalized;
    for (size_t i = 0; i < content.length(); i++) {
        if (content[i] == '\r') {
            if (i + 1 < content.length() && content[i + 1] == '\n') {
                // \r\n -> \n
                normalized += '\n';
                i++; // salta il \n
            } else {
                // \r da solo -> \n
                normalized += '\n';
            }
        } else {
            normalized += content[i];
        }
    }
    
    std::cerr << "DEBUG: Normalized content: [" << normalized << "]" << std::endl;
    return normalized;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    
    try {
        std::cerr << "DEBUG: Starting program..." << std::endl;
        
        // Read source file
        std::cerr << "DEBUG: Reading file: " << argv[1] << std::endl;
        std::string sourceCode = readFile(argv[1]);
        
        // Phase 1: Lexical Analysis
        std::cerr << "DEBUG: Starting lexical analysis..." << std::endl;
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();
        std::cerr << "DEBUG: Lexer completed. Generated " << tokens.size() << " tokens" << std::endl;
        
        // Debug: stampa i primi 10 token
        for (size_t i = 0; i < std::min(tokens.size(), size_t(10)); i++) {
            std::cerr << "DEBUG: Token " << i << ": " << static_cast<int>(tokens[i].type) 
                      << " [" << tokens[i].value << "]" << std::endl;
        }
        
        // Check for lexical errors
        for (const auto& token : tokens) {
            if (token.type == TokenType::ERROR) {
                throw std::runtime_error("Error: " + token.value);
            }
        }
        
        // Phase 2: Syntactic Analysis
        std::cerr << "DEBUG: Starting parsing..." << std::endl;
        Parser parser(tokens);
        auto program = parser.parseProgram();
        std::cerr << "DEBUG: Parsing completed successfully" << std::endl;
        
        // Phase 3: Execution
        std::cerr << "DEBUG: Starting execution..." << std::endl;
        Interpreter interpreter;
        interpreter.execute(*program);
        std::cerr << "DEBUG: Execution completed successfully" << std::endl;
        
    } catch (const ParseError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const RuntimeError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "DEBUG: Caught exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cerr << "DEBUG: Program finished normally" << std::endl;
    return 0;
}