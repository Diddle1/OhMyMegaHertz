#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Include necessary libraries
#include <lvgl.h> // Include the LVGL library
#include "RCSwitch.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

extern bool isJamloopActive;
extern volatile bool showPopupFlag;
extern bool isIRListening;

void createTabs();
void createSlider(); // If you have other functions, declare them too
void clearArray();
void Jamloop();
void handleDebruiSend(int repeatDebru);
void SendTesla();
void jeepset();
void jeepsend();
void IRPower();
#endif