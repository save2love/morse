// Turns Morse key into USB keyboard
#include <Keyboard.h>
#include <Bounce2.h> // include de-bounce library
#include "Config.h"
#include "Menu.h"

const uint8_t ledPin = 4;       // LED pin
const uint8_t buzzPin = 9;      // Buzzer pin
const uint8_t funcKeyPin = 8;   // Functional key pin
const uint8_t morseKeyPin = 7;  // Morse Key pin

Bounce funcKey = Bounce(funcKeyPin, 10);  // 10 ms debounce
Bounce morseKey = Bounce(morseKeyPin, 10);  // 10 ms debounce

unsigned long funcKeyPressedAt = 0;
uint8_t funcKeyButtonState = 0; // 0 - not pressed; 1 - single press; 2 - long press

const unsigned long dashThresh = 150; // time threshold in ms to differentiate dots from dashes
const unsigned long letterThresh = 500; // time threshold in ms to differentiate letter gaps
const unsigned long wordThresh = 1000; // time threshold in ms to differentiate word gaps

String inputString = ""; // initialise input string

unsigned long downTime = 0; // records the start time of state change
unsigned long upTime = 0; // records the end time of state change
unsigned long timeNow = 0; // records the current time
unsigned long changeDuration = 0; // records the duration of state change
unsigned long pauseDuration = 0; // records the duration of the last pause
uint8_t pauseFlag = 0; // initilise the flag to indicate whether a pause has already been evaluated

// Menu structure
// The first level is the main settings
// The second and others are options of particular setting
// Short press  - move to another setting or option
// Long press   - enter into current setting or use current option
// Double click - move back in tree
// ┬ Sound
// │  ├ On
// │  └ Off
// ├ Capitalize
// │  ├ On
// │  └ Off
// └ Language
//     ├ English
//     └ Russian

MenuItem m1_Sound = MenuItem("Sound");
MenuItem m1_On = MenuItem("On");
MenuItem m1_Off = MenuItem("Off");

MenuItem m2_Capitalize = MenuItem("Capitalize");
MenuItem m2_On = MenuItem("On");
MenuItem m2_Off = MenuItem("Off");

MenuItem m3_Language = MenuItem("Language");
MenuItem m3_English = MenuItem("English");
MenuItem m3_Russian = MenuItem("Russian");

void menuUseEvent(MenuUseEvent used);
void menuChangeEvent(MenuChangeEvent changed);
Menu menu = Menu(m1_Sound, menuUseEvent, menuChangeEvent);

enum State {
  ST_Default, // Normal state to read morze
  ST_Menu     // Menu mode
};
State State = ST_Default;

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
  pinMode(ledPin, OUTPUT); // configure the pin connected to the led as an output
  pinMode(buzzPin, OUTPUT);
  pinMode(funcKeyPin, INPUT_PULLUP); // configure the pin connected to the morse key as a pullup
  pinMode(morseKeyPin, INPUT_PULLUP); // configure the pin connected to the morse key as a pullup
  delay(10);
  menu.getRoot().add(m2_Capitalize).add(m3_Language);
  menu.getRoot().addSubItem(m1_On).add(m1_Off);
  m2_Capitalize.addSubItem(m2_On).add(m2_Off);
  m3_Language.addSubItem(m3_English).add(m3_Russian);
  delay(10);

  inputString = "";

  LoadCurrentConfig();
}

void loop()
{
  checkPause();

  if (morseKey.update()) {
    if (morseKey.risingEdge()) { // if input from key has gone to 1 and model is still 0, update model
      keyUp();
    } else if (morseKey.fallingEdge()) { // if input from key has gone to 0 and model is still 1, update model
      keyDown();
    }
  }

  if (funcKey.update()) {
    int value = funcKey.read();
    if (value == HIGH) {
      funcKeyButtonState = 0;
    } else {
      funcKeyButtonState = 1;
      funcKeyPressedAt = millis();
    }
  }

  if (funcKeyButtonState == 1 && millis() - funcKeyPressedAt >= 1000)
  { // Long press (> 3 sec)
    funcKeyButtonState = 0;
    State = ST_Menu;    // If device was in normal state and func key was pressed more than 3 sec - go to Menu Mode
    menu.toRoot();      // Reset menu
    upTime = millis();
  }
}

