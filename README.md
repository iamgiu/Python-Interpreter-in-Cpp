# Python Interpreter in C++

Interprete per un sottoinsieme del linguaggio Python sviluppato in C++ come progetto per il corso di Informatica e Computazione.

## Descrizione

Questo progetto implementa un interprete completo per un sottoinsieme semplificato di Python che include:

- **Tipi di dato**: interi, booleani e liste dinamiche
- **Operatori**: aritmetici (+, -, *, //), relazionali (<, <=, >, >=, ==, !=), booleani (and, or, not)  
- **Strutture di controllo**: if/elif/else, while, break, continue
- **Gestione liste**: creazione (`list()`), accesso (`lista[indice]`), modifica, append
- **Input/Output**: istruzione `print()`
- **Gestione indentazione**: seguendo le specifiche Python

## Architettura

Il progetto è strutturato in tre fasi principali:

### 1. Analisi Lessicale (Lexer)
- **File**: `lexer.h`, `lexer.cpp`
- Converte il codice sorgente in una sequenza di token
- Gestisce l'indentazione con algoritmo a stack per generare token INDENT/DEDENT
- Riconosce numeri, identificatori, parole chiave e operatori

### 2. Analisi Sintattica (Parser)  
- **File**: `parser.h`, `parser.cpp`
- Costruisce l'Abstract Syntax Tree (AST) seguendo la grammatica BNF specificata
- Gestisce precedenza e associatività degli operatori
- Implementa parsing ricorsivo discendente

### 3. Interpretazione (Interpreter)
- **File**: `interpreter.h`, `interpreter.cpp`
- Esegue il programma attraversando l'AST con pattern Visitor
- Gestisce ambiente delle variabili e controllo di flusso
- Implementa semantica short-circuit per operatori booleani

### 4. Strutture Dati
- **File**: `ast.h`, `ast.cpp`
- Definisce la gerarchia di nodi dell'AST
- Implementa pattern Visitor per attraversamento

## Compilazione

Il progetto è compatibile con C++20 e utilizza solo librerie standard. Per compilare:

```bash
g++ -std=c++20 *.cpp -o interpreter
```

## Utilizzo

```bash
./interpreter programma.txt
```

Dove `programma.txt` è un file contenente codice Python valido secondo le specifiche supportate.

## Esempio di Programma Supportato

```python
x = 42
lista = list()
lista.append(10)
lista.append(20)

if x > 0:
    print(x)
    print(lista[0])

while x > 35:
    x = x - 1
    if x == 40:
        break

print(x)
```

## Gestione Errori

L'interprete gestisce e segnala diversi tipi di errori:

- **Errori lessicali**: caratteri non riconosciuti, indentazione inconsistente
- **Errori sintattici**: violazioni della grammatica
- **Errori di runtime**: divisione per zero, accesso fuori bounds, tipi incompatibili

Tutti gli errori vengono segnalati nel formato:
```
Error: <descrizione errore>
```

## Specifiche Tecniche

- **Linguaggio**: C++20
- **Librerie**: Solo standard library C++
- **Tipi supportati**: int64_t, bool, vector<Value>
- **Indentazione**: Gestita tramite stack seguendo specifiche Python
- **Scope variabili**: Globale unico
- **Tipizzazione**: Dinamica

## File del Progetto

- `main.cpp` - Entry point e coordinamento fasi
- `lexer.h/.cpp` - Analizzatore lessicale  
- `parser.h/.cpp` - Analizzatore sintattico
- `interpreter.h/.cpp` - Motore di esecuzione
- `ast.h/.cpp` - Strutture dati AST
- `test_program.txt` - Programma di esempio