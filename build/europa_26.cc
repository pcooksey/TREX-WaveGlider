#include <PLASMA/Token.hh>

bool isEffect(EUROPA::TokenId const &tok) {
  return tok->hasAttributes(EUROPA::PSTokenType::EFFECT);
}

int main() { return 0; }
