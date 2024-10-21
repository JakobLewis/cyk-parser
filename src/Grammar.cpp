#include "Grammar.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

bool ContextFreeGrammar::try_load(std::string path) {
  std::ifstream grammar_file(path);

  if (!grammar_file.is_open()) {
    std::cerr << "Could not read in file '" << path << "', aborting"
              << std::endl;
    exit(1);
  }

  std::string line;
  size_t line_count = 0;
  while (std::getline(grammar_file, line)) {
    line_count++;

    // Using " -> " to split string into LHS & RHS
    size_t lhs_symbol_end = line.find(" -> ");
    if (lhs_symbol_end == std::string::npos) {
      std::cerr << "Could not find ' -> ' symbol on line " << line_count
                << " while parsing file '" << path << "'" << std::endl;
      return false;
    }

    std::string lhs_symbol = line.substr(0, lhs_symbol_end);
    line.erase(0, lhs_symbol_end + 4);

    // RHS can have multiple symbols, so we need to parse them out
    std::vector<std::string> rhs_symbols{};
    size_t rhs_symbol_end = line.find(" ");
    while (rhs_symbol_end != std::string::npos) {
      rhs_symbols.push_back(line.substr(0, rhs_symbol_end));
      line.erase(0, rhs_symbol_end + 1);
      rhs_symbol_end = line.find(" ");
    }

    if ((rhs_symbols.size() == 0) && (line == "")) {
      std::cerr << "Could not find RHS symbols on line " << line_count
                << " while parsing file '" << path << "'" << std::endl;
      return false;
    }

    rhs_symbols.push_back(line);

    // Make sure all terminal symbols are enclosed in two "'" marks
    for (size_t i = 0; i < rhs_symbols.size(); i++) {
      std::string s = rhs_symbols[i];
      if (s[0] != '\'')
        continue;

      if (s.find('\'', 1) == std::string::npos) {
        std::cerr << "Invalid terminal string on line " << line_count
                  << " while parsing file '" << path << "'" << std::endl;
        return false;
      }
    }

    // Add rule to rules map
    this->rules.insert(std::make_pair(lhs_symbol, rhs_symbols));

    // TODO purge duplicates

#ifdef DEBUG
    std::cout << lhs_symbol << " -> " << std::endl;
    for (size_t i = 0; i < rhs_symbols.size(); i++) {
      std::cout << "  | " << rhs_symbols[i] << std::endl;
    }
#endif
  }

  return true;
}

bool ChomskyNormalForm::try_from_CFG(const ContextFreeGrammar &grammar) {
  /** Note: a lot of the operations over the ruleset in this function are...
   * clunky. This is because I don't believe I can add or remove rules while
   * iterating through the ruleset, which is quite frustrating. Instead I
   * perform updates in-place when I can and create a temporary pile of new
   * rules to apply after.
   */

  Grammar_Rules_t copy{grammar.rules};

  /**
   * Replace start symbol with new start symbol
   * This ensures that the start symbol never occurs on the RHS. This is a bit
   * of a silly way to go about it, since it's quite possible that the start
   * symbol only occurs on the LHS already. There should be some way to detect
   * when a CFG has no start symbol. I can't implement it today since this
   * assignment is already late :).
   */
  if (rules.find("S0") == rules.end()) {
    rules.insert(
        std::make_pair(std::string{"S0"}, std::vector{std::string{"S"}}));
  }

  /** Here we separate out terminal symbols on the RHS into their own unit
   * rules. I am not entirely sure why this is done.
   *
   * We might also remove so-called "ε" rules here, however since I don't plan
   * on introducing any ε symbols into my grammar I believe I can skip this
   * step.
   */

  std::map<std::string, std::string> new_unit_terminals{};
  // Replace all RHS terminal-symbol occurences with new stand-ins
  for (auto it = copy.begin(); it != copy.end(); it++) {
    std::vector<std::string> rhs = it->second;

    if (rhs.size() == 1) {
      continue; // Rule is already in form A -> B
    }

    // Scan through for terminal symbols
    for (size_t i = 0; i < rhs.size(); i++) {
      std::string terminal_value = rhs[i];

      if (terminal_value[0] != '\'')
        continue;

      /** If we haven't already created a new unit rule for this terminal,
       * create said rule.
       *
       * TODO account for pre-existing terminal unit rules
       */
      if (new_unit_terminals.find(terminal_value) == new_unit_terminals.end()) {
        std::string terminal_name{};
        do {
          terminal_name = "C" + std::to_string(constant_count++);
        } while ((rules.find(terminal_name) != rules.end()) &&
                 (copy.find(terminal_name) != copy.end()));

        new_unit_terminals.insert(
            std::make_pair(terminal_value, terminal_name));
      }

      // Replace terminal symbol with unit rule
      it->second[i] = new_unit_terminals.find(terminal_value)->second;
    }
  }

  // Add newly generated rules to ruleset
  for (auto it = new_unit_terminals.begin(); it != new_unit_terminals.end();
       it++) {
    copy.insert(std::make_pair(std::string{it->second},
                               std::vector{std::string{it->first}}));
  }

  /**
   * Here we begin removing keys from our copy to place them into this object's
   * existing "completed" ruleset. In this stage we split rules with more than 2
   * items on their RHS into smaller sub-rules, allowing for binary operatons on
   * all grammar rules.
   */
  while (copy.begin() != copy.end()) {
    auto pair = copy.extract(copy.begin());
    std::string lhs{pair.key()};
    std::vector<std::string> rhs{pair.mapped()};

    // Pass rules with a RHS of two or less in immediately
    if (rhs.size() < 3) {
      rules.insert(std::pair(lhs, rhs));
      continue;
    }

    auto combined_rule_name = [](auto &first_rule_name,
                                 auto &second_rule_name) {
      std::string combigned_name{first_rule_name};
      combigned_name += "_" + second_rule_name;
      return combigned_name;
    };

    /** Scan through to see if any pairs of symbols have already been combined
     * into a new rule during an earlier loop iteration. If they have we swap
     * them out with their new symbol in-place and continue.
     */
    for (size_t i = 0; i < rhs.size() - 1; i++) {

      auto existing_replacement =
          this->rules.find(combined_rule_name(rhs[i], rhs[i + 1]));

      if (existing_replacement != this->rules.end()) {
        rhs.erase(rhs.begin() + i);
        rhs[i] = existing_replacement->first;
      }
    }

    /**
     * Create new composite rules until our current rule fits the form expected
     * in CNF.
     */
    while (rhs.size() > 2) {
      std::string first = rhs[rhs.size() - 1];
      std::string second = rhs[rhs.size() - 2];
      rhs.pop_back();
      rhs.pop_back();
      std::string combigned_name = combined_rule_name(first, second);

      this->rules.insert(std::pair(combigned_name, std::vector{first, second}));
      rhs.push_back(combigned_name);
    }
  }

  return true;
}