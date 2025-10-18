#include "EspUsbHost.h"

void EspUsbHost::begin(void) {
	usbTransferSize = 0;
	const usb_host_config_t config = {
		.skip_phy_setup = false,
		.intr_flags = ESP_INTR_FLAG_LEVEL1,
	};

	esp_err_t result = usb_host_install(&config);
	if (result != ESP_OK) {
		Serial.println("usb_host_install() error" + String(result));
		return;
	}

	const usb_host_client_config_t client_config = {
		.is_synchronous = true,
		.max_num_event_msg = 10,
		.async = {
			.client_event_callback = this->_clientEventCallback,
			.callback_arg = this,
		}
	};
	usb_host_client_register(&client_config, &this->clientHandle);
}

void EspUsbHost::_clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg) {
	EspUsbHost *usbHost = (EspUsbHost *)arg;
	esp_err_t result;
	switch (eventMsg->event) {
		case USB_HOST_CLIENT_EVENT_NEW_DEV: {
			Serial.println("USB_HOST_CLIENT_EVENT_NEW_DEV new_dev.address = " + String(eventMsg->new_dev.address));
			result = usb_host_device_open(usbHost->clientHandle, eventMsg->new_dev.address, &usbHost->deviceHandle);
			if (result != ESP_OK) {
				Serial.println("usb_host_device_open() error = " + String(result));
				return;
			}

			usb_device_info_t dev_info;
			result = usb_host_device_info(usbHost->deviceHandle, &dev_info);
			if (result != ESP_OK) {
				Serial.println("usb_host_device_info() error = " + String(result));
				return;
			}

			const usb_device_desc_t *dev_desc;
			result = usb_host_get_device_descriptor(usbHost->deviceHandle, &dev_desc);
			if (result != ESP_OK) {
				Serial.println("usb_host_get_device_descriptor() error = " + String(result));
				return;
			}

			const uint8_t requestdevice[8] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x12, 0x00 };
			Serial.println("GET DESCRIPTOR Request DEVICE");
			Serial.println("GET DESCRIPTOR Response DEVICE bcdUSB = " + String(dev_desc->bcdUSB, HEX));

			const usb_config_desc_t *config_desc;
			result = usb_host_get_active_config_descriptor(usbHost->deviceHandle, &config_desc);
			if (result != ESP_OK) {
				Serial.println("usb_host_get_active_config_descriptor() error = " + String(result));
				return;
			}

			const uint8_t requestconfig[8] = { 0x80, 0x06, 0x00, 0x02, 0x00, 0x00, 0x09, 0x00 };
			Serial.println("GET DESCRIPTOR Request CONFIGURATION");
			Serial.println("GET DESCRIPTOR Response CONFIGURATION");
			usbHost->_configCallback(config_desc);
			break;
		}
		case USB_HOST_CLIENT_EVENT_DEV_GONE: {
			Serial.println("USB_HOST_CLIENT_EVENT_DEV_GONE");
			for (uint8_t i = 0; i < usbHost->usbTransferSize; i++) {
				if (!usbHost->usbTransfer[i]) {
					continue;
				}

				usb_host_endpoint_clear(eventMsg->dev_gone.dev_hdl, usbHost->usbTransfer[i]->bEndpointAddress);
				usb_host_transfer_free(usbHost->usbTransfer[i]);
				usbHost->usbTransfer[i] = NULL;
			}

			usbHost->usbTransferSize = 0;
			for (uint8_t i = 0; i < usbHost->usbInterfaceSize; i++) {
				usb_host_interface_release(usbHost->clientHandle, usbHost->deviceHandle, usbHost->usbInterface[i]);
				usbHost->usbInterface[i] = 0;
			}

			usbHost->usbInterfaceSize = 0;
			usb_host_device_close(usbHost->clientHandle, usbHost->deviceHandle);
			usbHost->onGone(eventMsg);
			break;
		}
	}
}

void EspUsbHost::_configCallback(const usb_config_desc_t *config_desc) {
	const uint8_t *p = &config_desc->val[0];
	uint8_t bLength;
	const uint8_t setup[8] = { 0x80, 0x06, 0x00, 0x02, 0x00, 0x00, (uint8_t)config_desc->wTotalLength, 0x00 };
	Serial.println("GET DESCRIPTOR Request CONFIGURATION");
	Serial.println("GET DESCRIPTOR Response CONFIGURATION");
	for (uint16_t i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength) {
		bLength = *p;
		if ((i + bLength) >= config_desc->wTotalLength) {
			return;
		}

		const uint8_t bDescriptorType = *(p + 1);
		this->onConfig(bDescriptorType, p);
	}
}

