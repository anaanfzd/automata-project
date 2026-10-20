#include "Regex/RegexParser.h"
#include <stack>
#include <sstream>
#include <cctype>

namespace AutomataViz {

std::vector<Token> RegexParser::tokenize(const std::string& regex) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < regex.size(); ++i) {
        char c = regex[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue;
        }

        switch (c) {
            case '(':
                tokens.emplace_back(TokenType::LPAREN, c, static_cast<int>(i));
                break;
            case ')':
                tokens.emplace_back(TokenType::RPAREN, c, static_cast<int>(i));
                break;
            case '|':
                tokens.emplace_back(TokenType::UNION, c, static_cast<int>(i));
                break;
            case '*':
                tokens.emplace_back(TokenType::STAR, c, static_cast<int>(i));
                break;
            case '+':
                tokens.emplace_back(TokenType::PLUS, c, static_cast<int>(i));
                break;
            case '?':
                tokens.emplace_back(TokenType::QUESTION, c, static_cast<int>(i));
                break;
            case '.':
                // Explicit dot concatenation
                tokens.emplace_back(TokenType::CONCAT, c, static_cast<int>(i));
                break;
            case '#':
                // Explicit epsilon symbol representation
                tokens.emplace_back(TokenType::EPSILON, '#', static_cast<int>(i));
                break;
            default:
                // Check for UTF-8 epsilon character 'ε' (0xCE 0xBB)
                if (static_cast<unsigned char>(c) == 0xCE && i + 1 < regex.size() &&
                    static_cast<unsigned char>(regex[i + 1]) == 0xBB) {
                    tokens.emplace_back(TokenType::EPSILON, '#', static_cast<int>(i));
                    ++i; // skip second byte
                } else {
                    tokens.emplace_back(TokenType::SYMBOL, c, static_cast<int>(i));
                }
                break;
        }
    }
    return tokens;
}

bool RegexParser::shouldInsertConcat(const Token& prev, const Token& curr) {
    // Left token can be: SYMBOL, EPSILON, RPAREN, STAR, PLUS, QUESTION
    bool leftCanConcat = (prev.type == TokenType::SYMBOL ||
                          prev.type == TokenType::EPSILON ||
                          prev.type == TokenType::RPAREN ||
                          prev.type == TokenType::STAR ||
                          prev.type == TokenType::PLUS ||
                          prev.type == TokenType::QUESTION);

    // Right token can be: SYMBOL, EPSILON, LPAREN
    bool rightCanConcat = (curr.type == TokenType::SYMBOL ||
                           curr.type == TokenType::EPSILON ||
                           curr.type == TokenType::LPAREN);

    return leftCanConcat && rightCanConcat;
}

std::vector<Token> RegexParser::insertExplicitConcat(const std::vector<Token>& tokens) {
    std::vector<Token> result;
    if (tokens.empty()) return result;

    result.push_back(tokens[0]);
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (shouldInsertConcat(tokens[i - 1], tokens[i])) {
            result.emplace_back(TokenType::CONCAT, '.', tokens[i].position);
        }
        result.push_back(tokens[i]);
    }
    return result;
}

static std::string formatTokenList(const std::vector<Token>& list) {
    std::string s;
    for (const auto& t : list) {
        s += t.toString();
    }
    return s;
}

static std::string formatStack(const std::stack<Token>& st) {
    std::stack<Token> temp = st;
    std::vector<std::string> items;
    while (!temp.empty()) {
        items.push_back(temp.top().toString());
        temp.pop();
    }
    std::string s = "[";
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        s += items[i];
        if (i > 0) s += ", ";
    }
    s += "]";
    return s;
}

ParseResult RegexParser::parse(const std::string& inputRegex) {
    ParseResult result;
    ValidationResult val = RegexValidator::validate(inputRegex);
    if (!val.isValid) {
        result.success = false;
        result.errorMessage = val.errorMessage;
        return result;
    }

    result.rawTokens = tokenize(inputRegex);
    result.explicitTokens = insertExplicitConcat(result.rawTokens);

    for (const auto& t : result.explicitTokens) {
        if (t.type == TokenType::SYMBOL) {
            result.alphabet.insert(t.symbol);
        }
    }

    // Shunting-Yard Algorithm for Infix to Postfix conversion
    std::stack<Token> opStack;
    int stepNum = 1;

    for (const auto& token : result.explicitTokens) {
        ParseStepLog step;
        step.stepNumber = stepNum++;
        step.tokenDescription = "Token: '" + token.toString() + "'";

        if (token.type == TokenType::SYMBOL || token.type == TokenType::EPSILON) {
            result.postfixTokens.push_back(token);
            step.actionExplanation = "Operand '" + token.toString() + "' appended directly to postfix output.";
        } else if (token.type == TokenType::LPAREN) {
            opStack.push(token);
            step.actionExplanation = "Left parenthesis '(' pushed to operator stack to set subexpression scope.";
        } else if (token.type == TokenType::RPAREN) {
            while (!opStack.empty() && opStack.top().type != TokenType::LPAREN) {
                result.postfixTokens.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) {
                opStack.pop(); // Pop LPAREN
            }
            step.actionExplanation = "Right parenthesis ')': Popped operators inside parentheses to postfix output.";
        } else if (token.isUnary()) {
            // Postfix unary operator binds directly to the preceding atom
            result.postfixTokens.push_back(token);
            step.actionExplanation = "Unary operator '" + token.toString() + "' (precedence " +
                                     std::to_string(token.precedence()) + ") appended to postfix output.";
        } else if (token.isBinary()) {
            while (!opStack.empty() && opStack.top().type != TokenType::LPAREN &&
                   opStack.top().precedence() >= token.precedence()) {
                result.postfixTokens.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
            step.actionExplanation = "Binary operator '" + token.toString() + "' (precedence " +
                                     std::to_string(token.precedence()) + "): Popped higher/equal precedence operators, pushed to stack.";
        }

        step.operatorStackSnapshot = formatStack(opStack);
        step.postfixOutputSnapshot = formatTokenList(result.postfixTokens);
        result.stepLogs.push_back(step);
    }

    while (!opStack.empty()) {
        ParseStepLog step;
        step.stepNumber = stepNum++;
        step.tokenDescription = "End of input stream";
        step.actionExplanation = "Popping remaining operator '" + opStack.top().toString() + "' from stack to output.";
        result.postfixTokens.push_back(opStack.top());
        opStack.pop();
        step.operatorStackSnapshot = formatStack(opStack);
        step.postfixOutputSnapshot = formatTokenList(result.postfixTokens);
        result.stepLogs.push_back(step);
    }

    result.postfixString = formatTokenList(result.postfixTokens);
    result.success = true;
    return result;
}

} // namespace AutomataViz
