#pragma once
#include <map>
#include <string>
#include <vector>

/**
 * Context-free Grammar Rule
 */
typedef std::multimap<std::string, std::vector<std::string>> Grammar_Rules_t;

class ContextFreeGrammar {
public:
  Grammar_Rules_t rules;

  bool try_load(std::string file_path);
};

class ChomskyNormalForm {
public:
  Grammar_Rules_t rules;
  size_t constant_count;

  ChomskyNormalForm() : constant_count{} {}

  bool try_from_CFG(const ContextFreeGrammar &grammar);
};