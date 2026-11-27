#include "eadkpp.h"
#include "palette.h"

#include <stdio.h>
#include <string.h>

extern const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Calculator";
extern const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

namespace {

constexpr int k_maxEntryLength = 14;
constexpr double k_maxValue = 999999999.0;

char entry[24] = "";
double leftOperand = 0.0;
char operation = 0;
bool hasResult = false;

bool parseEntry(double * value) {
  if (entry[0] == '\0' || (entry[0] == '-' && entry[1] == '\0')) {
    return false;
  }

  bool negative = entry[0] == '-';
  int i = negative ? 1 : 0;
  bool decimal = false;
  double result = 0.0;
  double place = 0.1;
  for (; entry[i] != '\0'; i++) {
    if (entry[i] == '.') {
      if (decimal) {
        return false;
      }
      decimal = true;
    } else if (entry[i] < '0' || entry[i] > '9') {
      return false;
    } else if (decimal) {
      result += (entry[i] - '0') * place;
      place *= 0.1;
    } else {
      result = result * 10.0 + (entry[i] - '0');
    }
  }
  if (result > k_maxValue) {
    return false;
  }
  *value = negative ? -result : result;
  return true;
}

void formatValue(double value, char * buffer, size_t bufferSize) {
  bool negative = value < 0.0;
  if (negative) {
    value = -value;
  }

  long whole = static_cast<long>(value);
  long fraction = static_cast<long>((value - whole) * 1000000.0 + 0.5);
  if (fraction == 1000000) {
    whole++;
    fraction = 0;
  }
  if (fraction == 0) {
    snprintf(buffer, bufferSize, "%s%ld", negative && whole != 0 ? "-" : "", whole);
    return;
  }

  char fractionBuffer[7];
  snprintf(fractionBuffer, sizeof(fractionBuffer), "%06ld", fraction);
  int end = 5;
  while (end >= 0 && fractionBuffer[end] == '0') {
    fractionBuffer[end--] = '\0';
  }
  snprintf(buffer, bufferSize, "%s%ld.%s",
           negative ? "-" : "", whole, fractionBuffer);
}

void clearCalculator() {
  entry[0] = '\0';
  operation = 0;
  hasResult = false;
}

void showError() {
  strcpy(entry, "Error");
  operation = 0;
  hasResult = true;
}

bool calculate(double rightOperand, double * result) {
  switch (operation) {
    case '+': *result = leftOperand + rightOperand; break;
    case '-': *result = leftOperand - rightOperand; break;
    case '*': *result = leftOperand * rightOperand; break;
    case '/':
      if (rightOperand == 0.0) {
        return false;
      }
      *result = leftOperand / rightOperand;
      break;
    default: return false;
  }
  return *result == *result && *result <= k_maxValue && *result >= -k_maxValue;
}

void draw() {
  EADK::Display::pushRectUniform(EADK::Screen::Rect, Black);
  EADK::Display::drawString("CALCULATOR", EADK::Point(10, 20), true, White, Black);

  char display[48];
  if (operation != 0) {
    char left[20];
    formatValue(leftOperand, left, sizeof(left));
    snprintf(display, sizeof(display), "%s %c %s", left, operation, entry);
    EADK::Display::drawString(display, EADK::Point(10, 70), false, White, Black);
    EADK::Display::drawString(entry[0] == '\0' ? "0" : entry,
                              EADK::Point(10, 100), true, Yellow, Black);
  } else {
    EADK::Display::drawString(entry[0] == '\0' ? "0" : entry,
                              EADK::Point(10, 85), true, Yellow, Black);
  }
  EADK::Display::drawString("EXE or OK: calculate", EADK::Point(10, 150), false, White, Black);
  EADK::Display::drawString("Back: clear  DEL: erase", EADK::Point(10, 175), false, White, Black);
}

void appendDigit(char digit) {
  if (hasResult) {
    clearCalculator();
  }
  int length = strlen(entry);
  if (length < k_maxEntryLength) {
    entry[length] = digit;
    entry[length + 1] = '\0';
  }
}

void processKey(EADK::Keyboard::Key key) {
  char digit = 0;
  switch (key) {
    case EADK::Keyboard::Key::Zero: digit = '0'; break;
    case EADK::Keyboard::Key::One: digit = '1'; break;
    case EADK::Keyboard::Key::Two: digit = '2'; break;
    case EADK::Keyboard::Key::Three: digit = '3'; break;
    case EADK::Keyboard::Key::Four: digit = '4'; break;
    case EADK::Keyboard::Key::Five: digit = '5'; break;
    case EADK::Keyboard::Key::Six: digit = '6'; break;
    case EADK::Keyboard::Key::Seven: digit = '7'; break;
    case EADK::Keyboard::Key::Eight: digit = '8'; break;
    case EADK::Keyboard::Key::Nine: digit = '9'; break;
    default: break;
  }
  if (digit != 0) {
    appendDigit(digit);
    return;
  }

  if (key == EADK::Keyboard::Key::Dot) {
    if (hasResult) {
      clearCalculator();
    }
    if (strchr(entry, '.') == 0 && strlen(entry) < k_maxEntryLength - 1) {
      int length = strlen(entry);
      if (length == 0 || (length == 1 && entry[0] == '-')) {
        entry[length++] = '0';
      }
      entry[length++] = '.';
      entry[length] = '\0';
    }
  } else if (key == EADK::Keyboard::Key::Minus && entry[0] == '\0') {
    if (hasResult) {
      clearCalculator();
    }
    strcpy(entry, "-");
  } else if (key == EADK::Keyboard::Key::Plus ||
             key == EADK::Keyboard::Key::Minus ||
             key == EADK::Keyboard::Key::Multiplication ||
             key == EADK::Keyboard::Key::Division) {
    if (entry[0] != '\0' && !hasResult) {
      double value;
      if (!parseEntry(&value)) {
        showError();
        return;
      }
      if (operation != 0) {
        if (!calculate(value, &leftOperand)) {
          showError();
          return;
        }
      } else {
        leftOperand = value;
      }
      entry[0] = '\0';
    } else if (hasResult) {
      if (!parseEntry(&leftOperand)) {
        clearCalculator();
        return;
      }
      entry[0] = '\0';
      hasResult = false;
    } else if (operation == 0) {
      return;
    }
    operation = key == EADK::Keyboard::Key::Plus ? '+' :
                key == EADK::Keyboard::Key::Minus ? '-' :
                key == EADK::Keyboard::Key::Multiplication ? '*' : '/';
  } else if (key == EADK::Keyboard::Key::EXE || key == EADK::Keyboard::Key::OK) {
    double value, result;
    if (operation != 0 && parseEntry(&value)) {
      if (calculate(value, &result)) {
        formatValue(result, entry, sizeof(entry));
        operation = 0;
        hasResult = true;
      } else {
        showError();
      }
    }
  } else if (key == EADK::Keyboard::Key::Back) {
    clearCalculator();
  } else if (key == EADK::Keyboard::Key::Backspace) {
    if (hasResult) {
      clearCalculator();
    } else if (entry[0] != '\0') {
      entry[strlen(entry) - 1] = '\0';
    }
  }
}

}

