#include <lvgl.h>
#include "Functions.h"
#include "RCSwitch.h"
#include "pin_config.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>


String lastProtocol = "";
String lastAddress = "";
String lastCommand = "";
String lastRawSignal = "";

String protocol = typeToString((decode_type_t)results.decode_type, false);
String address = String(results.value >> 16, HEX);    // Extract the address
String command = String(results.value & 0xFFFF, HEX); // Extract the command

lv_obj_t *spectrum_container; // Global variable for the spectrum container
lv_obj_t *spectrum_line;      // Global variable for the spectrum line
lv_obj_t *tabview;
lv_obj_t *tab_config;  // Global variable for Config tab
lv_obj_t *popup;       // Global variable for the popup dialog
lv_obj_t *tab_sd;
lv_obj_t *dropdown_frequency;  // Global variable for frequency dropdown
lv_obj_t *dropdown_modulation; // Global variable for modulation dropdown
lv_obj_t *dropdown_power; // Global variable for modulation dropdown
lv_obj_t *btn_power;
#define NUM_POINTS 50  // Number of points in the line (adjust for smoother/faster animation)
lv_obj_t *heartbeat_line;  // Line object
lv_point_t heartbeat_points[NUM_POINTS];  // Points array for the heartbeat line
int heartbeat_offset = 0; 


int button_ids[] = {1, 2, 3, 4, 5, 6};  // Unique IDs for buttons

/////RECIEVE
#define SAMPLESIZE 500
// Variables for receiving
static int interruptPin = 43;
int state = 0;
int counter = 0;
static unsigned long resetTime;
bool Receive = 0;
int MODE = 2;
String Print;
static unsigned int timings[SAMPLESIZE];
lv_obj_t *results_label; // Label to display raw captured data
static uint8_t raw_signal_buffer[SAMPLESIZE];
static size_t raw_signal_length = 0;


////DEBRUIJIN
String debruijn_ten = "0000000000100000000110000000101000000011100000010010000001011000000110100000011110000010001000001001100000101010000010111000001100100000110110000011101000001111100001000010001100001001010000100111000010100100001010110000101101000010111100001100010000110011000011010100001101110000111001000011101100001111010000111111000100010100010001110001001001000100101100010011010001001111000101001100010101010001010111000101100100010110110001011101000101111100011000110010100011001110001101001000110101100011011010001101111000111001100011101010001110111000111100100011110110001111101000111111100100100110010010101001001011100100110110010011101001001111100101001010011100101010110010101101001010111100101100110010110101001011011100101110110010111101001011111100110011010011001111001101010100110101110011011011001101110100110111110011100111010110011101101001110111100111101010011110111001111101100111111010011111111010101010111010101101101010111110101101011011110101110111010111101101011111110110110111011011111101110111110111101111111111000000000";
String debruijn_nine = "0000000001000000011000000101000000111000001001000001011000001101000001111000010001000010011000010101000010111000011001000011011000011101000011111000100011000100101000100111000101001000101011000101101000101111000110011000110101000110111000111001000111011000111101000111111001001001011001001101001001111001010011001010101001010111001011011001011101001011111001100111001101011001101101001101111001110101001110111001111011001111101001111111010101011010101111010110111010111011010111111011011011111011101111011111111100000000";
String debruijn_eight = "00000000100000011000001010000011100001001000010110000110100001111000100010011000101010001011100011001000110110001110100011111001001010010011100101011001011010010111100110011010100110111001110110011110100111111010101011101011011010111110110111101110111111110000000";
///TESLA
const uint16_t pulseWidth = 400;                     // Mikroseconds
const uint16_t messageDistance = 23;                 // Milliseconds
const uint8_t transmissions = 5;                     // Repeat 5 tinmes
const uint8_t sequence[] = { 
  0x02,0xAA,0xAA,0xAA,                               // Preamble 26 bits by repeating 1010
  0x2B,                                              // Sync byte
  0x2C,0xCB,0x33,0x33,0x2D,0x34,0xB5,0x2B,0x4D,0x32,0xAD,0x2C,0x56,0x59,0x96,0x66,
  0x66,0x5A,0x69,0x6A,0x56,0x9A,0x65,0x5A,0x58,0xAC,0xB3,0x2C,0xCC,0xCC,0xB4,0xD2,
  0xD4,0xAD,0x34,0xCA,0xB4,0xA0
};
const uint8_t messageLength = sizeof(sequence);
bool isJamloopActive = false;
volatile bool showPopupFlag = false;
bool isIRListening = false;

int receivedValue = 0;
int receivedBitLength = 0;
int receivedProtocol = 0;
int repeatDebru = 5;

// Define a uniform size for all buttons
#define BUTTON_WIDTH 88
#define BUTTON_HEIGHT 32

