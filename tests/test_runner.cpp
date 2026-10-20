#include "Regex/RegexValidator.h"
#include "Regex/RegexParser.h"
#include "Algorithms/ThompsonConstruction.h"
#include "Algorithms/SubsetConstruction.h"
#include "Algorithms/DFAMinimization.h"
#include "Algorithms/StringSimulator.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace AutomataViz;

static int totalTests = 0;
static int passedTests = 0;

void testAssert(bool condition, const std::string& testName) {
    totalTests++;
    if (condition) {
        passedTests++;
        std::cout << "  [PASS] " << testName << std::endl;
    } else {
        std::cerr << "  [FAIL] " << testName << std::endl;
    }
}

void runValidationTests() {
    std::cout << "\n=== 1. Regex Validation Tests ===" << std::endl;

    // Valid expressions
    testAssert(RegexValidator::validate("a").isValid, "Valid single symbol 'a'");
    testAssert(RegexValidator::validate("a|b").isValid, "Valid union 'a|b'");
    testAssert(RegexValidator::validate("ab").isValid, "Valid concatenation 'ab'");
    testAssert(RegexValidator::validate("a*").isValid, "Valid Kleene star 'a*'");
    testAssert(RegexValidator::validate("(a|b)*").isValid, "Valid parenthesized star '(a|b)*'");
    testAssert(RegexValidator::validate("(a|b)*abb").isValid, "Valid complex regex '(a|b)*abb'");
    testAssert(RegexValidator::validate("((a|b)*c)+").isValid, "Valid nested parens '((a|b)*c)+'");

    // Invalid expressions
    testAssert(!RegexValidator::validate("").isValid, "Reject empty string");
    testAssert(!RegexValidator::validate("(a|b").isValid, "Reject unmatched opening '('");
    testAssert(!RegexValidator::validate("a|b)").isValid, "Reject unmatched closing ')'");
    testAssert(!RegexValidator::validate("()").isValid, "Reject empty parentheses '()'");
    testAssert(!RegexValidator::validate("|a").isValid, "Reject leading union '|a'");
    testAssert(!RegexValidator::validate("a|").isValid, "Reject trailing union 'a|'");
    testAssert(!RegexValidator::validate("a||b").isValid, "Reject consecutive union 'a||b'");
    testAssert(!RegexValidator::validate("*a").isValid, "Reject leading Kleene star '*a'");
    testAssert(!RegexValidator::validate("(|a)").isValid, "Reject subexpression starting with '|'");
}

void runParserTests() {
    std::cout << "\n=== 2. Tokenization & Postfix Conversion Tests ===" << std::endl;

    auto r1 = RegexParser::parse("ab");
    testAssert(r1.success && r1.postfixString == "ab.", "Infix 'ab' -> Postfix 'ab.'");

    auto r2 = RegexParser::parse("a|b");
    testAssert(r2.success && r2.postfixString == "ab|", "Infix 'a|b' -> Postfix 'ab|'");

    auto r3 = RegexParser::parse("a*");
    testAssert(r3.success && r3.postfixString == "a*", "Infix 'a*' -> Postfix 'a*'");

    auto r4 = RegexParser::parse("(a|b)*");
    testAssert(r4.success && r4.postfixString == "ab|*", "Infix '(a|b)*' -> Postfix 'ab|*'");

    auto r5 = RegexParser::parse("(a|b)*abb");
    testAssert(r5.success && r5.postfixString == "ab|*a.b.b.", "Infix '(a|b)*abb' -> Postfix 'ab|*a.b.b.'");
}

void runThompsonConstructionTests() {
    std::cout << "\n=== 3. Thompson Construction Tests ===" << std::endl;

    std::vector<std::string> regexes = {"a", "a|b", "ab", "a*", "(a|b)*", "(a|b)*abb"};

    for (const auto& re : regexes) {
        auto parsed = RegexParser::parse(re);
        assert(parsed.success);

        auto thomp = ThompsonConstruction::build(parsed.postfixTokens);
        testAssert(thomp.success, "Thompson NFA generated for '" + re + "'");
        testAssert(thomp.finalNFA.getStartState() != -1, "Valid start state for '" + re + "'");
        testAssert(!thomp.finalNFA.getFinalStates().empty(), "Non-empty final states for '" + re + "'");
        testAssert(thomp.finalNFA.stateCount() >= 2, "State count >= 2 for '" + re + "'");
    }
}

