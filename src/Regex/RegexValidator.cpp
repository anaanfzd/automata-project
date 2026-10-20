#include "Regex/RegexValidator.h"
#include <stack>
#include <cctype>

namespace AutomataViz {

ValidationResult RegexValidator::validate(const std::string& rawRegex) {
    // Filter out whitespace for validation
    std::string regex;
    for (char c : rawRegex) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            regex += c;
        }
    }

    if (regex.empty()) {
        return ValidationResult(false, "Regular expression cannot be empty.", 0);
    }

    std::stack<int> parenIndices;

    for (size_t i = 0; i < regex.size(); ++i) {
        char c = regex[i];

        // Check parentheses
        if (c == '(') {
            parenIndices.push(static_cast<int>(i));
            if (i + 1 < regex.size() && regex[i + 1] == ')') {
                return ValidationResult(false, "Empty parentheses '()' are not allowed.", static_cast<int>(i));
            }
        } else if (c == ')') {
            if (parenIndices.empty()) {
                return ValidationResult(false, "Unmatched closing parenthesis ')' without opening '('.", static_cast<int>(i));
            }
            parenIndices.pop();
        }

        // Check binary operator '|'
        if (c == '|') {
            if (i == 0) {
                return ValidationResult(false, "Union operator '|' cannot appear at the start of an expression.", 0);
            }
            if (i == regex.size() - 1) {
                return ValidationResult(false, "Union operator '|' cannot appear at the end of an expression.", static_cast<int>(i));
            }
            char prev = regex[i - 1];
            char next = regex[i + 1];
            if (prev == '(' || prev == '|') {
                return ValidationResult(false, "Union operator '|' has missing left operand.", static_cast<int>(i));
            }
            if (next == ')' || next == '|' || next == '*' || next == '+' || next == '?') {
                return ValidationResult(false, "Union operator '|' has invalid right operand.", static_cast<int>(i));
            }
        }

        // Check unary operators '*', '+', '?'
        if (c == '*' || c == '+' || c == '?') {
            if (i == 0) {
                return ValidationResult(false, std::string("Unary operator '") + c + "' requires a preceding operand.", 0);
            }
            char prev = regex[i - 1];
            if (prev == '(' || prev == '|') {
                return ValidationResult(false, std::string("Unary operator '") + c + "' cannot directly follow '" + prev + "'.", static_cast<int>(i));
            }
        }
    }

    if (!parenIndices.empty()) {
        return ValidationResult(false, "Unclosed opening parenthesis '(' missing corresponding ')'.", parenIndices.top());
    }

    return ValidationResult(true, "Valid Regular Expression ✓", -1);
}

} // namespace AutomataViz