void send_action(lv_event_t *e) {
    int *button_id = (int *)lv_event_get_user_data(e);
    LV_LOG_USER("Send button pressed!");
    
    // Simulate a spike in the heartbeat
    int spike_index = NUM_POINTS / 2;  // Position of the spike in the waveform
    heartbeat_points[spike_index].y = 10;  // Create the upward spike (lower y is higher)
    lv_obj_invalidate(heartbeat_line);  // Refresh the line to apply changes
    switch (*button_id) {
        case 1:
            LV_LOG_USER("Send action for Test1!");
            jeepset();
            jeepsend();
            break;
        case 2:
            LV_LOG_USER("Send action for Test2!");
            SendTesla();
            break;
        case 3:
            LV_LOG_USER("Send action for Test3!");
            
            break;
        case 5:
            LV_LOG_USER("Send action for Test5!");
            handleDebruiSend(repeatDebru);
            break;
        default:
            LV_LOG_USER("Unknown button!");
    }
}

void exit_action(lv_event_t *e) {
    // Action for the "Exit" button (close the popup)
    LV_LOG_USER("Exit button pressed!");
    lv_obj_del(popup);  // Close the popup after "Exit" is pressed
}

/*// Function to initialize the SD card
bool initSDCard() {
  if (!SD.begin()) {
    Serial.println("SD Card initialization failed!");
    return false;
  }
  Serial.println("SD Card initialized.");
  return true;
}

// Function to populate the SD content list
void showSDContents() {
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open SD card!");
    return;
  }

  // Iterate over files and list them in the dropdown
  lv_obj_t *dropdown = lv_dropdown_create(tab_sd);
  lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, 10, 50); // Adjust position as needed

  String fileList = "";
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }

    if (entry.isDirectory()) {
      // Add directory names to the list
      fileList += entry.name();
    } else {
      // Add file names to the list
      fileList += entry.name();
    }

    fileList += "\n"; // Separate entries by newline
  }

  // Set the dropdown options with the file list
  lv_dropdown_set_options(dropdown, fileList.c_str());

  root.close();
}

// Button event handler for the SD content button
static void btn_sd_event_handler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // On button click, display SD card contents
    showSDContents();
  }
}
*/
void toggle_portal_event_handler(lv_event_t *e) {
  lv_obj_t *toggle = lv_event_get_target(e); // Get the toggle switch object
  uint32_t state = lv_obj_get_state(toggle);  // Get the state of the switch

  // Check if the switch is on (LV_STATE_CHECKED)
  if (state & LV_STATE_CHECKED) {
      // Call the Portal function if switch is on
  }
}
// Event handler for Ja toggle
void toggle_ja_event_handler(lv_event_t *e) {
  lv_obj_t *toggle = lv_event_get_target(e);  // Get the toggle switch object
  bool state = lv_obj_get_state(toggle);   // Get the state of the switch

  if (state) {
    // If the toggle is ON, set isTestloopActive to true
    isJamloopActive = true;
    Serial.println("Test loop activated.");
  } else {
    // If the toggle is OFF, set isTestloopActive to false
    isJamloopActive = false;
    Serial.println("Test loop deactivated.");
  }
}

void frequency_dropdown_event_handler(lv_event_t *e) {
    lv_obj_t *dropdown = lv_event_get_target(e);  // Get the dropdown object
    uint16_t selected = lv_dropdown_get_selected(dropdown);  // Get the selected option index

    // Set frequency based on selection
    switch (selected) {
        case 0:  // "315 MHz" selected
            ELECHOUSE_cc1101.setMHZ(freq_1);  // First module set to 315 MHz
            currentFrequency = freq_1;
            ELECHOUSE_cc1101.setPA(12);
            ELECHOUSE_cc1101.Init();
            mySwitch.enableTransmit(gdo0);
            ELECHOUSE_cc1101.SetTx();
            Serial.println("Frequency set to 315 MHz");
            break;
        case 1:  // "433.92 MHz" selected
            ELECHOUSE_cc1101.setMHZ(freq_2);  // First module set to 433 MHz
            currentFrequency = freq_2;
            ELECHOUSE_cc1101.setPA(12);
            ELECHOUSE_cc1101.Init();
            mySwitch.enableTransmit(gdo0);
            ELECHOUSE_cc1101.SetTx();
            Serial.println("Frequency set to 433.92 MHz");
            break;
        case 2:  // "868.35 MHz" selected
            ELECHOUSE_cc1101.setMHZ(freq_3);  // First module set to 433 MHz
            currentFrequency = freq_3;
            ELECHOUSE_cc1101.setPA(12);
            ELECHOUSE_cc1101.Init();
            mySwitch.enableTransmit(gdo0);
            ELECHOUSE_cc1101.SetTx();
            Serial.println("Frequency set to 868.35 MHz");
            break;
    }
}

