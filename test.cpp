#include "Grammar.hpp"
#include <iostream>

int main() {
  ContextFreeGrammar r;

  std::cout << r.try_load("../test.grammar") << std::endl;

  ChomskyNormalForm c;

  c.try_from_CFG(r);

  for (auto it = c.rules.begin(); it != c.rules.end(); it++) {
    std::cout << it->first << " -> " << std::endl;
    for (size_t i = 0; i < it->second.size(); i++) {
      std::cout << "  | " << it->second[i] << std::endl;
    }
  }

  return 0;
}
