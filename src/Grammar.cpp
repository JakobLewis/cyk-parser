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

  // Validate inputs
  std::string line;
  size_t line_count = 0;

  while (std::getline(grammar_file, line)) {
    line_count++;

    size_t lhs_symbol_end = line.find(" -> ");
    if (lhs_symbol_end == std::string::npos) {
      std::cerr << "Could not find ' -> ' symbol on line " << line_count
                << " while parsing file '" << path << "'" << std::endl;
      return false;
    }

    std::string lhs_symbol = line.substr(0, lhs_symbol_end);
    line.erase(0, lhs_symbol_end + 4);

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

    // TODO purge duplicates
    this->rules.insert(std::make_pair(lhs_symbol, rhs_symbols));

    // std::cout << lhs_symbol << " -> " << std::endl;
    // for (size_t i = 0; i < rhs_symbols.size(); i++) {
    //   std::cout << "  | " << rhs_symbols[i] << std::endl;
    // }
  }

  return true;
}

bool ChomskyNormalForm::try_from_CFG(const ContextFreeGrammar &grammar) {

  Grammar_Rules_t copy{grammar.rules};

  // Guarantee our start symbol never occurs on the RHS
  if (rules.find("S0") == rules.end()) {
    rules.insert(
        std::make_pair(std::string{"S0"}, std::vector{std::string{"S"}}));
  }

  std::map<std::string, std::string> new_terminals{};
  // Replace all RHS terminal-symbol occurences with new stand-ins
  for (auto it = copy.begin(); it != copy.end(); it++) {
    std::vector<std::string> rhs = it->second;

    if (rhs.size() == 1)
      continue;

    for (size_t i = 0; i < rhs.size(); i++) {
      std::string terminal_value = rhs[i];

      if (terminal_value[0] != '\'')
        continue;

      if (new_terminals.find(terminal_value) == new_terminals.end()) {
        std::string terminal_name{};
        do {
          terminal_name = "C" + std::to_string(constant_count++);
        } while ((rules.find(terminal_name) != rules.end()) &&
                 (copy.find(terminal_name) != copy.end()));

        new_terminals.insert(std::make_pair(terminal_value, terminal_name));
      }

      it->second[i] = new_terminals.find(terminal_value)->second;
    }
  }

  for (auto it = new_terminals.begin(); it != new_terminals.end(); it++) {
    copy.insert(std::make_pair(std::string{it->second},
                               std::vector{std::string{it->first}}));
  }

  while (copy.begin() != copy.end()) {
    auto pair = copy.extract(copy.begin());
    if (pair.mapped().size() < 3) {
      rules.insert(std::pair(pair.key(), pair.mapped()));
      continue;
    }

    std::string lhs{pair.key()};
    std::vector<std::string> rhs{pair.mapped()};

    for (size_t i = 0; i < rhs.size() - 1; i++) {
      std::string combigned_name{rhs[i]};
      combigned_name += "_" + rhs[i + 1];

      auto existing_replacement = this->rules.find(combigned_name);

      if (existing_replacement != this->rules.end()) {
        rhs.erase(rhs.begin() + i);
        rhs[i] = existing_replacement->first;
      }
    }

    while (rhs.size() > 2) {
      std::string first = rhs[rhs.size() - 1];
      std::string second = rhs[rhs.size() - 2];
      rhs.pop_back();
      rhs.pop_back();
      std::string combigned_name = first;
      combigned_name += "_" + second;

      this->rules.insert(std::pair(combigned_name, std::vector{first, second}));
      rhs.push_back(combigned_name);
    }
  }

  return true;
}