void createDropdowns() {
  // Create the Frequency dropdown in the Config tab
  dropdown_frequency = lv_dropdown_create(tab_config);  // Removed the second argument
  lv_obj_align(dropdown_frequency, LV_ALIGN_TOP_RIGHT, 10, 50);  // Adjusted position
  lv_dropdown_set_options(dropdown_frequency, "315\n433.92\n868.35");
  lv_dropdown_set_selected(dropdown_frequency, 0);  // Set default selection to 315 MHz
  lv_dropdown_set_dir(dropdown_frequency, LV_DIR_BOTTOM);
  lv_obj_add_event_cb(dropdown_frequency, frequency_dropdown_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
  // Create the Modulation dropdown next to Frequency in the Config tab
  dropdown_modulation = lv_dropdown_create(tab_config);  // Removed the second argument
  lv_obj_align(dropdown_modulation, LV_ALIGN_TOP_RIGHT, 10, 10);  // Position it below Frequency
  lv_dropdown_set_options(dropdown_modulation, "AOOK\nFSK");
  lv_dropdown_set_selected(dropdown_modulation, 0);  // Set default selection to AOOK
  lv_dropdown_set_dir(dropdown_modulation, LV_DIR_BOTTOM);
  //Create Power Dropdown
  dropdown_power = lv_dropdown_create(tab_config);  // Removed the second argument
  lv_obj_align_to(dropdown_power, btn_power, LV_ALIGN_TOP_RIGHT, 10, 90);
  lv_dropdown_set_options(dropdown_power, "6\n12");
  lv_dropdown_set_selected(dropdown_power, 1);  // Set default selection to AOOK
  lv_dropdown_set_dir(dropdown_power, LV_DIR_BOTTOM);
}
void createTabs() {
  // Create a tab view object at the top of the screen with 50px tab height
  tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);

  // Add tabs with icons
  lv_obj_t *tab_sd = lv_tabview_add_tab(tabview, LV_SYMBOL_SD_CARD);
  tab_config = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS);  // Add settings icon
  lv_obj_t *tab_home = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME); // Add home icon
  lv_obj_t *tab_ja = lv_tabview_add_tab(tabview, LV_SYMBOL_WARNING);
  lv_obj_t *tab_wifi = lv_tabview_add_tab(tabview, LV_SYMBOL_WIFI);
  lv_obj_t *tab_ir = lv_tabview_add_tab(tabview, "IR");


  // Remove unnecessary text in the tabs, add buttons instead
  // Add content to the SD tab
  lv_obj_t *btn_sd = lv_btn_create(tab_sd);
  lv_obj_t *label_sd = lv_label_create(btn_sd);
  lv_label_set_text(label_sd, "SD Content");
  lv_obj_align(btn_sd, LV_ALIGN_TOP_LEFT, 10, 10); // Center the button in SD tab
