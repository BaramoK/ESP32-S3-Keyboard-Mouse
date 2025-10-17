#include "util.h"

EspUsbHost usbhost;
BleComboKeyboard blekeyboard;
BleComboMouse blemouse(&blekeyboard);
std::vector<keypress> keys;
Adafruit_NeoPixel strip(LED_COUNT, RGB_BUILTIN, NEO_GRB + NEO_KHZ800);

const unsigned long repeatDelay = 500; // auto-repeat delay
const unsigned long repeatRate = 50;   // auto-repeat rate

void onKeyboard(hid_keyboard_report_t report, hid_keyboard_report_t last_report) {
	unsigned long currentTime = millis();
	if (report.modifier & 0x01){
		blekeyboard.press(KEY_LEFT_CTRL);
		Serial.print("Key pressed: ");
		Serial.println(KEY_LEFT_CTRL);
	}else{
		blekeyboard.release(KEY_LEFT_CTRL);
	};
	if (report.modifier & 0x02) {
		blekeyboard.press(KEY_LEFT_SHIFT);
	}
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
 void onMouse(hid_mouse_report_t report, uint8_t last_buttons) {
    uint8_t changed = report.buttons ^ last_buttons;
    if (changed & MOUSE_BUTTON_LEFT)   (report.buttons & MOUSE_BUTTON_LEFT)   ? blemouse.press(MOUSE_LEFT)   : blemouse.release(MOUSE_LEFT);
    if (changed & MOUSE_BUTTON_RIGHT)  (report.buttons & MOUSE_BUTTON_RIGHT)  ? blemouse.press(MOUSE_RIGHT)  : blemouse.release(MOUSE_RIGHT);
    if (changed & MOUSE_BUTTON_MIDDLE) (report.buttons & MOUSE_BUTTON_MIDDLE) ? blemouse.press(MOUSE_MIDDLE) : blemouse.release(MOUSE_MIDDLE);

    // Mouvement + molette (wheel sur 8 bits signé)
    int8_t dx = (int8_t)report.x;
    int8_t dy = (int8_t)report.y;
    int8_t wheel = (int8_t)report.wheel;
    if (dx != 0 || dy != 0 || wheel != 0) blemouse.move(dx, dy, wheel);
	//add debug info
	Serial.printf("Mouse Move: dx=%d, dy=%d, wheel=%d, buttons=%02X\n", dx, dy, wheel, report.buttons);
 }


void setup() {
	Serial.begin(115200);
	Serial.println("Starting USB Host...");
	usbhost.begin();
	blekeyboard.begin();
	blemouse.begin();
	usbhost.setHIDLocal(HID_LOCAL_French);
	usbhost.setKeyboardCallback(onKeyboard);
	usbhost.setMouseCallback(onMouse);
	Serial.println("Setup complete");
}

void loop() {
	// Send a message every 1 milliseconds
	static unsigned long lastLoop = 0;
	if (millis() - lastLoop > 1) {
		usbhost.task();
		lastLoop = millis();
	}
}
