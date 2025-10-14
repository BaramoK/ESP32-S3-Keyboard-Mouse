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
				Serial.print("Key pressed: "); Serial.println(keycodes[key]);
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
	
	blemouse.move(report.x, report.y, report.wheel, report.pan);
	if (report.buttons & 0x01) blemouse.press(MOUSE_LEFT);
	else blemouse.release(MOUSE_LEFT);
	if (report.buttons & 0x02) blemouse.press(MOUSE_RIGHT);
	else blemouse.release(MOUSE_RIGHT);
	if (report.buttons & 0x04) blemouse.press(MOUSE_MIDDLE);
	else blemouse.release(MOUSE_MIDDLE);
	if (report.buttons & 0x08) blemouse.press(MOUSE_BACK);
	else blemouse.release(MOUSE_BACK);
	if (report.buttons & 0x10) blemouse.press(MOUSE_FORWARD);
	else blemouse.release(MOUSE_FORWARD);
	Serial.print("Mouse: "); Serial.print("X="); Serial.print(report.x); Serial.print(" Y="); Serial.println(report.y);		
}

void setup() {
	Serial.begin(115200);
	blekeyboard.begin();
	blemouse.begin();
	Serial.println("Starting USB Host...");
	usbhost.begin();
	usbhost.setHIDLocal(HID_LOCAL_French);
	usbhost.setKeyboardCallback(onKeyboard);
	usbhost.setMouseCallback(onMouse);
	Serial.println("Setup complete");
}

void loop() {
	usbhost.task();
	// key auto-repeat handling
	unsigned long currentTime = millis();
	for (const auto& keypress : keys) {
		if (currentTime - keypress.pressTime >= repeatDelay) {
			if ((currentTime - keypress.pressTime - repeatDelay) % repeatRate == 0) {
				blekeyboard.press(keycodes[keypress.key]);
			}
		}
	}
	delay(1);
}
