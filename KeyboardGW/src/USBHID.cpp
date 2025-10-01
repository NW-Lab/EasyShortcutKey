#include "USBHID.h"

USBHIDClass USBHID;

#if defined(USE_USB_HID) && USE_USB_HID == 1

#include "tusb.h"
#include <string.h>

// HID report: 8 bytes: modifiers, reserved, 6 keycodes
struct __attribute__((packed)) KeyboardReport {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
};

// Convert an ASCII character to HID usage and optional Shift modifier.
// Returns true if mapped, false otherwise.
static bool asciiToUsage(char c, uint8_t* outUsage, uint8_t* outModifier) {
  *outModifier = 0;
  *outUsage = 0;
  if (c >= 'a' && c <= 'z') {
    *outUsage = 0x04 + (c - 'a');
    return true;
  }
  if (c >= 'A' && c <= 'Z') {
    *outUsage = 0x04 + (c - 'A');
    *outModifier = 0x02; // Shift
    return true;
  }
  if (c >= '1' && c <= '9') {
    *outUsage = 0x1E + (c - '1');
    return true;
  }
  if (c == '0') { *outUsage = 0x27; return true; }

  // Number-row symbols with shift
  switch (c) {
    case '!': *outUsage = 0x1E; *outModifier = 0x02; return true;
    case '@': *outUsage = 0x1F; *outModifier = 0x02; return true;
    case '#': *outUsage = 0x20; *outModifier = 0x02; return true;
    case '$': *outUsage = 0x21; *outModifier = 0x02; return true;
    case '%': *outUsage = 0x22; *outModifier = 0x02; return true;
    case '^': *outUsage = 0x23; *outModifier = 0x02; return true;
    case '&': *outUsage = 0x24; *outModifier = 0x02; return true;
    case '*': *outUsage = 0x25; *outModifier = 0x02; return true;
    case '(' : *outUsage = 0x26; *outModifier = 0x02; return true;
    case ')' : *outUsage = 0x27; *outModifier = 0x02; return true;
  }

  // Common punctuation and symbols
  switch (c) {
    case ' ' : *outUsage = 0x2C; return true;
    case '\n': case '\r': *outUsage = 0x28; return true; // Enter
    case '\b': *outUsage = 0x2A; return true; // Backspace
    case '\t': *outUsage = 0x2B; return true; // Tab
    case '-' : *outUsage = 0x2D; return true;
    case '_' : *outUsage = 0x2D; *outModifier = 0x02; return true;
    case '=' : *outUsage = 0x2E; return true;
    case '+' : *outUsage = 0x2E; *outModifier = 0x02; return true;
    case '[' : *outUsage = 0x2F; return true;
    case '{' : *outUsage = 0x2F; *outModifier = 0x02; return true;
    case ']' : *outUsage = 0x30; return true;
    case '}' : *outUsage = 0x30; *outModifier = 0x02; return true;
    case '\\': *outUsage = 0x31; return true;
    case '|' : *outUsage = 0x31; *outModifier = 0x02; return true;
    case ';' : *outUsage = 0x33; return true;
    case ':' : *outUsage = 0x33; *outModifier = 0x02; return true;
    case '\'' : *outUsage = 0x34; return true;
    case '"' : *outUsage = 0x34; *outModifier = 0x02; return true;
    case '`' : *outUsage = 0x35; return true;
    case '~' : *outUsage = 0x35; *outModifier = 0x02; return true;
    case ',' : *outUsage = 0x36; return true;
    case '<' : *outUsage = 0x36; *outModifier = 0x02; return true;
    case '.' : *outUsage = 0x37; return true;
    case '>' : *outUsage = 0x37; *outModifier = 0x02; return true;
    case '/' : *outUsage = 0x38; return true;
    case '?' : *outUsage = 0x38; *outModifier = 0x02; return true;
  }

  // arrows and some control words are not handled here when user requested literal typing
  return false;
}

void USBHIDClass::begin() {
  // TinyUSB initialized by core/USB.begin(); nothing here for now
}