//  lv_obj_add_event_cb(btn_sd, btn_sd_event_handler, LV_EVENT_CLICKED, NULL); // Set button click handler
  // Add content to the Wifi tab
  static lv_style_t style_red_btn;
  lv_style_init(&style_red_btn);
  lv_style_set_bg_color(&style_red_btn, lv_color_hex(0xFF0000)); // Set background to red
  lv_style_set_bg_opa(&style_red_btn, LV_OPA_COVER);            // Set background opacity to fully visible
  lv_style_set_border_color(&style_red_btn, lv_color_hex(0x880000)); // Optional: Set a darker red border
  lv_style_set_border_width(&style_red_btn, 2);                 // Optional: Set border width

  // Create the "Ja" button
  lv_obj_t *btn_ja = lv_btn_create(tab_ja);                     // Create a button in the tab_ja tab
  lv_obj_add_style(btn_ja, &style_red_btn, LV_PART_MAIN);       // Apply the red button style
  lv_obj_align(btn_ja, LV_ALIGN_TOP_LEFT, 10, 10);              // Align the button to the top-left corner

  // Add a label to the button
  lv_obj_t *label_ja = lv_label_create(btn_ja);                 // Create a label inside the button
  lv_label_set_text(label_ja, "Jammer\nOn/Off");                            // Set the label text to "Ja"

  lv_obj_t *btn_ir = lv_btn_create(tab_ir);                     
  lv_obj_add_style(btn_ir, &style_red_btn, LV_PART_MAIN);       // Apply the red button style
  lv_obj_align(btn_ir, LV_ALIGN_TOP_LEFT, 10, 10);  

  lv_obj_t *label_ir = lv_label_create(btn_ir);                 // Create a label inside the button
  lv_label_set_text(label_ir, "Listen");
  lv_obj_add_event_cb(btn_ir, listenButtonEvent, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(btn_ir, BUTTON_WIDTH, BUTTON_HEIGHT);

  //IR Power
  lv_obj_t *btn_irs = lv_btn_create(tab_ir);                     
  lv_obj_add_style(btn_irs, &style_red_btn, LV_PART_MAIN);       // Apply the red button style
  lv_obj_align(btn_ir, LV_ALIGN_TOP_LEFT, 0, 50); 
  lv_obj_set_size(btn_irs, BUTTON_WIDTH, BUTTON_HEIGHT);

  lv_obj_t *label_irs = lv_label_create(btn_irs);                 // Create a label inside the button
  lv_label_set_text(label_irs, "IR Power");
  lv_obj_add_event_cb(btn_irs, IR_Power_action, LV_EVENT_CLICKED, NULL);
  

  // Add a button to the Wifi tab with the label "Portal"
  lv_obj_t *btn_wifi = lv_btn_create(tab_wifi);         // Create a button in the tab_wifi tab
  lv_obj_align(btn_wifi, LV_ALIGN_TOP_LEFT, 10, 10);    // Align the button to the top-left corner

  lv_obj_t *label_wifi = lv_label_create(btn_wifi);     // Create a label inside the button
  lv_label_set_text(label_wifi, "Wifi");

  // Add content to the Config tab
  lv_obj_t *btn_frequency = lv_btn_create(tab_config);
  lv_obj_t *label_frequency = lv_label_create(btn_frequency);
  lv_label_set_text(label_frequency, "Frequency");
  lv_obj_set_size(btn_frequency, 100, 35);
  lv_obj_align(btn_frequency, LV_ALIGN_TOP_LEFT, 10, 50);

  // Create the "Modulation" button next to the "Frequency" label
  lv_obj_t *btn_modulation = lv_btn_create(tab_config);
  lv_obj_t *label_modulation = lv_label_create(btn_modulation);
  lv_label_set_text(label_modulation, "Modulation");
  lv_obj_set_size(btn_modulation, 100, 35); // Same size as frequency button
  lv_obj_align(btn_modulation, LV_ALIGN_TOP_LEFT, 10, 10); // Position next to frequency label

  lv_obj_t *btn_power = lv_btn_create(tab_config);
  lv_obj_t *label_power = lv_label_create(btn_power);
  lv_label_set_text(label_power, "Power");
  lv_obj_set_size(btn_power, 100, 35);
  lv_obj_align(btn_power, LV_ALIGN_OUT_RIGHT_MID, 10, 90); // Position next to frequency label

  lv_obj_t *btn_rxb = lv_btn_create(tab_config);
  lv_obj_t *label_rxb = lv_label_create(btn_rxb);
  lv_label_set_text(label_rxb, "Deviation");
  lv_obj_set_size(btn_rxb, 100, 35); // Same size for consistency
  lv_obj_align(btn_rxb, LV_ALIGN_OUT_RIGHT_MID, 10, 130); // Position next to frequency label /////Deviation 5 and 0 ////RXBW 812.50 and 58

  // Create a toggle switch for the "Portal" section
  lv_obj_t *toggle_portal = lv_switch_create(tab_wifi);
  lv_obj_align(toggle_portal, LV_ALIGN_CENTER, 0, 0); // Position below "Portal" label
  lv_obj_add_event_cb(toggle_portal, toggle_portal_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

  // Create a toggle switch for the "Ja" page
  lv_obj_t *toggle_ja = lv_switch_create(tab_ja);
  lv_obj_align(toggle_ja, LV_ALIGN_CENTER, 0, 0); // Center toggle on the Ja page
  lv_obj_add_event_cb(toggle_ja, toggle_ja_event_handler, LV_EVENT_VALUE_CHANGED, NULL); // Add event handler

  // Create buttons in the Home tab
  lv_obj_t *btn_listen = lv_btn_create(tab_home);
  lv_obj_t *label_listen = lv_label_create(btn_listen);
  lv_label_set_text(label_listen, "Listen");

  lv_obj_t *btn_test1 = lv_btn_create(tab_home);
  lv_obj_t *label_test1 = lv_label_create(btn_test1);
  lv_label_set_text(label_test1, "Panic");

  lv_obj_t *btn_test2 = lv_btn_create(tab_home);
  lv_obj_t *label_test2 = lv_label_create(btn_test2);
  lv_label_set_text(label_test2, "Tesla");

  lv_obj_t *btn_test3 = lv_btn_create(tab_home);
  lv_obj_t *label_test3 = lv_label_create(btn_test3);
  lv_label_set_text(label_test3, "Empty");

  lv_obj_t *btn_test4 = lv_btn_create(tab_home);
  lv_obj_t *label_test4 = lv_label_create(btn_test4);
  lv_label_set_text(label_test4, "Scan");  

  lv_obj_t *btn_test5 = lv_btn_create(tab_home);
  lv_obj_t *label_test5 = lv_label_create(btn_test5);
  lv_label_set_text(label_test5, "Debruijin");  

    // Set sizes for all buttons to make them uniform
  lv_obj_set_size(btn_test1, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_set_size(btn_test2, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_set_size(btn_test3, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_set_size(btn_test4, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_set_size(btn_test5, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_set_size(btn_listen, BUTTON_WIDTH, BUTTON_HEIGHT);

  // Align the buttons (stack them in two rows: 3 buttons in the first row, "Listen" in the second)
  lv_obj_align(btn_test1, LV_ALIGN_TOP_LEFT, 0, 10);  // Align btn_test1 to the top left
  lv_obj_align(btn_test2, LV_ALIGN_TOP_MID, 0, 10);     // Align btn_test2 to the top center
  lv_obj_align(btn_test3, LV_ALIGN_TOP_RIGHT, 0, 10); // Align btn_test3 to the top right
  lv_obj_align(btn_test4, LV_ALIGN_BOTTOM_MID, 0, 10); // Align btn_listen below the first row,
  // Move "Listen" to the second row below the first three buttons, centered
  lv_obj_align(btn_listen, LV_ALIGN_BOTTOM_LEFT, 0, 10); // Align btn_listen below the first row, centered
  lv_obj_align(btn_test5, LV_ALIGN_BOTTOM_RIGHT, 0, 10);
  // Add an event callback to "Test1" button
  // Button 1
  lv_obj_add_event_cb(btn_test1, button_action, LV_EVENT_CLICKED, &button_ids[0]);
  // Button 2
  lv_obj_add_event_cb(btn_test2, button_action, LV_EVENT_CLICKED, &button_ids[1]);
  // Button 3
  lv_obj_add_event_cb(btn_test3, button_action, LV_EVENT_CLICKED, &button_ids[2]);
  // Button 4
  lv_obj_add_event_cb(btn_test4, scan_button_action, LV_EVENT_CLICKED, NULL);
  // Button 5
  lv_obj_add_event_cb(btn_test5, button_action, LV_EVENT_CLICKED, &button_ids[4]);
  // Listen Button
  lv_obj_add_event_cb(btn_listen, listen_button_action, LV_EVENT_CLICKED, NULL);
/*  if (!initSDCard()) {
   Serial.println("Failed SD init!");
  }  */
  createDropdowns();
}

void spectrum_animation_cb(lv_timer_t *timer) {
    // Update the line points to simulate the heartbeat
    for (int i = 0; i < NUM_POINTS; i++) {
        int index = (heartbeat_offset + i) % NUM_POINTS;

        // Create a basic waveform pattern with random noise
        heartbeat_points[index].y = 40 + (i % 10 == 0 ? lv_rand(-5, 5) : 0);
    }

    // Increment offset to scroll the waveform
    heartbeat_offset = (heartbeat_offset + 1) % NUM_POINTS;

    // Refresh the line object
    lv_obj_invalidate(heartbeat_line);
}
void button_action(lv_event_t *e) {
    // Get the button ID or identifier passed through the event
    int *button_id = (int *)lv_event_get_user_data(e);

    // Create a new popup
    popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup, 215, 155);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Add a label inside the popup
    lv_obj_t *label_popup = lv_label_create(popup);
    lv_label_set_text(label_popup, "Heartbeat Monitor");

    // Create a container for the heartbeat line
    spectrum_container = lv_obj_create(popup);
    lv_obj_set_size(spectrum_container, 200, 80);
    lv_obj_align(spectrum_container, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_border_color(spectrum_container, lv_color_hex(0x000000), LV_PART_MAIN);

    // Initialize the points for the heartbeat line
    for (int i = 0; i < NUM_POINTS; i++) {
        heartbeat_points[i].x = i * (200 / NUM_POINTS);  // Distribute points evenly
        heartbeat_points[i].y = 40;  // Default flat line at the middle
    }

    // Create the line object for the heartbeat
    heartbeat_line = lv_line_create(spectrum_container);
    lv_line_set_points(heartbeat_line, heartbeat_points, NUM_POINTS);  // Set initial points
    lv_obj_set_style_line_width(heartbeat_line, 2, LV_PART_MAIN);  // Thin line
    lv_obj_set_style_line_color(heartbeat_line, lv_color_hex(0xFF69B4), LV_PART_MAIN);  // Pink color
    lv_obj_center(heartbeat_line);  // Center the line in the container

    // Create a "Send" button
    lv_obj_t *btn_send = lv_btn_create(popup);
    lv_obj_t *label_send = lv_label_create(btn_send);
    lv_label_set_text(label_send, "Send");
    lv_obj_align(btn_send, LV_ALIGN_BOTTOM_LEFT, 10, -20);
    lv_obj_add_event_cb(btn_send, send_action, LV_EVENT_CLICKED, button_id);  // Pass button ID

    // Create an "Exit" button
    lv_obj_t *btn_exit = lv_btn_create(popup);
    lv_obj_t *label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, "Exit");
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_RIGHT, -10, -20);
    lv_obj_add_event_cb(btn_exit, exit_action, LV_EVENT_CLICKED, NULL);

    // Timer to animate the heartbeat line
    lv_timer_t *timer = lv_timer_create(spectrum_animation_cb, 100, NULL);
    lv_timer_set_repeat_count(timer, -1);
    lv_obj_align(btn_send, LV_ALIGN_BOTTOM_LEFT, 10, 2);  // Move "Send" button down
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_RIGHT, -10, 2); // Move "Exit" button down
}
void scan_button_action(lv_event_t *e) {
    ELECHOUSE_cc1101.SetRx();
    mySwitch.disableTransmit();
    delay(100);
    mySwitch.enableReceive(gdo0);

    // Create a pop-up for scanning
    lv_obj_t *popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup, 220, 150);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create a container to hold the scanning result text
    lv_obj_t *results_container = lv_obj_create(popup);
    lv_obj_set_size(results_container, 200, 80);  // Adjust the size as needed
    lv_obj_align(results_container, LV_ALIGN_TOP_MID, 0, 0);  // Adjust the position
    lv_obj_set_style_border_color(results_container, lv_color_hex(0x000000), LV_PART_MAIN); // Border color

    // Add a label to show scanning progress/results inside the container
    lv_obj_t *label_scan = lv_label_create(results_container);
    lv_label_set_text(label_scan, "Scanning...");
    lv_obj_align(label_scan, LV_ALIGN_TOP_MID, 0, 10);

    // Add an "Exit" button
    lv_obj_t *btn_exit = lv_btn_create(popup);
    lv_obj_t *label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, "Exit");
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Store the popup pointer in the user data for the button callback
    lv_obj_add_event_cb(btn_exit, [](lv_event_t *e) {
        lv_obj_t *popup = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
        lv_obj_del(popup); // Close the popup
    }, LV_EVENT_CLICKED, popup);

    // Start scanning in a separate task to avoid blocking the UI
    struct ScanData {
        lv_obj_t *label_scan;
        float freq;
        String result;
    };

    auto *data = new ScanData{label_scan, 433.0, ""};
    lv_timer_t *timer = lv_timer_create([](lv_timer_t *timer) {
        auto *data = static_cast<ScanData *>(timer->user_data);

        // Perform a single step of scanning
        ELECHOUSE_cc1101.setMHZ(data->freq);
        int rssi = ELECHOUSE_cc1101.getRssi();
        Serial.println(rssi);

        if (rssi > -70) { // Adjust threshold as needed
            data->result += "Freq: " + String(data->freq) + " MHz, RSSI: " + String(rssi) + "\n";
        }

        data->freq += 0.1;
        if (data->freq > 434.0) { // Stop scanning
            lv_label_set_text(data->label_scan, data->result.c_str());
            lv_timer_del(timer); // Stop the timer
            delete data; // Clean up
        }
    }, 100, data); // Update every 100 ms
}
void handleInterrupt() {
    static unsigned long lastTime = 0;
    const long time = micros();
    const unsigned int duration = time - lastTime;

    if (millis() - resetTime > 100) { // Example threshold
        if (counter > 10) { // Example count threshold
            Receive = 1;
            detachInterrupt(interruptPin);
        } else {
            counter = 0;
            Receive = 0;
        }
    }

    if (duration > 100) {
        timings[counter] = duration;
        Serial.println(duration);
        counter++;
    }

    if (counter >= SAMPLESIZE) {
        Receive = 1;
        detachInterrupt(interruptPin);
    }

    lastTime = time;
    resetTime = millis();
}

// Function to update results in the popup
void update_results_label() {
    if (Receive) {
        String captured_data = "Raw Captured:\n";
        for (unsigned int i = 1; i < counter; i++) {
            captured_data += String(timings[i]) + ",";
        }

        // Remove the trailing comma and update the label
        captured_data = captured_data.substring(0, captured_data.length() - 1);
        lv_label_set_text(results_label, captured_data.c_str());
        Receive = 0; // Reset Receive flag for next capture
        counter = 0; // Reset counter
    }
}

// Listen button action
void listen_button_action(lv_event_t *e) {
    // Create a pop-up for listening
    lv_obj_t *popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup, 220, 150);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t *results_container = lv_obj_create(popup);
    lv_obj_set_size(results_container, 200, 80);
    lv_obj_align(results_container, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_border_color(results_container, lv_color_hex(0x000000), LV_PART_MAIN);

    results_label = lv_label_create(results_container);
    lv_label_set_text(results_label, "Waiting for data...");
    lv_obj_align(results_label, LV_ALIGN_TOP_LEFT, 5, 5);

    // Add Save, Replay, and Exit buttons
    lv_obj_t *btn_save = lv_btn_create(popup);
    lv_obj_t *label_save = lv_label_create(btn_save);
    lv_label_set_text(label_save, "Save");
    lv_obj_align(btn_save, LV_ALIGN_BOTTOM_LEFT, -3, 4);

    lv_obj_t *btn_replay = lv_btn_create(popup);
    lv_obj_t *label_replay = lv_label_create(btn_replay);
    lv_label_set_text(label_replay, "Replay");
    lv_obj_align(btn_replay, LV_ALIGN_BOTTOM_MID, 0, 4);
    lv_obj_add_event_cb(btn_replay, [](lv_event_t *e) {
        replayRawSignal();
    }, LV_EVENT_CLICKED, NULL);    

    lv_obj_t *btn_exit = lv_btn_create(popup);
    lv_obj_t *label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, "Exit");
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_RIGHT, 0, 4);
    lv_obj_add_event_cb(btn_exit, [](lv_event_t *e) {
        lv_obj_t *popup = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
        lv_obj_del(popup); // Close the popup
    }, LV_EVENT_CLICKED, popup);

    // Start receiving data
    attachInterrupt(interruptPin, handleInterrupt, CHANGE);

    // Poll for received data and update the label
    lv_timer_create([](lv_timer_t *timer) {
        update_results_label();
    }, 100, NULL); // Check for updates every 100ms
}
void onRawSignalReceived(const uint8_t *data, size_t length) {
    if (length <= SAMPLESIZE) {
        memcpy(raw_signal_buffer, data, length);
        raw_signal_length = length;
    }
}
void replayRawSignal() {
    if (raw_signal_length > 0) {
        ELECHOUSE_cc1101.SendData(raw_signal_buffer, raw_signal_length);
        Serial.println("Signal replayed successfully!");
    } else {
        Serial.println("No signal to replay!");
    }
}

