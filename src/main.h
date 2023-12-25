/**
 * @file main.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Includes, defines and globals
 * @version 0.1
 * @date 2023-04-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include <Arduino.h>
#include <WisBlock-API-V2.h>
#include <blues-minimal-i2c.h>
#include "RAK1906_env.h"

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 1
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                     \
	do                                      \
	{                                       \
		if (tag)                            \
			PRINTF("[%s] ", tag);           \
		PRINTF(__VA_ARGS__);                \
		PRINTF("\n");                       \
		Serial.flush();                     \
		if (g_ble_uart_is_connected)        \
		{                                   \
			g_ble_uart.printf(__VA_ARGS__); \
			g_ble_uart.printf("\n");        \
		}                                   \
	} while (0)
#else
#define MYLOG(...)
#endif

/** Define the version of your SW */
#ifndef SW_VERSION_1
#define SW_VERSION_1 1 // major version increase on API change / not backwards compatible
#endif
#ifndef SW_VERSION_2
#define SW_VERSION_2 0 // minor version increase on API change / backward compatible
#endif
#ifndef SW_VERSION_3
#define SW_VERSION_3 0 // patch version increase on bugfix, no affect on API
#endif

/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);

// Wakeup flags
#define PARSE 0b1000000000000000
#define N_PARSE 0b0111111111111111

// Cayenne LPP Channel numbers per sensor value
#define LPP_CHANNEL_BATT 1 // Base Board

// Globals
extern WisCayenne g_solution_data;

// Parser
bool blues_parse_send(uint8_t *data, uint16_t data_len);

// Blues.io
struct s_blues_settings
{
	uint16_t valid_mark = 0xAA55;								 // Validity marker
	char product_uid[256] = "com.my-company.my-name:my-project"; // Blues Product UID
	bool conn_continous = false;								 // Use periodic connection
	uint8_t sim_usage = 0;										 // 0 int SIM, 1 ext SIM, 2 ext int SIM, 3 int ext SIM
	char ext_sim_apn[256] = "internet";							 // APN to be used with external SIM
	bool motion_trigger = true;									 // Send data on motion trigger
};

bool init_blues(void);
bool blues_send_req(void);
void blues_hub_status(void);
bool blues_send_payload(uint8_t *data, uint16_t data_len);
void blues_card_restore(void);
extern RAK_BLUES rak_blues;
extern s_blues_settings g_blues_settings;
extern char blues_response[];

// User AT commands
void init_user_at(void);
bool read_blues_settings(void);
void save_blues_settings(void);

// OLED
#include <nRF_SSD1306Wire.h>
bool init_rak1921(void);
void rak1921_add_line(char *line);
void rak1921_show(void);
void rak1921_write_header(char *header_line);
void rak1921_clear(void);
void rak1921_write_line(int16_t line, int16_t y_pos, String text);
void rak1921_display(void);
extern char line_str[];
extern bool has_rak1921;

#endif // _MAIN_H_