void USBHIDClass::writeKeys(const char** keys, size_t count) {
  if (!tud_hid_ready()) return;

  // For each received token, type it literally as text: iterate chars
  for (size_t i = 0; i < count; ++i) {
    const char* s = keys[i];
    if (!s) continue;
    for (size_t j = 0; s[j] != '\0'; ++j) {
      uint8_t usage = 0;
      uint8_t mod = 0;
      if (!asciiToUsage(s[j], &usage, &mod)) continue; // skip unmapped chars

      KeyboardReport rpt;
      memset(&rpt, 0, sizeof(rpt));
      rpt.modifiers = mod;
      rpt.keys[0] = usage;
      // press
      tud_hid_report(0, &rpt, sizeof(rpt));
      delay(8);
      // release
      memset(&rpt, 0, sizeof(rpt));
      tud_hid_report(0, &rpt, sizeof(rpt));
      delay(6);
    }
    // small pause between tokens
    delay(10);
  }
}

void USBHIDClass::writeShortcut(const char** keys, size_t count) {
  if (!tud_hid_ready()) return;

  uint8_t modifiers = 0;
  uint8_t keyUsages[6] = {0}; // HID supports up to 6 simultaneous keys
  size_t keyIndex = 0;

  // Parse keys and build modifier + key combination
  for (size_t i = 0; i < count && keyIndex < 6; ++i) {
    const char* key = keys[i];
    if (!key) continue;

    // Check for modifier keys (case insensitive)
    if (strcasecmp(key, "cmd") == 0 || strcasecmp(key, "gui") == 0 || strcasecmp(key, "win") == 0) {
      modifiers |= 0x08; // Left GUI (Cmd/Win)
    } else if (strcasecmp(key, "ctrl") == 0 || strcasecmp(key, "control") == 0) {
      modifiers |= 0x01; // Left Ctrl
    } else if (strcasecmp(key, "alt") == 0 || strcasecmp(key, "option") == 0) {
      modifiers |= 0x04; // Left Alt
    } else if (strcasecmp(key, "shift") == 0) {
      modifiers |= 0x02; // Left Shift
    } else {
      // Check for special key names first
      if (strcasecmp(key, "space") == 0) {
        keyUsages[keyIndex++] = 0x2C; // Space key
      } else if (strcasecmp(key, "enter") == 0 || strcasecmp(key, "return") == 0) {
        keyUsages[keyIndex++] = 0x28; // Enter key
      } else if (strcasecmp(key, "tab") == 0) {
        keyUsages[keyIndex++] = 0x2B; // Tab key
      } else if (strcasecmp(key, "backspace") == 0) {
        keyUsages[keyIndex++] = 0x2A; // Backspace key
      } else if (strcasecmp(key, "delete") == 0) {
        keyUsages[keyIndex++] = 0x4C; // Delete key
      } else if (strcasecmp(key, "escape") == 0 || strcasecmp(key, "esc") == 0) {
        keyUsages[keyIndex++] = 0x29; // Escape key
      } else if (strlen(key) == 1) {
        // Regular single character key - convert to usage code
        uint8_t usage = 0;
        uint8_t mod = 0;
        // Convert to lowercase for consistent handling
        char lowercaseKey = tolower(key[0]);
        if (asciiToUsage(lowercaseKey, &usage, &mod)) {
          keyUsages[keyIndex++] = usage;
          // Don't add individual key modifiers for shortcuts - use only explicit modifiers
          // modifiers |= mod; 
        }
      }
    }
  }

  // Debug: print what we're about to send
  Serial.print("Sending shortcut - Modifiers: 0x");
  Serial.print(modifiers, HEX);
  Serial.print(", Keys: ");
  for (int i = 0; i < 6; i++) {
    if (keyUsages[i] != 0) {
      Serial.print("0x");
      Serial.print(keyUsages[i], HEX);
      Serial.print(" ");
    }
  }
  Serial.println();

  // Send the complete shortcut as one key combination
  KeyboardReport rpt;
  memset(&rpt, 0, sizeof(rpt));
  rpt.modifiers = modifiers;
  memcpy(rpt.keys, keyUsages, sizeof(keyUsages));
  
  // Press all keys
  tud_hid_report(0, &rpt, sizeof(rpt));
  delay(50); // Hold for a bit
  
  // Release all keys
  memset(&rpt, 0, sizeof(rpt));
  tud_hid_report(0, &rpt, sizeof(rpt));
}

#else

void USBHIDClass::begin() {
  // USB HID disabled at compile time; nothing to do.
}

void USBHIDClass::writeKeys(const char** keys, size_t count) {
  // Fallback: do nothing (Serial may be unavailable per user).
}

void USBHIDClass::writeShortcut(const char** keys, size_t count) {
  // Fallback: do nothing (USB HID disabled).
}

#endif