/////RF STUFF
void Jamloop() {
  mySwitch.send("ffffffff");
}
void sendSequence() {
  for (uint8_t i = 0; i < messageLength; i++) {
    sendByte(sequence[i]);
  }
  
}
//-------------------------------------------------------------------------------
void sendByte(uint8_t dataByte) {
  for (int8_t bit = 7; bit >= 0; bit--) {            // MSB first
    digitalWrite(44, (dataByte & (1 << bit)) ? HIGH : LOW);
    delayMicroseconds(pulseWidth);
  }
}
void SendTesla(){
  for (uint8_t t = 0; t < transmissions; t++) {
    sendSequence();
    delay(messageDistance);
  }
}
void jeepset() {
  ELECHOUSE_cc1101.setDRate(2);
  ELECHOUSE_cc1101.setSyncMode(2);
  mySwitch.setProtocol(1);   // Standard protocol for 315 MHz ASK
  mySwitch.setPulseLength(200); 
}
void jeepsend() {
  uint64_t signal = 0xAAAA95969A969A99599A595555A999AA655A5955595A999659669;
  mySwitch.send(signal, 72);
}
String convertToPWM(const char* binaryString, const String& zero_pwm, const String& one_pwm) {
    String pwmString = "";
    while (*binaryString) {
        if (*binaryString == '0') {
            pwmString += zero_pwm;
        } else if (*binaryString == '1') {
            pwmString += one_pwm;
        }
        binaryString++;
    }
    return pwmString;
}
void sendPWMData(RCSwitch &mySwitch, const String& pwmString, int protocol) {
    mySwitch.disableReceive();
    delay(100);
    mySwitch.enableTransmit(gdo0);

    mySwitch.setProtocol(protocol);
    
    // Transmit the PWM signal
    for (int i = 0; i < pwmString.length(); i++) {
        if (pwmString[i] == '1') {
            mySwitch.send(1, 1); // Adjust as needed to represent a '1' pulse
        } else {
            mySwitch.send(0, 1); // Adjust as needed to represent a '0' pulse
        }
        // You may need to add delays between pulses or adjust signal timing
    }

    mySwitch.disableTransmit();
    delay(100);
    mySwitch.enableReceive(gdo0);
}

