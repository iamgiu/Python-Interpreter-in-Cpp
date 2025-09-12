#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Cannot open file " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    // Controlla argomenti da linea di comando
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    
    try {
        // Leggi il file sorgente
        std::string sourceCode = readFile(argv[1]);
        
        // Crea il lexer e tokenizza
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();
        
        // Per ora stampa i token (per debug)
        std::cout << "=== TOKENS ===" << std::endl;
        lexer.printTokens();
        
        std::cout << "\nTokenization completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}