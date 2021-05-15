#include "functional-avr/nonstd.h"
namespace std {
  template<class Sig>
  using function = nonstd::function<Sig>;
}

void keysetLoop(std::function<void(char)> keyboardWrite);
void keysetSetup();
int getNumberOfModes();