void runSubsetAndMinimizationTests() {
    std::cout << "\n=== 4. Subset Construction & DFA Minimization Tests ===" << std::endl;

    std::string re = "(a|b)*abb";
    auto parsed = RegexParser::parse(re);
    auto thomp = ThompsonConstruction::build(parsed.postfixTokens);
    auto subset = SubsetConstruction::convert(thomp.finalNFA);

    testAssert(subset.success, "Subset construction succeeded for '(a|b)*abb'");
    testAssert(subset.finalDFA.stateCount() > 0, "DFA has states");
    testAssert(subset.finalDFA.getStartState() != -1, "DFA has start state");
    testAssert(!subset.finalDFA.getFinalStates().empty(), "DFA has accepting states");

    auto min = DFAMinimization::minimize(subset.finalDFA);
    testAssert(min.success, "DFA Minimization succeeded for '(a|b)*abb'");
    testAssert(min.minimizedDFA.stateCount() <= subset.finalDFA.stateCount(), "Minimized states <= Original states");
    testAssert(min.minimizedDFA.stateCount() == 4, "Canonical minimized state count for '(a|b)*abb' is 4");
}

void runStringSimulationTests() {
    std::cout << "\n=== 5. String Acceptance Simulation Tests ===" << std::endl;

    // Build minimized DFA for (a|b)*abb
    auto parsed = RegexParser::parse("(a|b)*abb");
    auto thomp = ThompsonConstruction::build(parsed.postfixTokens);
    auto subset = SubsetConstruction::convert(thomp.finalNFA);
    auto min = DFAMinimization::minimize(subset.finalDFA);

    DFA dfa = min.minimizedDFA;

    // Test strings from syllabus requirements
    testAssert(dfa.simulate("abb").accepted, "Accept 'abb'");
    testAssert(dfa.simulate("aabb").accepted, "Accept 'aabb'");
    testAssert(dfa.simulate("ababb").accepted, "Accept 'ababb'");
    testAssert(dfa.simulate("babb").accepted, "Accept 'babb'");
    testAssert(dfa.simulate("ababbabb").accepted, "Accept 'ababbabb'");
    testAssert(dfa.simulate("bbabb").accepted, "Accept 'bbabb'");

    testAssert(!dfa.simulate("ab").accepted, "Reject 'ab' (missing final 'b')");
    testAssert(!dfa.simulate("aaa").accepted, "Reject 'aaa'");
    testAssert(!dfa.simulate("").accepted, "Reject empty string ''");
    testAssert(!dfa.simulate("b").accepted, "Reject 'b'");
    testAssert(!dfa.simulate("abba").accepted, "Reject 'abba'");

    // Test a*
    auto pStar = RegexParser::parse("a*");
    auto tStar = ThompsonConstruction::build(pStar.postfixTokens);
    auto sStar = SubsetConstruction::convert(tStar.finalNFA);
    auto mStar = DFAMinimization::minimize(sStar.finalDFA);
    testAssert(mStar.minimizedDFA.simulate("").accepted, "a* accepts empty string ''");
    testAssert(mStar.minimizedDFA.simulate("a").accepted, "a* accepts 'a'");
    testAssert(mStar.minimizedDFA.simulate("aaaa").accepted, "a* accepts 'aaaa'");
    testAssert(!mStar.minimizedDFA.simulate("b").accepted, "a* rejects 'b'");
    testAssert(!mStar.minimizedDFA.simulate("ab").accepted, "a* rejects 'ab'");
}

int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "  AutomataViz Automated Algorithm & Verification Suite" << std::endl;
    std::cout << "========================================================" << std::endl;

    runValidationTests();
    runParserTests();
    runThompsonConstructionTests();
    runSubsetAndMinimizationTests();
    runStringSimulationTests();

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Test Results: " << passedTests << " / " << totalTests << " Passed ("
              << (passedTests * 100 / totalTests) << "%)" << std::endl;
    std::cout << "========================================================" << std::endl;

    return (passedTests == totalTests) ? 0 : 1;
}
