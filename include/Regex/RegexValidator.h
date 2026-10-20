#ifndef AUTOMATAVIZ_REGEXVALIDATOR_H
#define AUTOMATAVIZ_REGEXVALIDATOR_H

#include <string>

namespace AutomataViz {

struct ValidationResult {
    bool isValid;
    std::string errorMessage;
    int errorPosition;

    ValidationResult(bool valid = true, const std::string& msg = "", int pos = -1)
        : isValid(valid), errorMessage(msg), errorPosition(pos) {}
};

class RegexValidator {
public:
    static ValidationResult validate(const std::string& regex);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_REGEXVALIDATOR_H
