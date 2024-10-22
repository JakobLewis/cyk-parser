#include "CYK_Parser.hpp"
#include <iostream>
#include <set>
#include <vector>

bool is_valid_cyk(std::string sentance, const ChomskyNormalForm &grammar) {

  /**
   * First split sentance by spaces and wrap each token with "'" characters so
   * they match the grammar definitions.
   */
  std::vector<std::string> tokens{};
  while (true) {
    size_t next_space = sentance.find(' ');

    if (next_space == std::string ::npos) {
      tokens.push_back("'" + sentance + "'");
      break;
    } else {
      tokens.push_back("'" + sentance.substr(0, next_space) + "'");
      sentance.erase(0, next_space + 1);
    }
  }

  std::vector<std::vector<std::set<std::string>>> table{};
  for (size_t i = 0; i < tokens.size(); i++) {
    table.push_back(std::vector<std::set<std::string>>{tokens.size()});
  }

  /**
   * First, process the terminal symbols. All valid sentances must first be
   * recognisable as a series of known tokens.
   */
  for (size_t i = 0; i < tokens.size(); i++) {
    for (auto it = grammar.rules.begin(); it != grammar.rules.end(); it++) {
      if ((it->second.size() == 1) && (it->second[0] == tokens[i])) {
        table[i][i].insert(it->first);
      }
    }

    if (table[i][i].size() == 0) {
      // Sentance contained unknown token
      return false;
    }
  }

  /**
   * Here is the meat of the algorithm. We build out our triangular table of
   * possible valid grammar by exhaustively checking all possible
   * interpretations. While slow, this process is sped up by using CNF to
   * represent our rules. Every comparison only ever checks if two tokens are in
   * the grammar.
   *
   * I chose to use words (i.e. space-seperated) instead of characters mostly to
   * make debugging (and constructing grammars) easier for me. However there is
   * no reason why this implementation couldn't be refactired to operate on
   * characters.
   */
  for (size_t l = 1; l < tokens.size(); l++) { // For every span
    for (size_t r = 0; r < (tokens.size() - l); r++) {
      for (size_t t = 0; t < l;
           t++) { // And every possible way to split that span into two

        auto first = table[r][r + t];
        auto second = table[r + t + 1][r + l];

        std::string terminal_symbol{};
        std::string nonterminal_symbol{};
        for (auto it = grammar.rules.begin(); it != grammar.rules.end(); it++) {
          if (it->second.size() != 2) {
            continue;
          }

          /**
           * Check every rule's RHS and see if any combination of the two
           * "lower" symbols matches. If so, add the LHS of that rule to the
           * next row of the table.
           *
           * This entire section could have been O(nm) where n is the size of
           * first and m is the size of second. However I represented the rules
           * as RHS -> LHS in the multimap, so it needs to be O(nmr) where r is
           * the number of rules. Creating an 'inverted' multimap wouldn't be
           * too hard, but this assignment is late enough.
           */

          for (auto ts_it = first.begin(); ts_it != first.end(); ts_it++) {
            if (*ts_it != it->second[0]) {
              continue;
            }
            for (auto nts_it = second.begin(); nts_it != second.end();
                 nts_it++) {
              if (*nts_it == it->second[1]) {
                table[r][r + l].insert(it->first);
              }
            }
          }
        }
      }
    }
  }

#ifdef DEBUG
  std::cout << std::endl;
  for (size_t i = 0; i < tokens.size(); i++) {
    for (size_t j = 0; j < tokens.size(); j++) {
      auto set = table[i][j];
      std::cout << "{";
      for (auto it = set.begin(); it != set.end(); it++) {
        std::cout << *it << ",";
      }
      std::cout << "}";
    }
    std::cout << std::endl;
  }
#endif

  /**
   * If the top-right set contains the starting symbol, there is at least 1 way
   * in which the sentance can be parsed in the grammar.
   */
  auto last = table[0][tokens.size() - 1];

  return (last.find(std::string{"S"}) != last.end());
}