void keyDown()
{
  downTime = millis();
  digitalWrite(ledPin, HIGH); // switch LED on
  if (Config.IsBeep) digitalWrite(buzzPin, HIGH); // Enable buzzer
}

void keyUp()
{
  upTime = millis();

  changeDuration = upTime - downTime;
  digitalWrite(ledPin, LOW); // switch LED off
  if (Config.IsBeep) digitalWrite(buzzPin, LOW); // disable buzzer

  if (changeDuration > 0 and changeDuration < dashThresh) {
    inputString = inputString + ".";
  } else if (changeDuration >= dashThresh) {
    inputString = inputString + "-";
  }

  pauseFlag = 1;
}

void checkPause()
{
  pauseDuration = millis() - upTime;

  switch (State) {
    case ST_Default:
      if (pauseDuration >= letterThresh and pauseDuration < wordThresh and pauseFlag) {
        // if the preceding pause was long enough AND a pause hasn't just been evaluated, evaluate the previous inputs as a single letter
        evaluateLetter();
        pauseFlag = 0;
      } else if (pauseDuration >= wordThresh and pauseFlag) {
        evaluateLetter();
        pressKey(' ');
        pauseFlag = 0;
      }
      break;

    case ST_Menu:
      if (pauseDuration >= letterThresh and pauseDuration < wordThresh and pauseFlag) {
        if (inputString == "-") {
          if (menu.getCurrent().getSubItem())
            menu.moveSubItem();
          else
            menu.use();
          inputString = "";
        } else if (inputString == "..") {
          if (menu.getCurrent() == menu.getRoot()) {
            State = ST_Default; // Return to the Default state
            clearMenu();
#ifdef DEBUG
            Serial.println(F("Exit from menu Mode"));
#endif
          }
          else {
            menu.moveBack();
          }
          inputString = "";
        } else {
          menu.moveNext();
          inputString = "";
        }
        pauseFlag = 0;
      }

      if (pauseDuration > 10000) { // Return to default state after 10 second
        //clearMenu();
        pauseFlag = 0;
        inputString = "";   // Reset input string
        State = ST_Default; // Return to the Default state
#ifdef DEBUG
        Serial.println(F("Auto exit from menu Mode"));
#endif
      }
      break;
  }
}

void pressKey(char c) {
  Keyboard.press(c);
  Keyboard.release(c);
  inputString = ""; // re-initialise inputString ready for new letter
}

