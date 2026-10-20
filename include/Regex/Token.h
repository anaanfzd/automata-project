#ifndef AUTOMATAVIZ_TOKEN_H
#define AUTOMATAVIZ_TOKEN_H

#include <string>

namespace AutomataViz {

enum class TokenType {
    SYMBOL,       // Normal alphabet symbol: 'a', 'b', '0', '1', etc.
    EPSILON,      // Epsilon transition: 'ε' or '#'
    CONCAT,       // Explicit or implicit concatenation: '.'
    UNION,        // Alternation / Union: '|'
    STAR,         // Kleene star: '*' (zero or more)
    PLUS,         // Plus: '+' (one or more)
    QUESTION,     // Optional: '?' (zero or one)
    LPAREN,       // Left parenthesis: '('
    RPAREN        // Right parenthesis: ')'
};

struct Token {
    TokenType type;
    char symbol;         // Valid when type is SYMBOL or EPSILON
    int position;        // 0-based index in the original regex string

    Token(TokenType t = TokenType::SYMBOL, char s = '\0', int pos = -1)
        : type(t), symbol(s), position(pos) {}

    bool isOperator() const {
        return type == TokenType::UNION || type == TokenType::CONCAT ||
               type == TokenType::STAR || type == TokenType::PLUS ||
               type == TokenType::QUESTION;
    }

    bool isUnary() const {
        return type == TokenType::STAR || type == TokenType::PLUS || type == TokenType::QUESTION;
    }

    bool isBinary() const {
        return type == TokenType::UNION || type == TokenType::CONCAT;
    }

    int precedence() const {
        switch (type) {
            case TokenType::STAR:
            case TokenType::PLUS:
            case TokenType::QUESTION:
                return 3;
            case TokenType::CONCAT:
                return 2;
            case TokenType::UNION:
                return 1;
            default:
                return 0;
        }
    }

    std::string toString() const {
        switch (type) {
            case TokenType::SYMBOL:
                return std::string(1, symbol);
            case TokenType::EPSILON:
                return "ε";
            case TokenType::CONCAT:
                return ".";
            case TokenType::UNION:
                return "|";
            case TokenType::STAR:
                return "*";
            case TokenType::PLUS:
                return "+";
            case TokenType::QUESTION:
                return "?";
            case TokenType::LPAREN:
                return "(";
            case TokenType::RPAREN:
                return ")";
            default:
                return "?";
        }
    }
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_TOKEN_H