// Function to send command
void sendCommand(RCSwitch &mySwitch, const char* command, const String& zero_pwm, const String& one_pwm, int protocol) {
    // Convert binary command to PWM string
    String pwmString = convertToPWM(command, zero_pwm, one_pwm);

    // Use the sendPWMData function to send the PWM-encoded data
    sendPWMData(mySwitch, pwmString, protocol);
}
unsigned long binaryStringToLong(const char* binaryString) {
    unsigned long value = 0;
    while (*binaryString) {
        value = (value << 1) | (*binaryString - '0');
        binaryString++;
    }
    return value;
}
void handleDebruiSend(int repeatDebru) {
    // Create an instance of RCSwitch if not already done
    RCSwitch mySwitch = RCSwitch(); // Ensure you have instantiated mySwitch

    // Linear10 DeBrui
    sendCommand(mySwitch, debruijn_ten.c_str(), "0", "1", 2); // Adjusted call
    Serial.println("EOTX: Linear10 DeBrui"); 

    // Stanley10 DeBrui
    sendCommand(mySwitch, debruijn_ten.c_str(), "0", "1", 2); // Adjusted call
    Serial.println("EOTX: Stanley10 DeBrui"); 

    // Chamberlain789 DeBrui
    sendCommand(mySwitch, debruijn_nine.c_str(), "0", "1", 2); // Adjusted call
    Serial.println("EOTX: Chamberlain789 DeBrui"); 

    // Linear8 DeBrui
    Serial.println("TX: Linear8 DeBrui");
    sendCommand(mySwitch, debruijn_eight.c_str(), "0", "1", 2); // Adjusted call
}


