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
    
    return normalized;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    
    try {
        // Read source file
        std::string sourceCode = readFile(argv[1]);
        
        // Phase 1: Lexical Analysis
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();
        
        // CORREZIONE: Controlla errori lessicali IMMEDIATAMENTE
        for (const auto& token : tokens) {
            if (token.type == TokenType::ERROR) {
                std::cerr << "Error: " << token.value << std::endl;
                return 1; // Ferma al primo errore lessicale
            }
        }
        
        // Phase 2: Syntactic Analysis
        Parser parser(tokens);
        auto program = parser.parseProgram();
        
        // Phase 3: Execution
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