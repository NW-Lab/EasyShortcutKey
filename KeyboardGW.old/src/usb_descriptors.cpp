// USB descriptors and TinyUSB configuration for CDC + HID composite device
#include "tusb.h"

// Arduino-ESP32 TinyUSB integration requires these specific callback names
// to override the default descriptors

enum
{
  ITF_NUM_HID = 0,
};

#define ITF_NUM_TOTAL 1
#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

// Export descriptors and callbacks with C linkage so esp-idf component
// can consume them when installing the tinyusb driver.
extern "C" {

// HID report descriptor (keyboard)
// Use the standard keyboard report descriptor without injecting
// an unexpected report-id value.
const uint8_t hid_report_descriptor[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(),
};

// Device descriptor (composite device)
const tusb_desc_device_t descriptor_device = {
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,
  .bcdUSB = 0x0200,
  .bDeviceClass = TUSB_CLASS_MISC,
  .bDeviceSubClass = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol = MISC_PROTOCOL_IAD,
  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor = 0x303A, // Espressif
  .idProduct = 0x0002, // Custom PID for HID+CDC
  .bcdDevice = 0x0100,
  .iManufacturer = 0x01,
  .iProduct = 0x02,
  .iSerialNumber = 0x03,
  .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR request
// Must override Arduino-ESP32 default with __attribute__((weak))
uint8_t const* tud_descriptor_device_cb(void) __attribute__((weak));
uint8_t const* tud_descriptor_device_cb(void)
{
  return (uint8_t const*)&descriptor_device;
}

// Invoked when received GET HID REPORT DESCRIPTOR request
uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance)
{
  (void)instance;
  return hid_report_descriptor;
}

// Configuration descriptor: composite device with CDC and HID
const uint8_t configuration_descriptor[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // HID interface - keyboard protocol (single interface)
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 4, HID_ITF_PROTOCOL_KEYBOARD, sizeof(hid_report_descriptor), EPNUM_HID, 8, 10),
};

// String descriptors
const char* string_descriptor[] = {
  (char[]){0x09, 0x04}, // supported language: English
  "NW-Lab",            // Manufacturer
  "EasyShortcutKey GW",// Product
  "",                  // Serial (leave empty, stack may fill)
  "HID Interface",     // HID Interface
};

// Export string count so C users can determine the array length
const int descriptor_string_count = (int)(sizeof(string_descriptor) / sizeof(string_descriptor[0]));

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
  // Not implementing feature reports
  return 0;
}

// Invoked when received SET_REPORT control request or received data on OUT endpoint
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
  // Nothing to do for now
}

// Note: in Arduino/PlatformIO environment we rely on the built-in TinyUSB
// integration; explicit IDF tinyusb_config_t / tusb_cdc_acm types are
// intentionally NOT referenced here so this file can compile with the
// Arduino TinyUSB integration that the framework provides.

// Provide the descriptors and the minimal tud_* callbacks required by
// the TinyUSB device stack. The Arduino core will pick up these
// callbacks when building the device.

// Return pointer to configuration descriptor
// TinyUSB core will call this to obtain the configuration we defined above.
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) __attribute__((weak));
uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index;
  return configuration_descriptor;
}

// String descriptor helper. TinyUSB core will call this to obtain
// UTF-16LE string descriptors. This implementation mirrors examples
// from TinyUSB and uses the `string_descriptor[]` table above.
const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) __attribute__((weak));
const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  // Static buffer for returned UTF-16 string (max 31 chars)
  static uint16_t desc_str[32];

  if (index == 0) {
    // Supported language: English (0x0409)
    static uint16_t langid[] = {0x0409};
    return langid;
  }

  if (index >= (sizeof(string_descriptor) / sizeof(string_descriptor[0]))) {
    return NULL;
  }

  const char* str = string_descriptor[index];

  // Convert ASCII string into UTF-16 (le)
  uint8_t chr_count = 0;
  while (str[chr_count] && chr_count < 31) chr_count++;

  // first uint16_t is descriptor length in bytes (including this field)
  desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  for (uint8_t i = 0; i < chr_count; i++) {
    desc_str[1 + i] = (uint16_t) str[i];
  }

  return desc_str;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks (additional ones - avoid duplicates)
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  // Optional: Add mount handling
}

// Invoked when device is unmounted  
void tud_umount_cb(void) {
  // Optional: Add unmount handling
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  // Optional: Add suspend handling
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  // Optional: Add resume handling
}

// Invoked when CDC line state changed
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf; (void) dtr; (void) rts;
  // Optional: Handle line state changes
}

// Invoked when CDC line coding is changed via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
  (void) itf; (void) p_line_coding;
  // Optional: Handle line coding changes
}

// Invoked when received new data
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
  // Optional: Handle incoming data
}

// End C linkage block
} 