bool evaluateEnglishLetter() {
  if (inputString == ".-") {
    pressKey(Config.IsCapitalize ? 'A' : 'a');
  } else if (inputString == "-...") {
    pressKey(Config.IsCapitalize ? 'B' : 'b');
  } else if (inputString == "-.-.") {
    pressKey(Config.IsCapitalize ? 'C' : 'c');
  } else if (inputString == "-..") {
    pressKey(Config.IsCapitalize ? 'D' : 'd');
  } else if (inputString == ".") {
    pressKey(Config.IsCapitalize ? 'E' : 'e');
  } else if (inputString == "..-.") {
    pressKey(Config.IsCapitalize ? 'F' : 'f');
  } else if (inputString == "--.") {
    pressKey(Config.IsCapitalize ? 'G' : 'g');
  } else if (inputString == "....") {
    pressKey(Config.IsCapitalize ? 'H' : 'h');
  } else if (inputString == "..") {
    pressKey(Config.IsCapitalize ? 'I' : 'i');
  } else if (inputString == ".---") {
    pressKey(Config.IsCapitalize ? 'J' : 'j');
  } else if (inputString == "-.-") {
    pressKey(Config.IsCapitalize ? 'K' : 'k');
  } else if (inputString == ".-..") {
    pressKey(Config.IsCapitalize ? 'L' : 'l');
  } else if (inputString == "--") {
    pressKey(Config.IsCapitalize ? 'M' : 'm');
  } else if (inputString == "-.") {
    pressKey(Config.IsCapitalize ? 'N' : 'n');
  } else if (inputString == "---") {
    pressKey(Config.IsCapitalize ? 'O' : 'o');
  } else if (inputString == ".--.") {
    pressKey(Config.IsCapitalize ? 'P' : 'p');
  } else if (inputString == "--.-") {
    pressKey(Config.IsCapitalize ? 'Q' : 'q');
  } else if (inputString == ".-.") {
    pressKey(Config.IsCapitalize ? 'R' : 'r');
  } else if (inputString == "...") {
    pressKey(Config.IsCapitalize ? 'S' : 's');
  } else if (inputString == "-") {
    pressKey(Config.IsCapitalize ? 'T' : 't');
  } else if (inputString == "..-") {
    pressKey(Config.IsCapitalize ? 'U' : 'u');
  } else if (inputString == "...-") {
    pressKey(Config.IsCapitalize ? 'V' : 'v');
  } else if (inputString == ".--") {
    pressKey(Config.IsCapitalize ? 'W' : 'w');
  } else if (inputString == "-..-") {
    pressKey(Config.IsCapitalize ? 'X' : 'x');
  } else if (inputString == "-.--") {
    pressKey(Config.IsCapitalize ? 'Y' : 'y');
  } else if (inputString == "--..") {
    pressKey(Config.IsCapitalize ? 'Z' : 'z');
  } else {
    return false;
  }
  return true;
}

// Use english key codes to print russian letters
bool evaluateRussianLetter() {
  if (inputString == ".-") {
    pressKey(Config.IsCapitalize ? 'F' : 'f'); // А
  } else if (inputString == "-...") {
    pressKey(Config.IsCapitalize ? '<' : ','); // Б
  } else if (inputString == ".--") {
    pressKey(Config.IsCapitalize ? 'D' : 'd'); // В
  } else if (inputString == "--.") {
    pressKey(Config.IsCapitalize ? 'U' : 'u'); // Г
  } else if (inputString == "-..") {
    pressKey(Config.IsCapitalize ? 'L' : 'l'); // Д
  } else if (inputString == ".") {
    pressKey(Config.IsCapitalize ? 'T' : 't'); // Е(Ё)
  } else if (inputString == "...-") {
    pressKey(Config.IsCapitalize ? ':' : ';'); // Ж
  } else if (inputString == "--..") {
    pressKey(Config.IsCapitalize ? 'P' : 'p'); // З
  } else if (inputString == "..") {
    pressKey(Config.IsCapitalize ? 'B' : 'b'); // И
  } else if (inputString == ".---") {
    pressKey(Config.IsCapitalize ? 'Q' : 'q'); // Й
  } else if (inputString == "-.-") {
    pressKey(Config.IsCapitalize ? 'R' : 'r'); // К
  } else if (inputString == ".-..") {
    pressKey(Config.IsCapitalize ? 'K' : 'k'); // Л
  } else if (inputString == "--") {
    pressKey(Config.IsCapitalize ? 'V' : 'v'); // М
  } else if (inputString == "-.") {
    pressKey(Config.IsCapitalize ? 'Y' : 'y'); // Н
  } else if (inputString == "---") {
    pressKey(Config.IsCapitalize ? 'J' : 'j'); // О
  } else if (inputString == ".--.") {
    pressKey(Config.IsCapitalize ? 'G' : 'g'); // П
  } else if (inputString == ".-.") {
    pressKey(Config.IsCapitalize ? 'H' : 'h'); // Р
  } else if (inputString == "...") {
    pressKey(Config.IsCapitalize ? 'C' : 'c'); // С
  } else if (inputString == "-") {
    pressKey(Config.IsCapitalize ? 'N' : 'n'); // Т
  } else if (inputString == "..-") {
    pressKey(Config.IsCapitalize ? 'E' : 'e'); // У
  } else if (inputString == "..-.") {
    pressKey(Config.IsCapitalize ? 'A' : 'a'); // Ф
  } else if (inputString == "....") {
    pressKey(Config.IsCapitalize ? '{' : '['); // Х
  } else if (inputString == "-.-.") {
    pressKey(Config.IsCapitalize ? 'W' : 'w'); // Ц
  } else if (inputString == "---.") {
    pressKey(Config.IsCapitalize ? 'X' : 'x'); // Ч
  } else if (inputString == "----") {
    pressKey(Config.IsCapitalize ? 'I' : 'i'); // Ш
  } else if (inputString == "--.-") {
    pressKey(Config.IsCapitalize ? 'O' : 'o'); // Щ
  } else if (inputString == "-.--") {
    pressKey(Config.IsCapitalize ? 'S' : 's'); // Ы
  } else if (inputString == "-..-") {
    pressKey(Config.IsCapitalize ? 'M' : 'm'); // ь(Ъ)
  } else if (inputString == "--.--") {
    pressKey(Config.IsCapitalize ? 'X' : 'x'); // Ъ
  } else if (inputString == "..-..") {
    pressKey(Config.IsCapitalize ? '"' : '\''); // Э
  } else if (inputString == "..--") {
    pressKey(Config.IsCapitalize ? '>' : '.'); // Ю
  } else if (inputString == ".-.-") {
    pressKey(Config.IsCapitalize ? 'Z' : 'z'); // Я
  } else {
    return false;
  }
  return true;
}