int main(int argc, char * argv[]) {
  (void)argc;
  (void)argv;
  draw();

  constexpr EADK::Keyboard::Key keys[] = {
    EADK::Keyboard::Key::Zero, EADK::Keyboard::Key::One,
    EADK::Keyboard::Key::Two, EADK::Keyboard::Key::Three,
    EADK::Keyboard::Key::Four, EADK::Keyboard::Key::Five,
    EADK::Keyboard::Key::Six, EADK::Keyboard::Key::Seven,
    EADK::Keyboard::Key::Eight, EADK::Keyboard::Key::Nine,
    EADK::Keyboard::Key::Dot, EADK::Keyboard::Key::Plus,
    EADK::Keyboard::Key::Minus, EADK::Keyboard::Key::Multiplication,
    EADK::Keyboard::Key::Division, EADK::Keyboard::Key::EXE,
    EADK::Keyboard::Key::OK, EADK::Keyboard::Key::Back,
    EADK::Keyboard::Key::Backspace
  };
  EADK::Keyboard::State previous;
  while (true) {
    EADK::Keyboard::State current = EADK::Keyboard::scan();
    bool changed = false;
    for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
      if (current.keyDown(keys[i]) && !previous.keyDown(keys[i])) {
        processKey(keys[i]);
        changed = true;
      }
    }
    if (changed) {
      draw();
    }
    previous = current;
    EADK::Timing::msleep(20);
  }
}
