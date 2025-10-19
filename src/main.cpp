#include "util.h"

EspUsbHost usbhost;
BleComboKeyboard blekeyboard;
BleComboMouse blemouse(&blekeyboard);
// BleKeyboard blekeyboard;
// BleMouse blemouse;
std::vector<keypress> keys;

const unsigned long repeatDelay = 500; // auto-repeat delay
const unsigned long repeatRate = 50;   // auto-repeat rate

void onKeyboard(hid_keyboard_report_t report, hid_keyboard_report_t last_report) {
	unsigned long currentTime = millis();
	// Handle modifier keys
	if (report.modifier & 0x01) blekeyboard.press(KEY_LEFT_CTRL); 
	else blekeyboard.release(KEY_LEFT_CTRL);
	if (report.modifier & 0x02) blekeyboard.press(KEY_LEFT_SHIFT);
	else blekeyboard.release(KEY_LEFT_SHIFT);
	if (report.modifier & 0x04) blekeyboard.press(KEY_LEFT_ALT);
	else blekeyboard.release(KEY_LEFT_ALT);
	if (report.modifier & 0x08) blekeyboard.press(KEY_LEFT_GUI);
	else blekeyboard.release(KEY_LEFT_GUI);
	if (report.modifier & 0x10) blekeyboard.press(KEY_RIGHT_CTRL);
	else blekeyboard.release(KEY_RIGHT_CTRL);
	if (report.modifier & 0x20) blekeyboard.press(KEY_RIGHT_SHIFT);
	else blekeyboard.release(KEY_RIGHT_SHIFT);
	if (report.modifier & 0x40) blekeyboard.press(KEY_RIGHT_ALT);
	else blekeyboard.release(KEY_RIGHT_ALT);
	if (report.modifier & 0x80) blekeyboard.press(KEY_RIGHT_GUI);
	else blekeyboard.release(KEY_RIGHT_GUI);
	// Press new keys	
	for (uint8_t i = 0; i < 6; ++i) {
		uint8_t key = report.keycode[i];
		if (key != 0) {
			auto it = std::find_if(keys.begin(), keys.end(), [key](const keypress& kp) { return kp.key == key; });
			if (it == keys.end()) {
				keys.push_back({key, currentTime});
				blekeyboard.press(keycodes[key]);
				Serial.printf("Keyboard Key: %d\n", keycodes[key]);
			} else {
				it->pressTime = currentTime;
			}
		}
	}
	// Release keys that are no longer pressed	
	for (auto it = keys.begin(); it != keys.end();) {
		bool autorepeat = false;
		for (uint8_t i = 0; i < 6; ++i) {
			if (report.keycode[i] == it->key) {
				autorepeat = true;
				break;
			}
		}

		if (!autorepeat) {
			blekeyboard.release(keycodes[it->key]);
			it = keys.erase(it);
		} else {
			++it;
		}
	}
}

// debug mouse events
void dump_report(const uint8_t* rpt, size_t len) {
  printf("HID rpt (%u):", (unsigned)len);
  for (size_t i=0;i<len;i++) printf(" %02X", rpt[i]);
  printf("\n");
}

 void parse_mouse_report(const uint8_t* rpt, size_t len) {
  if (len == 0) return;

  uint8_t id = rpt[0];
  const uint8_t* p = rpt + 1;
  size_t n = len - 1;

  switch (id) {
    case 1: { // Mouse X/Y + Buttons
      if (n < 3) return;
      uint8_t buttons = p[0];
      int8_t  dx = (int8_t)p[1];
      int8_t  dy = (int8_t)p[2];
      printf("Mouse Move: dx=%d, dy=%d, buttons=%02X\n", dx, dy, buttons);
    } break;

    case 2: { // Wheel (vertical) + éventuellement pan
      // Certaines souris envoient wheel seul: 1 octet signé
      if (n >= 1) {
        int8_t wheel = (int8_t)p[0];
        printf("Wheel: %d\n", wheel);
      }
    } break;

    default:
      dump_report(rpt, len); // Pour inspection
      break;
  }
}
// Mouse callback
 void onMouse(hid_mouse_report_t report, uint8_t last_buttons) {
    uint8_t changed = report.buttons ^ last_buttons;
	parse_mouse_report((const uint8_t*)&report, sizeof(report));
	
    // Mouvement + molette (wheel sur 8 bits signé)
    int8_t dx = (int8_t)report.x;
    int8_t dy = (int8_t)report.y;
    int8_t wheel = (int8_t)report.wheel;
    if (dx != 0 || dy != 0 || wheel != 0) blemouse.move(dx, dy, wheel);

	// Boutons
    if (changed & MOUSE_BUTTON_LEFT)   (report.buttons & MOUSE_BUTTON_LEFT)   ? blemouse.press(MOUSE_LEFT)   : blemouse.release(MOUSE_LEFT);
    if (changed & MOUSE_BUTTON_RIGHT)  (report.buttons & MOUSE_BUTTON_RIGHT)  ? blemouse.press(MOUSE_RIGHT)  : blemouse.release(MOUSE_RIGHT);
    if (changed & MOUSE_BUTTON_MIDDLE) (report.buttons & MOUSE_BUTTON_MIDDLE) ? blemouse.press(MOUSE_MIDDLE) : blemouse.release(MOUSE_MIDDLE);
 }



void setup() {
	Serial.println("Setup strarting...");
	Serial.begin(115200);
	usbhost.begin(); Serial.println("- Starting USB Host done");
	blekeyboard.begin(); Serial.println("-- Starting BLE Keyboard done");
	usbhost.setHIDLocal(HID_LOCAL_French);
	usbhost.setKeyboardCallback(onKeyboard); Serial.println("--- set KeyboardCallback done");
	usbhost.setMouseCallback(onMouse); Serial.println("--- set MouseCallback done");
	Serial.println("Setup complete");
}

void loop() {
	// resete esp32 when shortcut key is pressed
	// if (blekeyboard.isConnected()) {
	// 	if (blekeyboard.press(KEY_LEFT_CTRL) && blekeyboard.press(KEY_LEFT_SHIFT) && blekeyboard.press(KEY_DOWN_ARROW)) {
	// 		Serial.println("Resetting ESP32...");
	// 		blekeyboard.releaseAll();
	// 		blekeyboard.end();
	// 		delay(1000);
	// 		ESP.restart();
	// 	}
	// }

	// Send a message every 1 milliseconds
	static unsigned long lastLoop = 0;
	if (millis() - lastLoop > 10) {
		usbhost.task();
		lastLoop = millis();
	}
}
