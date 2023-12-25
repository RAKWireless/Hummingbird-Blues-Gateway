/**
 * @file blues.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Blues.IO NoteCard handler
 * @version 0.1
 * @date 2023-04-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h"
#include "product_uid.h"

#ifndef PRODUCT_UID
#define PRODUCT_UID "com.my-company.my-name:my-project"
#pragma message "PRODUCT_UID is not defined in this example. Please ensure your Notecard has a product identifier set before running this example or define it in code here. More details at https://dev.blues.io/tools-and-sdks/samples/product-uid"
#endif
#define myProductID PRODUCT_UID

RAK_BLUES rak_blues;

/** Buffer for serialized JSON response */
char blues_response[4096];

bool init_blues(void)
{
	Wire.begin();
	Wire.setClock(100000);

	// Get the ProductUID from the saved settings
	// If no settings are found, use NoteCard internal settings!
	if (read_blues_settings())
	{
		MYLOG("BLUES", "Found saved settings, override NoteCard internal settings!");
		if (memcmp(g_blues_settings.product_uid, "com.my-company.my-name", 22) == 0)
		{
			MYLOG("BLUES", "No Product ID saved");
			AT_PRINTF(":EVT NO PUID");
			memcpy(g_blues_settings.product_uid, PRODUCT_UID, 33);
		}

		// {"req": "hub.set"}
		MYLOG("BLUES", "Set Product ID and connection mode");
		if (rak_blues.start_req((char *)"hub.set"))
		{
			rak_blues.add_string_entry((char *)"product", g_blues_settings.product_uid);
			if (g_blues_settings.conn_continous)
			{
				rak_blues.add_string_entry((char *)"mode", "continuous");
			}
			else
			{
				rak_blues.add_string_entry((char *)"mode", "minimum");
			}
			// Set sync time to 20 times the sensor read time
			rak_blues.add_int32_entry((char *)"seconds", (g_lorawan_settings.send_repeat_time * 20 / 1000));
			rak_blues.add_bool_entry((char *)"heartbeat", true);

			if (!rak_blues.send_req())
			{
				MYLOG("BLUES", "hub.set request failed");
				return false;
			}
		}
		else
		{
			MYLOG("BLUES", "hub.set request failed");
			return false;
		}

#if USE_GNSS == 1
		// {"req": "card.location.mode"}
		MYLOG("BLUES", "Set location mode");
		if (rak_blues.start_req((char *)"card.location.mode"))
		{
			// Continous GNSS mode
			// rak_blues.add_string_entry((char *)"mode", "continous");

			// Periodic GNSS mode
			rak_blues.add_string_entry((char *)"mode", "periodic");

			// Set location acquisition time to the sensor read time
			rak_blues.add_int32_entry((char *)"seconds", (g_lorawan_settings.send_repeat_time / 2000));
			rak_blues.add_bool_entry((char *)"heartbeat", true);
			if (!rak_blues.send_req())
			{
				MYLOG("BLUES", "card.location.mode request failed");
				return false;
			}
		}
		else
		{
			MYLOG("BLUES", "card.location.mode request failed");
			return false;
		}
#else
		// {"req": "card.location.mode"}
		MYLOG("BLUES", "Stop location mode");
		if (rak_blues.start_req((char *)"card.location.mode"))
		{
			// GNSS mode off
			rak_blues.add_string_entry((char *)"mode", "off");
			if (!rak_blues.send_req())
			{
				MYLOG("BLUES", "card.location.mode request failed");
				return false;
			}
		}
		else
		{
			MYLOG("BLUES", "card.location.mode request failed");
			return false;
		}
#endif

		MYLOG("BLUES", "Set APN");
		// {“req”:”card.wireless”}
		if (rak_blues.start_req((char *)"card.wireless"))
		{
			rak_blues.add_string_entry((char *)"mode", "auto");

			switch (g_blues_settings.sim_usage)
			{
			case 0:
				// USING BLUES eSIM CARD
				rak_blues.add_string_entry((char *)"method", (char *)"primary");
				break;
			case 1:
				// USING EXTERNAL SIM CARD only
				rak_blues.add_string_entry((char *)"apn", g_blues_settings.ext_sim_apn);
				rak_blues.add_string_entry((char *)"method", (char *)"secondary");
				break;
			case 2:
				// USING EXTERNAL SIM CARD as primary
				rak_blues.add_string_entry((char *)"apn", g_blues_settings.ext_sim_apn);
				rak_blues.add_string_entry((char *)"method", (char *)"dual-secondary-primary");
				break;
			case 3:
				// USING EXTERNAL SIM CARD as secondary
				rak_blues.add_string_entry((char *)"apn", g_blues_settings.ext_sim_apn);
				rak_blues.add_string_entry((char *)"method", (char *)"dual-primary-secondary");
				break;
			}

			if (!rak_blues.send_req())
			{
				MYLOG("BLUES", "card.wireless request failed");
				return false;
			}
		}
		else
		{
			MYLOG("BLUES", "card.wireless request failed");
			return false;
		}

#if IS_V2 == 1
		// Only for V2 cards, setup the WiFi network
		// {"req": "card.wifi"}
		MYLOG("BLUES", "Set WiFi");
		if (rak_blues.start_req((char *)"card.wifi"))
		{
			rak_blues.add_string_entry((char *)"ssid", "-");
			rak_blues.add_string_entry((char *)"password", "-");
			rak_blues.add_string_entry((char *)"name", "RAK-");
			rak_blues.add_string_entry((char *)"org", "RAK-PH");
			rak_blues.add_bool_entry((char *)"start", false);

			if (!rak_blues.send_req())
			{
				MYLOG("BLUES", "card.wifi request failed");
			}
		}
		else
		{
			MYLOG("BLUES", "card.wifi request failed");
			return false;
		}
#endif
	}

	// {"req": "card.version"}
	if (rak_blues.start_req((char *)"card.version"))
	{
		if (!rak_blues.send_req(blues_response, 4096))
		{
			MYLOG("BLUES", "card.version request failed");
		}
		char version[64];
		char dev_id[64];
		char sku[64];
		rak_blues.get_string_entry((char *)"version", version, 64);
		rak_blues.get_string_entry((char *)"sku", sku, 64);
		rak_blues.get_string_entry((char *)"device", dev_id, 64);

		AT_PRINTF("+EVT:V=%s", version);
		AT_PRINTF("+EVT:T=%s", sku);
		AT_PRINTF("+EVT:ID=%s", dev_id);
	}
	return true;
}

void blues_hub_status(void)
{
	rak_blues.start_req((char *)"hub.status");
	if (!rak_blues.send_req(blues_response, 4096))
	{
		MYLOG("BLUES", "hub.status request failed");
	}
	AT_PRINTF("+EVT:%s", blues_response);
}

void blues_card_restore(void)
{
	rak_blues.start_req((char *)"hub.status");
	rak_blues.add_bool_entry((char *)"delete", true);
	rak_blues.add_bool_entry((char *)"connected", true);
	if (!rak_blues.send_req())
	{
		MYLOG("BLUES", "hub.status reset request failed");
	}
}
