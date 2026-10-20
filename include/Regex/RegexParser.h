#ifndef AUTOMATAVIZ_REGEXPARSER_H
#define AUTOMATAVIZ_REGEXPARSER_H

#include "Regex/Token.h"
#include "Regex/RegexValidator.h"
#include <vector>
#include <string>
#include <set>

namespace AutomataViz {

struct ParseStepLog {
    int stepNumber;
    std::string tokenDescription;
    std::string operatorStackSnapshot;
    std::string postfixOutputSnapshot;
    std::string actionExplanation;
};

struct ParseResult {
    bool success;
    std::string errorMessage;
    std::vector<Token> rawTokens;
    std::vector<Token> explicitTokens;  // With explicit concatenation tokens inserted
    std::vector<Token> postfixTokens;
    std::string postfixString;
    std::set<char> alphabet;
    std::vector<ParseStepLog> stepLogs;
};

class RegexParser {
public:
    static ParseResult parse(const std::string& inputRegex);

private:
    static std::vector<Token> tokenize(const std::string& regex);
    static std::vector<Token> insertExplicitConcat(const std::vector<Token>& tokens);
    static bool shouldInsertConcat(const Token& prev, const Token& curr);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_REGEXPARSER_H