bool evaluateNumbers() {
  if (inputString == ".----") {
    pressKey('1');
  } else if (inputString == "..---") {
    pressKey('2');
  } else if (inputString == "...--") {
    pressKey('3');
  } else if (inputString == "....-") {
    pressKey('4');
  } else if (inputString == ".....") {
    pressKey('5');
  } else if (inputString == "-....") {
    pressKey('6');
  } else if (inputString == "--...") {
    pressKey('7');
  } else if (inputString == "---..") {
    pressKey('8');
  } else if (inputString == "----.") {
    pressKey('9');
  } else if (inputString == "-----") {
    pressKey('0');
  } else {
    return false;
  }
  return true;
}

void evaluateLetter()
{
#ifdef DEBUG
  Serial.println(inputString);
#endif

  switch (Config.Language) {
    default:
    case 0: if (evaluateEnglishLetter()) return; break;
    case 1: if (evaluateRussianLetter()) return; break;
  }

  if (!evaluateNumbers()) {
#endif DEBUG
    Serial.print("Unknown sequence: ");
    Serial.println(inputString);
#endif
    pressKey('-');
  }
}

void clearMenu() {
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_HOME);
  delay(1);
  Keyboard.press(KEY_DELETE);
  Keyboard.releaseAll();
}

void printMenuItem(MenuItem item) {
  const char* name = item.getName();
  Keyboard.print(name);
}

void menuUseEvent(MenuUseEvent used)
{
#ifdef DEBUG
  Serial.print(F("Menu used "));
  Serial.println(used.item.getName());
#endif
  if (used.item == m1_On) {
    Config.IsBeep = true;
    SaveCurrentConfig();
  } else if (used.item == m1_Off) {
    Config.IsBeep = false;
    SaveCurrentConfig();
  } else if (used.item == m2_On) {
    Config.IsCapitalize = true;
    SaveCurrentConfig();
  } else if (used.item == m2_Off) {
    Config.IsCapitalize = false;
    SaveCurrentConfig();
  } else if (used.item == m3_English) {
    Config.Language = 0;
    SaveCurrentConfig();
  } else if (used.item == m3_Russian) {
    Config.Language = 1;
    SaveCurrentConfig();
  }
  menu.moveBack();
}

void menuChangeEvent(MenuChangeEvent changed)
{
#ifdef DEBUG
  Serial.print("Menu changed: ");
  Serial.print(changed.from.getName());
  Serial.print(" -> ");
  Serial.println(changed.to.getName());
#endif
  clearMenu();
  printMenuItem(changed.to);
}

