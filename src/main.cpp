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
	static bool numlockState = false;
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
				// numlock off handling
				keys.push_back({key, currentTime});

				if (keycodes[key] == KEY_NUM_LOCK) {
						// check numlock state
						if ((last_report.modifier & KEY_NUM_LOCK) == 0 || (report.modifier & KEY_NUM_LOCK) != 0) {
							// CTRL key state changed
							numlockState = !numlockState;
							Serial.printf("Numlock state changed: %s\n", numlockState ? "ON" : "OFF");
						}
				} else if (numlockState == 0 && keycodes[key] >= KEY_NUM_SLASH && keycodes[key] <= KEY_NUM_9) {
					// simulate mouse movement with 2,4,6,8 arrow keys when num lock is off
					int8_t mouseX = 0;
					int8_t mouseY = 0;
					int8_t mouseIcrement = 10;
					//horizontal and vertical movement
					// 2 as down
					if (keycodes[key] == KEY_NUM_2) mouseY += mouseIcrement;
 					// 8 as up
					if (keycodes[key] == KEY_NUM_8) mouseY -= mouseIcrement;
					// 4 as left
					if (keycodes[key] == KEY_NUM_4) mouseX -= mouseIcrement;
					// 6 as right
					if (keycodes[key] == KEY_NUM_6) mouseX += mouseIcrement;
					// diagonal movement
					// 1 as down-left
					if (keycodes[key] == KEY_NUM_1) {
						mouseY += mouseIcrement;
						mouseX -= mouseIcrement;
					}
					// 3 as down-right
					if (keycodes[key] == KEY_NUM_3) {
						mouseY += mouseIcrement;
						mouseX += mouseIcrement;
					}
					// 7 as up-left
					if (keycodes[key] == KEY_NUM_7) {
						mouseY -= mouseIcrement;
						mouseX -= mouseIcrement;
					}
					// 9 as up-right
					if (keycodes[key] == KEY_NUM_9) {
						mouseY -= mouseIcrement;
						mouseX += mouseIcrement;
					}
					// num 5 as right click
					if (keycodes[key] == KEY_NUM_5) {
						blemouse.click(MOUSE_LEFT);
						Serial.println("Mouse left Click");
					}
					if (mouseX != 0 || mouseY != 0) {
						blemouse.move(mouseX, mouseY);
						Serial.printf("Mouse Move: dx=%d, dy=%d\n", mouseX, mouseY);
					}
				} else {
					// New key press	
					blekeyboard.press(keycodes[key]);
					Serial.printf("Keyboard Key: %d\n", keycodes[key]);
				}

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

	case 2: { // Wheel (vertical)
		if (n >= 1) {
		int8_t wheel = (int8_t)p[0];
		printf("Wheel: %d\n", wheel);
		}
	} break;

	default:
		dump_report(rpt, len);
		break;
	}
}
// Mouse callback
 void onMouse(hid_mouse_report_t report, uint8_t last_buttons) {
	uint8_t changed = report.buttons ^ last_buttons;
	parse_mouse_report((const uint8_t*)&report, sizeof(report));
	// Mouse movement and wheel
	int8_t dx = (int8_t)report.x;
	int8_t dy = (int8_t)report.y;
	int8_t wheel = (int8_t)report.wheel;
	if (dx != 0 || dy != 0 || wheel != 0) blemouse.move(dx, dy, wheel);
	// Buttons
	if (changed & MOUSE_BUTTON_LEFT)   (report.buttons & MOUSE_BUTTON_LEFT)   ? blemouse.press(MOUSE_LEFT)   : blemouse.release(MOUSE_LEFT);
	if (changed & MOUSE_BUTTON_RIGHT)  (report.buttons & MOUSE_BUTTON_RIGHT)  ? blemouse.press(MOUSE_RIGHT)  : blemouse.release(MOUSE_RIGHT);
	if (changed & MOUSE_BUTTON_MIDDLE) (report.buttons & MOUSE_BUTTON_MIDDLE) ? blemouse.press(MOUSE_MIDDLE) : blemouse.release(MOUSE_MIDDLE);
 }

void usbhostTask(void *pvParameters) {
	while (1) {
		// Run USB host task
		usbhost.task();
		vTaskDelay(1 / portTICK_PERIOD_MS); // delay to allow other tasks to run
	}
}
void setup() {
	Serial.println("Setup strarting...");
	// Initialize Serial for debugging
	Serial.begin(115200);
	// Initialize BLE Keyboard and USB Host
	usbhost.begin(); Serial.println("- Starting USB Host done");
	blekeyboard.begin(); Serial.println("- Starting BLE Keyboard done");
	blemouse.begin(); Serial.println("- Starting BLE Keyboard done");
	// Set HID locale to International
	usbhost.setHIDLocal(HID_LOCAL_International);
	// Set keyboard and mouse callbacks
	usbhost.setKeyboardCallback(onKeyboard); Serial.println("--- set KeyboardCallback done");
	usbhost.setMouseCallback(onMouse); Serial.println("--- set MouseCallback done");
	// Create USB host task
	xTaskCreatePinnedToCore(usbhostTask, "usbhostTask", 4096, NULL, 1, NULL, 1); Serial.println("- Creating usbhostTask done");
	Serial.println("Setup complete");
}

void loop() {
	// nothing to do here, all handled in usbhostTask
}