//// IR
String resultToHexadecimal(decode_results *results) {
    if (results->decode_type != UNKNOWN) {
        return String(results->value, HEX);
    } else {
        String hexCode = "";
        for (int i = 0; i < results->rawlen; i++) {
            hexCode += String(results->rawbuf[i], HEX);
        }
        return hexCode;
    }
}

String typeToString(decode_type_t type) {
    switch (type) {
        case NEC: return "NEC";
        case SONY: return "Sony";
        case RC5: return "RC5";
        case RC6: return "RC6";
        case DISH: return "DISH";
        case SHARP: return "Sharp";
        case JVC: return "JVC";
        case SANYO: return "Sanyo";
        case MITSUBISHI: return "Mitsubishi";
        case SAMSUNG: return "Samsung";
        case LG: return "LG";
        case WHYNTER: return "Whynter";
        case AIWA_RC_T501: return "Aiwa RC-T501";
        case PANASONIC: return "Panasonic";
        case DENON: return "Denon";
        default: return "Unknown";
    }
}

void sendCode(String type, String code) {
    uint32_t irCode = strtoul(code.c_str(), NULL, 16);  // Convert hex string to uint32_t
    if (type == "NEC") {
        irsend.sendNEC(irCode, 32);
    } else if (type == "Sony") {
        irsend.sendSony(irCode, 20);
    } else if (type == "RC5") {
        irsend.sendRC5(irCode, 13);
    } else if (type == "RC6") {
        irsend.sendRC6(irCode, 20);
    } else if (type == "RAW") {
        sendRaw(code);
        Serial.println("Replaying raw signal...");
    } else {
        Serial.println("Unknown type. Cannot send.");
    }
}

