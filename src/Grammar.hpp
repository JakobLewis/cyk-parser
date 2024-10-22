#pragma once
#include <map>
#include <string>
#include <vector>

/**
 * Context-free Grammar Rule type ; Maps single LHS symbol to multiple RHS
 * symbols
 *
 * Chosen as it allows nice grouping of one key to multiple values for printing.
 * A more efficient version might allow looking up values in the opposite
 * direction for generating the table in the Parsing step.
 */
typedef std::multimap<std::string, std::vector<std::string>> Grammar_Rules_t;

class ContextFreeGrammar {
public:
  Grammar_Rules_t rules;

  bool try_load(std::string file_path);
};

/** Specialised CFG form */
class ChomskyNormalForm {
public:
  Grammar_Rules_t rules;
  size_t constant_count;

  ChomskyNormalForm() : constant_count{} {}

  bool try_from_CFG(const ContextFreeGrammar &grammar);
};