void EspUsbHost::task(void) {
	usb_host_lib_handle_events(1, &this->eventFlags);
	usb_host_client_handle_events(this->clientHandle, 1);
	if (!this->isReady) {
		return;
	}

	unsigned long now = millis();
	if ((now - this->lastCheck) > this->interval) {
		this->lastCheck = now;

		for (uint8_t i = 0; i < this->usbTransferSize; i++) {
			if (!this->usbTransfer[i]) {
				continue;
			}

			usb_host_transfer_submit(this->usbTransfer[i]);
		}
	}
}

String EspUsbHost::getUsbDescString(const usb_str_desc_t *str_desc) {
	String str = "";
	if (!str_desc) {
		return str;
	}

	for (int i = 0; i < str_desc->bLength / 2; i++) {
		if (str_desc->wData[i] > 0xFF) {
			continue;
		}
		str += char(str_desc->wData[i]);
	}
	return str;
}

void EspUsbHost::onConfig(const uint8_t bDescriptorType, const uint8_t *p) {
	switch (bDescriptorType) {
		case USB_INTERFACE_DESC: {
			const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;
			this->claim_err = usb_host_interface_claim(this->clientHandle, this->deviceHandle, intf->bInterfaceNumber, intf->bAlternateSetting);
			if (this->claim_err != ESP_OK) {
				Serial.println("usb_host_interface_claim() error" + String(claim_err));
				return;
			}

			this->usbInterface[this->usbInterfaceSize] = intf->bInterfaceNumber;
			this->usbInterfaceSize++;
			_bInterfaceClass = intf->bInterfaceClass;
			_bInterfaceSubClass = intf->bInterfaceSubClass;
			_bInterfaceProtocol = intf->bInterfaceProtocol;
			break;
		}
		case USB_ENDPOINT_DESC: {
			const usb_ep_desc_t *ep_desc = (const usb_ep_desc_t *)p;
			if (this->claim_err != ESP_OK) {
				Serial.println("usb_host_interface_claim() error" + String(claim_err));
				return;
			}

			this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceClass = _bInterfaceClass;
			this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceSubClass = _bInterfaceSubClass;
			this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceProtocol = _bInterfaceProtocol;
			this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bCountryCode = _bCountryCode;

			if ((ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT) {
				Serial.println("err ep_desc->bmAttributes=" + String(ep_desc->bmAttributes));
				return;
			}

			if (ep_desc->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
				esp_err_t err = usb_host_transfer_alloc(ep_desc->wMaxPacketSize + 1, 0, &this->usbTransfer[this->usbTransferSize]);
				if (err != ESP_OK) {
					this->usbTransfer[this->usbTransferSize] = NULL;
					Serial.println("usb_host_transfer_alloc() error" + String(err));
					return;
				}

				this->usbTransfer[this->usbTransferSize]->device_handle = this->deviceHandle;
				this->usbTransfer[this->usbTransferSize]->bEndpointAddress = ep_desc->bEndpointAddress;
				this->usbTransfer[this->usbTransferSize]->callback = this->_onReceive;
				this->usbTransfer[this->usbTransferSize]->context = this;
				this->usbTransfer[this->usbTransferSize]->num_bytes = ep_desc->wMaxPacketSize;
				interval = ep_desc->bInterval;
				isReady = true;
				this->usbTransferSize++;
			}
			break;
		}
		case USB_HID_DESC: {
			const tusb_hid_descriptor_hid_t *hid_desc = (const tusb_hid_descriptor_hid_t *)p;
			_bCountryCode = hid_desc->bCountryCode;
			submitControl(0x81, 0x00, 0x22, 0x0000, 136);
			break;
		}
	}
}
// Modificaiton lecture souris 12 bits X/Y
static inline int16_t s12(int v) {
  v &= 0x0FFF;
  if (v & 0x0800) v -= 0x1000;
  return (int16_t)v;
}
// Accumulateurs globaux ou static (conservent la fraction perdue entre rapports)
static int32_t acc_x = 0;
static int32_t acc_y = 0;

static inline int16_t s12(uint16_t v) {
  v &= 0x0FFF;
  if (v & 0x0800) v -= 0x1000;
  return (int16_t)v;
}

// Émet autant de rapports int8 que nécessaire pour ne pas saturer
static void emit_mouse_chunks(EspUsbHost* usbHost, int32_t* ax, int32_t* ay, uint8_t buttons, uint8_t* plast_buttons) {
  while (*ax || *ay) {
    int8_t sx = 0, sy = 0;

    if (*ax > 127)       { sx = 127;  *ax -= 127; }
    else if (*ax < -127) { sx = -127; *ax += 127; }
    else                 { sx = (int8_t)*ax; *ax = 0; }

    if (*ay > 127)       { sy = 127;  *ay -= 127; }
    else if (*ay < -127) { sy = -127; *ay += 127; }
    else                 { sy = (int8_t)*ay; *ay = 0; }

    hid_mouse_report_t rpt = {};
    rpt.buttons = buttons;
    rpt.x = sx;
    rpt.y = sy;
    rpt.wheel = 0;
    usbHost->mouseCallback(rpt, *plast_buttons);
    *plast_buttons = rpt.buttons;
  }
}

void EspUsbHost::_onReceive(usb_transfer_t *transfer) {
  EspUsbHost *usbHost = static_cast<EspUsbHost *>(transfer->context);
  endpoint_data_t *endpoint_data = &usbHost->endpoint_data_list[(transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_NUM_MASK)];

  // On ne traite que HID
  if (endpoint_data->bInterfaceClass != USB_CLASS_HID) {
    usbHost->onReceive(transfer);
    return;
  }

  const uint8_t *buf = transfer->data_buffer;
  size_t n = transfer->actual_num_bytes; // IMPORTANT: longueur réelle du rapport

  if (endpoint_data->bInterfaceProtocol == HID_ITF_PROTOCOL_KEYBOARD) {
    // --- Chemin clavier inchangé (boot keyboard) ---
    static hid_keyboard_report_t last_report = {};
    if (n >= sizeof(hid_keyboard_report_t)) {
      hid_keyboard_report_t report = {};
      memcpy(&report, buf, sizeof(report));
      if (memcmp(&last_report, &report, sizeof(report))) {
        usbHost->keyboardCallback(report, last_report);

        // LEDs (comme avant)
        uint8_t keycode = report.keycode[0];
        uint8_t led = 0;
        switch (keycode) {
          case HID_KEY_NUM_LOCK:    led |= KEYBOARD_LED_NUMLOCK;   break;
          case HID_KEY_CAPS_LOCK:   led |= KEYBOARD_LED_CAPSLOCK;  break;
          case HID_KEY_SCROLL_LOCK: led |= KEYBOARD_LED_SCROLLLOCK;break;
        }
        if (led) usbHost->sendKeyboardLeds(led);

        memcpy(&last_report, &report, sizeof(report));
      }
    }
  } else {
  // --- Chemin HID générique / souris ---
  static uint8_t last_buttons = 0;
  const uint8_t *buf = transfer->data_buffer;
  size_t n = transfer->actual_num_bytes; // ou actual_length selon ta version

  if (endpoint_data->bInterfaceProtocol == HID_ITF_PROTOCOL_MOUSE ||
      endpoint_data->bInterfaceProtocol == 0 /* report protocol */) {

    int16_t dx = 0, dy = 0;
    uint8_t buttons = 0;
    bool parsed = false;

   	// A) Ton cas “5 octets” (corrigé: décalage d'1 octet)
	if (n == 5) {
	uint8_t b0 = buf[0], b1 = buf[1], b2 = buf[2], b3 = buf[3]; // b4 = buf[4] inutilisé

	auto s12 = [](uint16_t v)->int16_t {
		v &= 0x0FFF;
		if (v & 0x0800) v -= 0x1000;
		return (int16_t)v;
	};

	buttons = b0;
	int16_t x12 = s12( (uint16_t)b1 | ((uint16_t)(b2 & 0x0F) << 8) );
	int16_t y12 = s12( (uint16_t)(b2 >> 4) | ((uint16_t)b3 << 4) );

	dx = x12;
	dy = y12;
	parsed = true;
	}


    // B) Rapports avec ReportID (ID=1: XY/boutons, ID=2: wheel)
    if (!parsed && n >= 2) {
      uint8_t rid = buf[0];
      if (rid == 1 && n >= 4) {
        buttons = buf[1];
        dx = (int8_t)buf[2];
        dy = (int8_t)buf[3];
        parsed = true;
      } else if (rid == 2 && n >= 2) {
        // Molette seule
        hid_mouse_report_t rpt = {};
        rpt.buttons = last_buttons;
        rpt.x = 0; rpt.y = 0;
        rpt.wheel = (int8_t)buf[1];
        usbHost->mouseCallback(rpt, last_buttons);
        last_buttons = rpt.buttons;
        // Pas de dx/dy à accumuler
      }
    }

    // C) Fallback boot
    if (!parsed && n >= 3) {
      buttons = buf[0];
      dx = (int8_t)buf[1];
      dy = (int8_t)buf[2];
      parsed = true;
    }

    // Accumulation + émission en chunks
    if (parsed) {
      acc_x += dx;
      acc_y += dy;
      emit_mouse_chunks(usbHost, &acc_x, &acc_y, buttons, &last_buttons);
    }
  }
}

  usbHost->onReceive(transfer);
}


void EspUsbHost::setHIDLocal(hid_local_enum_t code) {
	hidLocal = code;
}

esp_err_t EspUsbHost::submitControl(const uint8_t bmRequestType, const uint8_t bDescriptorIndex, const uint8_t bDescriptorType, const uint16_t wInterfaceNumber, const uint16_t wDescriptorLength) {
	usb_transfer_t *transfer;
	usb_host_transfer_alloc(wDescriptorLength + 9, 0, &transfer);
	transfer->num_bytes = wDescriptorLength + 8;
	transfer->data_buffer[0] = bmRequestType;
	transfer->data_buffer[1] = 0x06;
	transfer->data_buffer[2] = bDescriptorIndex;
	transfer->data_buffer[3] = bDescriptorType;
	transfer->data_buffer[4] = wInterfaceNumber & 0xff;
	transfer->data_buffer[5] = wInterfaceNumber >> 8;
	transfer->data_buffer[6] = wDescriptorLength & 0xff;
	transfer->data_buffer[7] = wDescriptorLength >> 8;
	transfer->device_handle = deviceHandle;
	transfer->bEndpointAddress = 0x00;
	transfer->callback = _onReceiveControl;
	transfer->context = this;
	if (bmRequestType == 0x81 && bDescriptorIndex == 0x00 && bDescriptorType == 0x22) {
		Serial.println("GET DESCRIPTOR Request HID Report");
	}

	esp_err_t result = usb_host_transfer_submit_control(clientHandle, transfer);
	return result;
}

void EspUsbHost::_onReceiveControl(usb_transfer_t *transfer) {
	EspUsbHost *usbHost = (EspUsbHost *)transfer->context;
	usb_host_transfer_free(transfer);
}

void EspUsbHost::setKeyboardCallback(keyboard_callback callback) {
	this->keyboardCallback = callback;
}

void EspUsbHost::setMouseCallback(mouse_callback callback) {
	this->mouseCallback = callback;
}

void EspUsbHost::sendKeyboardLeds(uint8_t led) {
	keyboard_leds ^= led;
	usb_transfer_t *transfer;
	esp_err_t transferResult = usb_host_transfer_alloc(9, 0, &transfer);
	if (transferResult != ESP_OK) {
		Serial.println("usb_host_transfer_alloc() error " + String(transferResult));
		return;
	}

	transfer->num_bytes = 9;
	transfer->data_buffer[0] = 0x21; // bmRequestType
	transfer->data_buffer[1] = 0x09; // bRequest (SetReport)
	transfer->data_buffer[2] = 0x00; // wValue (Report ID)
	transfer->data_buffer[3] = 0x02; // wValue (Report Type)
	transfer->data_buffer[4] = 0x00; // wIndex (Interface number, low byte)
	transfer->data_buffer[5] = 0x00; // wIndex (Interface number, high byte)
	transfer->data_buffer[6] = 0x01; // wLength (Data length, low byte)
	transfer->data_buffer[7] = 0x00; // wLength (Data length, high byte)
	transfer->data_buffer[8] = keyboard_leds; // Data stage (LED state bitfield)
	transfer->device_handle = deviceHandle;
	transfer->bEndpointAddress = 0x00; // Control endpoint
	transfer->callback = _onReceiveControl;
	transfer->context = this;
	usb_host_transfer_submit_control(clientHandle, transfer);
}
