#pragma once

// Copy this file to wifi_config.h in the same Arduino sketch folder,
// then update these values for your bench network.
#define SDR_WIFI_SSID "YOUR_WIFI_SSID"
#define SDR_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Use the IP address of the SDR/Linux board running sdr_length_tcp_server.py.
#define SDR_TCP_HOST "192.168.2.1"
#define SDR_TCP_PORT 9000
