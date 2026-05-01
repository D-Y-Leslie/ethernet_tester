import pathlib
import re
import unittest


SKETCH_PATH = (
    pathlib.Path(__file__).resolve().parents[1]
    / "ethernet_tester1"
    / "ethernet_tester1.ino"
)


class EthernetTesterLengthPageTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = SKETCH_PATH.read_text(encoding="utf-8")

    def test_length_page_is_registered_with_state_and_objects(self):
        self.assertRegex(
            self.source,
            r"PAGE_SHORT,\s*PAGE_LENGTH",
        )
        self.assertIn("bool lengthEnterFlag = false;", self.source)
        self.assertIn("bool lengthStartFlag = false;", self.source)
        self.assertIn("bool lengthBusy = false;", self.source)
        self.assertIn("uint32_t lastLengthStartEventMs = 0;", self.source)

        for idx in range(10):
            self.assertIn(f'static const char* LENGTH_T{idx} = "p_length.t{idx}";', self.source)

    def test_length_page_reset_initializes_tjc_fields(self):
        required_snippets = [
            'tjcPage("p_length");',
            'tjcSetText(LENGTH_T0, "Cable Length");',
            'tjcSetText(LENGTH_T1, "WAIT TEST");',
            'tjcSetText(LENGTH_T3, "1-2");',
            'tjcSetText(LENGTH_T5, "--");',
            'tjcSetText(LENGTH_T7, "SDR PHASE");',
            'tjcSetText(LENGTH_T9, "PRESS START");',
            "currentPage = PAGE_LENGTH;",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_length_events_are_parsed_before_generic_start(self):
        length_enter_at = self.source.index('endsWith("lengthenter")')
        length_start_at = self.source.index('endsWith("lengthstart")')
        generic_start_at = self.source.index('endsWith("start")')

        self.assertLess(length_enter_at, generic_start_at)
        self.assertLess(length_start_at, generic_start_at)
        self.assertIn("lengthEnterFlag = true;", self.source)
        self.assertIn("lengthStartFlag = true;", self.source)
        self.assertIn('Serial.println("[EVENT] lengthstart accepted");', self.source)

    def test_loop_enters_length_page_and_runs_stub(self):
        self.assertIn("if (lengthEnterFlag &&", self.source)
        self.assertIn("enterLengthPage();", self.source)
        self.assertIn("if (lengthStartFlag && !lengthBusy)", self.source)
        self.assertIn("runLengthTest();", self.source)

    def test_length_run_clears_start_and_busy_state(self):
        required_snippets = [
            "void runLengthTest()",
            "lengthBusy = true;",
            "lengthStartFlag = false;",
            "lengthBusy = false;",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_sdr_uart_constants_and_setup_are_present(self):
        self.assertIn("static const uint32_t SDR_BAUD = 115200;", self.source)
        self.assertIn("static const int SDR_RX_PIN = 39;", self.source)
        self.assertIn("static const int SDR_TX_PIN = 40;", self.source)
        self.assertIn("Serial2.begin(SDR_BAUD, SERIAL_8N1, SDR_RX_PIN, SDR_TX_PIN);", self.source)

    def test_length_run_can_trigger_sdr_over_tcp_network(self):
        required_snippets = [
            "#include <WiFi.h>",
            '#if __has_include("wifi_config.h")',
            '#include "wifi_config.h"',
            "static const bool LENGTH_USE_SDR_TCP = true;",
            "#ifndef SDR_WIFI_SSID",
            "static const char* WIFI_SSID = SDR_WIFI_SSID;",
            "static const char* SDR_HOST = SDR_TCP_HOST;",
            "static const uint16_t SDR_PORT = SDR_TCP_PORT;",
            "WiFiClient client;",
            "ensureSdrWifi()",
            "client.connect(SDR_HOST, SDR_PORT)",
            'client.print("START\\n");',
            'Serial.println("SDR TCP TX: START");',
            "readSdrTcpLine(client, line, SDR_RESPONSE_TIMEOUT_MS)",
            "showLengthProtocolLine(line)",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_length_wifi_errors_are_distinguishable_on_screen(self):
        required_snippets = [
            "enum SdrWifiStatus",
            "SDR_WIFI_CONFIG_MISSING",
            "SDR_WIFI_CONNECT_FAILED",
            "SdrWifiStatus ensureSdrWifi()",
            'tjcSetText(LENGTH_T9, "ERR,WIFI_CFG");',
            'tjcSetText(LENGTH_T9, "ERR,WIFI_CONN");',
            'Serial.print("WiFi status after timeout: ");',
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_length_run_uses_start_len_err_timeout_protocol(self):
        required_snippets = [
            'tjcSetText(LENGTH_T1, "TESTING");',
            'tjcSetText(LENGTH_T9, "WAIT SDR");',
            "while (Serial2.available())",
            'Serial2.print("START\\n");',
            "const uint32_t timeoutMs = 15000;",
            'line.startsWith("LEN,")',
            "String value = line.substring(4);",
            'tjcSetText(LENGTH_T1, "TEST OK");',
            'tjcSetText(LENGTH_T5, value + " m");',
            'tjcSetText(LENGTH_T9, "SDR DONE");',
            'line.startsWith("ERR,")',
            'tjcSetText(LENGTH_T1, "TEST FAIL");',
            'tjcSetText(LENGTH_T9, line);',
            'tjcSetText(LENGTH_T9, "ERR,TIMEOUT");',
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

        self.assertNotIn("PC SDR READY", self.source)


if __name__ == "__main__":
    unittest.main()
