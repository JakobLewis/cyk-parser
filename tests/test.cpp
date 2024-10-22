#include "CYK_Parser.hpp"
#include "Grammar.hpp"
#include <iostream>

void test_grammar(std::string filename, std::string sentance,
                  bool expected_result) {
  ContextFreeGrammar cfg;

  std::cout << "Checking " + filename + " - \"" + sentance + "\" - ";

  if (!cfg.try_load("../tests/" + filename)) {
    std::cerr << "failed to load CFG" << std::endl;
    return;
  }

  ChomskyNormalForm cnf;

  if (!cnf.try_from_CFG(cfg)) {
    std::cerr << "failed to convert CFG into CNF" << std::endl;
    return;
  }

  bool result = is_valid_cyk(sentance, cnf);

  if (result == expected_result) {
    std::cout << "PASSED" << std::endl;
  } else {
    std::cout << "FAILED" << std::endl;
    std::cout << "   Got " << result << " and expected " << expected_result
              << std::endl;
  }
}

int main() {
  // Try a basic CFG Grammar
  test_grammar("grammar1.txt", "I shot an elephant in my pajamas", true);
  test_grammar("grammar1.txt", "I shot elephant an in my my", false);
  // Try a grammar already in CNF
  test_grammar("grammar2.txt", "a very heavy orange book", true);
  test_grammar("grammar2.txt", "a very tall extremely muscular man", true);
  test_grammar("grammar2.txt", "a very book book book", false);
  test_grammar("grammar2.txt", "book orange heavy very a", false);
  // Basic delimeter discovery
  test_grammar("grammar3.txt", "0 # 1", true);
  test_grammar("grammar3.txt", "0 0 0 # 1 1", true);
  test_grammar("grammar3.txt", " 0 0 # 1 1 0", false);
  test_grammar("grammar3.txt", "# 0 0 # 1 1 1", false);
  // Noughts and crosses!
  test_grammar("grammar4.txt", "x x o , o o x , x x x", true);
  test_grammar("grammar4.txt", "x x o x , o x , x x x", false);

  // Try a sentance with unknown tokens
  test_grammar("grammar4.txt", "x x o , o o x + , x x x", false);
  test_grammar("grammar4.txt", "x x o , o o x , x x x badtoken", false);

  // Try a sentance with a recursive start symbol
  test_grammar("grammar5.txt", ". . . . .", true);
  test_grammar("grammar5.txt", ".", true);

  return 0;
}