String resultToRawData(decode_results *results) {
    String rawData = "";
    for (int i = 1; i < results->rawlen; i++) {
        if (i > 1) rawData += ",";
        rawData += String(results->rawbuf[i] * 50);  // Adjust timing calculation
    }
    return rawData;
}

void sendRaw(String rawSignal) {
    uint16_t rawArray[100];
    int length = 0;

    Serial.println("Raw signal input: " + rawSignal);  // Debug print

    char rawSignalCopy[rawSignal.length() + 1];
    rawSignal.toCharArray(rawSignalCopy, sizeof(rawSignalCopy));

    char *token = strtok(rawSignalCopy, ",");
    while (token != NULL && length < 100) {
        rawArray[length++] = atoi(token);
        token = strtok(NULL, ",");
    }

    Serial.print("Sending raw array: ");
    for (int i = 0; i < length; i++) {
        Serial.print(rawArray[i]);
        Serial.print(" ");
    }
    Serial.println();

    irsend.sendRaw(rawArray, length, 38);

    Serial.println("Raw signal sent!");
}

void closePopupEvent(lv_event_t *e) {
    lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_del(popup); // Close the popup
    }
}

void showPopup(const char *message) {
    static bool popupShown = false;  // Prevent multiple popups

    if (!popupShown) {
        lv_obj_t *popup = lv_obj_create(lv_scr_act());
        lv_obj_set_size(popup, 220, 150);
        lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_pad_all(popup, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

        String fullMessage = "Protocol: " + lastProtocol + "\n";
        fullMessage += "Address: " + lastAddress + "\n";
        fullMessage += "Command: " + lastCommand;

        lv_obj_t *label = lv_label_create(popup);
        lv_label_set_text(label, fullMessage.c_str());
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

        lv_obj_t *btn_replay = lv_btn_create(popup);
        lv_obj_set_size(btn_replay, 80, 40);
        lv_obj_align(btn_replay, LV_ALIGN_BOTTOM_LEFT, 0, 4);
        lv_obj_t *label_replay = lv_label_create(btn_replay);
        lv_label_set_text(label_replay, "Replay");

        lv_obj_add_event_cb(btn_replay, (lv_event_cb_t)[](lv_event_t *e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                Serial.println("Replay button clicked.");
                if (!lastProtocol.isEmpty()) {
                    sendCode(lastProtocol, lastAddress + lastCommand);  // Replay the protocol with address and command
                    Serial.println("Replayed signal: Protocol " + lastProtocol + ", Address " + lastAddress + ", Command " + lastCommand);  // Debug print
                } else {
                    Serial.println("No signal to replay.");
                }
            }
        }, LV_EVENT_CLICKED, NULL);

        lv_obj_t *btn_close = lv_btn_create(popup);
        lv_obj_set_size(btn_close, 80, 40);
        lv_obj_align(btn_close, LV_ALIGN_BOTTOM_RIGHT, 0, 4);
        lv_obj_t *label_close = lv_label_create(btn_close);
        lv_label_set_text(label_close, "Close");

        lv_obj_add_event_cb(btn_close, closePopupEvent, LV_EVENT_CLICKED, (void *)popup);

        lv_obj_align(btn_replay, LV_ALIGN_BOTTOM_LEFT, 10, 2);  // Move "Replay" button down
        lv_obj_align(btn_close, LV_ALIGN_BOTTOM_RIGHT, -10, 2); // Move "Close" button down
        popupShown = true;  // Mark popup as shown
    }
}

void IRListenTask() {
    if (isIRListening && irrecv.decode(&results)) {
        Serial.println("IR signal received!");
        
        // Check if raw data is available
        if (results.rawlen > 0) {
            Serial.print("Raw data: ");
            for (int i = 0; i < results.rawlen; i++) {
                Serial.print(results.rawbuf[i]);
                Serial.print(" ");
            }
            Serial.println();  // New line after printing raw data
        }

        // Store the decoded signal
        lastProtocol = protocol;    // Save protocol
        lastAddress = address;      // Save address
        lastCommand = command;      // Save command
        lastRawSignal = resultToRawData(&results);  // Save raw data (if needed for non-standard protocols)

        showPopupFlag = true;  // Set the flag to show the popup

        irrecv.resume();  // Prepare for the next signal
    }
}

void listenButtonEvent(lv_event_t *e) {
    irrecv.enableIRIn();
    isIRListening = true;
    Serial.println("Listening for IR signals...");
}
void IR_Power_action(lv_event_t *e) {
    IRPower();
}
void IRPower() {
    irsend.sendNEC(0x00FF48B7, 32); delay(500);
    irsend.sendNEC(0x20DF10EF, 32); delay(500);
    irsend.sendPanasonic(0x40040100, 32); delay(500);
    irsend.sendRC5(0x1F40, 13); delay(500);
    irsend.sendNEC(0xE0E040BF, 32); delay(500);
}

