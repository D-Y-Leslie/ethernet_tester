#include <Arduino.h>
#include <WiFi.h>
#include <ctype.h>
#include <string.h>

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#endif

/*
  ============================================================
  Ethernet Twisted-pair Tester
  ESP32-S3 + TJC UART Screen
  ------------------------------------------------------------
  功能：
    1) Wire Map 自动检测
    2) DCR 页面演示
    3) Cable Type 实测版（RJ45金属壳连续性）

  固定硬件：
    TJC:
      ESP32 GPIO17 -> 屏 RX
      ESP32 GPIO18 <- 屏 TX
      Serial1.begin(115200, SERIAL_8N1, 18, 17);

    Wire Map:
      T1 -> GPIO4
      T2 -> GPIO5
      T3 -> GPIO6
      T4 -> GPIO8
      T5 -> GPIO9
      T6 -> GPIO7
      T7 -> GPIO10
      T8 -> GPIO11

    Type(真实测量版):
      GPIO12 = 屏蔽连续性检测输入 INPUT_PULLUP
      近端RJ45金属壳 -> 1k -> GND
      远端RJ45金属壳 -> 1k -> GPIO12

      LOW  => SHIELDED
      HIGH => UTP
  ============================================================
*/

// ============================================================
// 前向声明
// ============================================================

void mainPageReset();
void wirePageReset();
void dcrPageReset();
void typePageReset();
void shortPageReset();
void lengthPageReset();
void poePageReset();

void enterWirePage();
void enterDcrPage();
void enterTypePage();
void enterShortPage();
void enterLengthPage();
void enterPoePage();

void clearResults();
void setupLinePins();
void releaseAllLines();
void runWireTest();
void runDcrTest();
void runTypeTest();
void runShortTest();
void runLengthTest();
void runLengthTestTcp();
void runLengthTestUart();
void runPoeTest();

uint8_t scanStableMask(uint8_t driveIdx);
bool quickCablePresent();
bool readSdrTcpLine(WiFiClient& client, String& line, uint32_t timeoutMs);
bool showLengthProtocolLine(const String& rawLine);
float readPoeAdcVoltageV(uint8_t pin);
float estimatePoeInputVoltageV(float adcV);
const char* poeStatusText(float va);
const char* poeModeText(float va);
void poeShowTesting();
void poeShowResult(float va, const String& vbText, const String& mode, const String& status);

// ============================================================
// 基本参数
// ============================================================

static const uint8_t NUM_LINES = 8;

// 逻辑线号
static const uint8_t lineNums[NUM_LINES] = {1, 2, 3, 4, 5, 6, 7, 8};

// GPIO 映射（按逻辑线号顺序）
static const uint8_t testPins[NUM_LINES] = {4, 5, 6, 8, 9, 7, 10, 11};

// 当前远端夹具期望配对
static const uint8_t expectMate[NUM_LINES] = {2, 1, 6, 5, 4, 3, 8, 7};

// TJC 串口
static const uint32_t TJC_BAUD = 115200;
static const int TJC_RX_PIN = 18;
static const int TJC_TX_PIN = 17;

// SDR result UART
static const uint32_t SDR_BAUD = 115200;
static const int SDR_RX_PIN = 39;
static const int SDR_TX_PIN = 40;

// SDR network trigger. Keep UART fallback until JP5 pinout is confirmed.
#ifndef SDR_WIFI_SSID
#define SDR_WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef SDR_WIFI_PASSWORD
#define SDR_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#ifndef SDR_TCP_HOST
#define SDR_TCP_HOST "192.168.2.1"
#endif

#ifndef SDR_TCP_PORT
#define SDR_TCP_PORT 9000
#endif

static const bool LENGTH_USE_SDR_TCP = true;
static const char* WIFI_SSID = SDR_WIFI_SSID;
static const char* WIFI_PASSWORD = SDR_WIFI_PASSWORD;
static const char* SDR_HOST = SDR_TCP_HOST;
static const uint16_t SDR_PORT = SDR_TCP_PORT;
static const uint32_t SDR_TCP_CONNECT_TIMEOUT_MS = 8000;
static const uint32_t SDR_RESPONSE_TIMEOUT_MS = 45000;

enum SdrWifiStatus : uint8_t {
  SDR_WIFI_READY = 0,
  SDR_WIFI_CONFIG_MISSING,
  SDR_WIFI_CONNECT_FAILED
};

SdrWifiStatus ensureSdrWifi();

// Type 页面：真实屏蔽连续性检测
static const int PIN_TYPE_SENSE = 12;
static const bool TYPE_USE_DEMO_JUMPER = false;   // false=真实版，true=杜邦线演示版
// ============================================================
// DCR 实测参数：1-2线对 ADC 版
// ============================================================

// GPIO4 已用于 Wire Map 的 T1，不再拿来做 ADC
// DCR 使用 GPIO1 作为独立 ADC 采样脚
static const int PIN_DCR_ADC = 1;

// 参考电阻实际值：先写 47.0，之后用万用表实测后改
static const float DCR_RREF_OHM = 46.4f;
static const float DCR_VSRC_V = 3.08f;
static const float DCR_OFFSET_OHM = 0.25f;

// ADC 平均采样次数
static const uint16_t DCR_ADC_SAMPLES = 120;

// ============================================================
// PoE Mode A voltage detect parameters
// ============================================================

static const int PIN_POE_ADC_A = 2;
static const bool POE_MODE_B_AVAILABLE = false;

static const float POE_DIVIDER_RATIO = 20.6078f;
static const float POE_BRIDGE_DROP_COMP_V = 1.2f;
static const float POE_ADC_NOISE_FLOOR_V = 0.05f;

static const float NO_POE_MAX_V = 5.0f;
static const float POE_PRESENT_MIN_V = 30.0f;
static const float POE_OVER_RANGE_V = 60.0f;

static const uint16_t POE_ADC_SAMPLES = 120;
static const uint16_t POE_ADC_SAMPLE_DELAY_US = 1000;

// ============================================================
// 防抖与扫描增强参数
// ============================================================

// 按钮防抖
static const uint32_t START_DEBOUNCE_MS = 600;

// 单次驱动稳定时间
static const uint8_t DRIVE_SETTLE_MS = 4;

// 每轮内部采样次数（对某一根线）
static const uint8_t INNER_SAMPLES = 8;

// 内部采样间隔
static const uint8_t INNER_SAMPLE_DELAY_MS = 2;

// 外层重复扫描轮数（同一根线重复扫几轮）
static const uint8_t OUTER_ROUNDS = 5;

// 外层扫描轮间隔
static const uint8_t OUTER_ROUND_GAP_MS = 3;

// UI 每步停留时间
static const uint16_t STEP_UI_DELAY_MS = 260;

// 全部释放后等待电平沉降
static const uint8_t RELEASE_SETTLE_MS = 3;

// ============================================================
// 页面对象名
// ============================================================

// Wire 页面对象
static const char* OBJ_T0 = "p_wire.t0";
static const char* OBJ_T1 = "p_wire.t1";
static const char* OBJ_T2 = "p_wire.t2";
static const char* OBJ_T3 = "p_wire.t3";
static const char* OBJ_T4 = "p_wire.t4";
static const char* OBJ_T7 = "p_wire.t7";
static const char* OBJ_T8 = "p_wire.t8";
static const char* OBJ_T9 = "p_wire.t9";

// DCR 页面对象
static const char* DCR_T0  = "p_dcr.t0";
static const char* DCR_T1  = "p_dcr.t1";
static const char* DCR_T2  = "p_dcr.t2";
static const char* DCR_T3  = "p_dcr.t3";
static const char* DCR_T4  = "p_dcr.t4";
static const char* DCR_T5  = "p_dcr.t5";
static const char* DCR_T6  = "p_dcr.t6";
static const char* DCR_T7  = "p_dcr.t7";
static const char* DCR_T8  = "p_dcr.t8";
static const char* DCR_T9  = "p_dcr.t9";
static const char* DCR_T10 = "p_dcr.t10";
static const char* DCR_T11 = "p_dcr.t11";
static const char* DCR_T12 = "p_dcr.t12";
static const char* DCR_T13 = "p_dcr.t13";
static const char* DCR_T14 = "p_dcr.t14";
static const char* DCR_T15 = "p_dcr.t15";

// Type 页面对象（与你现在页面一致：t0~t9）
static const char* TYPE_T0 = "p_type.t0";
static const char* TYPE_T1 = "p_type.t1";
static const char* TYPE_T2 = "p_type.t2";
static const char* TYPE_T3 = "p_type.t3";
static const char* TYPE_T4 = "p_type.t4";
static const char* TYPE_T5 = "p_type.t5";
static const char* TYPE_T6 = "p_type.t6";
static const char* TYPE_T7 = "p_type.t7";
static const char* TYPE_T8 = "p_type.t8";
static const char* TYPE_T9 = "p_type.t9";

// Short Detect page objects
static const char* SHORT_T0 = "p_short.t0";
static const char* SHORT_T1 = "p_short.t1";
static const char* SHORT_T2 = "p_short.t2";
static const char* SHORT_T3 = "p_short.t3";
static const char* SHORT_T4 = "p_short.t4";
static const char* SHORT_T5 = "p_short.t5";
static const char* SHORT_T6 = "p_short.t6";
static const char* SHORT_T7 = "p_short.t7";
static const char* SHORT_T8 = "p_short.t8";
static const char* SHORT_T9 = "p_short.t9";

// Length page objects
static const char* LENGTH_T0 = "p_length.t0";
static const char* LENGTH_T1 = "p_length.t1";
static const char* LENGTH_T2 = "p_length.t2";
static const char* LENGTH_T3 = "p_length.t3";
static const char* LENGTH_T4 = "p_length.t4";
static const char* LENGTH_T5 = "p_length.t5";
static const char* LENGTH_T6 = "p_length.t6";
static const char* LENGTH_T7 = "p_length.t7";
static const char* LENGTH_T8 = "p_length.t8";
static const char* LENGTH_T9 = "p_length.t9";

// PoE page objects
static const char* POE_T0 = "p_poe.t0";
static const char* POE_T1 = "p_poe.t1";
static const char* POE_T2 = "p_poe.t2";
static const char* POE_T3 = "p_poe.t3";
static const char* POE_T4 = "p_poe.t4";
static const char* POE_T5 = "p_poe.t5";
static const char* POE_T6 = "p_poe.t6";
static const char* POE_T7 = "p_poe.t7";
static const char* POE_T8 = "p_poe.t8";
static const char* POE_T9 = "p_poe.t9";
static const char* POE_T10 = "p_poe.t10";
static const char* POE_T11 = "p_poe.t11";

// MAIN 页面对象
static const char* MAIN_T0 = "main.t0";
static const char* MAIN_T1 = "main.t1";

// 左右脚位块
static const char* OBJ_PL[NUM_LINES] = {
  "p_wire.pl0", "p_wire.pl1", "p_wire.pl2", "p_wire.pl3",
  "p_wire.pl4", "p_wire.pl5", "p_wire.pl6", "p_wire.pl7"
};

static const char* OBJ_PR[NUM_LINES] = {
  "p_wire.pr0", "p_wire.pr1", "p_wire.pr2", "p_wire.pr3",
  "p_wire.pr4", "p_wire.pr5", "p_wire.pr6", "p_wire.pr7"
};

// ============================================================
// 颜色（565）
// ============================================================

static const uint16_t COL_WHITE   = 65535;
static const uint16_t COL_BLACK   = 0;
static const uint16_t COL_GREEN   = 2016;
static const uint16_t COL_RED     = 63488;
static const uint16_t COL_YELLOW  = 65504;
static const uint16_t COL_CYAN    = 2047;
static const uint16_t COL_GRAY    = 33840;
static const uint16_t COL_ORANGE  = 64512;

// ============================================================
// DCR 四线对手动测量状态
// ============================================================

static const uint8_t DCR_PAIR_COUNT = 4;

uint8_t dcrPairIndex = 0;   // 0=1-2, 1=3-6, 2=4-5, 3=7-8, 4=完成

float dcrPairR[DCR_PAIR_COUNT] = {0, 0, 0, 0};
bool dcrPairOk[DCR_PAIR_COUNT] = {false, false, false, false};

// ============================================================
// 页面状态
// ============================================================

enum UiPage : uint8_t {
  PAGE_MAIN = 0,
  PAGE_WIRE,
  PAGE_DCR,
  PAGE_TYPE,
  PAGE_SHORT,
  PAGE_LENGTH,
  PAGE_POE
};

UiPage currentPage = PAGE_MAIN;

// 页面进入/返回事件
bool wireEnterFlag = false;
bool dcrEnterFlag  = false;
bool typeEnterFlag = false;
bool shortEnterFlag = false;
bool lengthEnterFlag = false;
bool poeEnterFlag = false;
bool backFlag      = false;

// 启动事件
bool startFlag      = false;
bool dcrStartFlag   = false;
bool typeStartFlag  = false;
bool shortStartFlag = false;
bool lengthStartFlag = false;
bool poeStartFlag = false;

// 运行状态
bool wireBusy = false;
bool dcrBusy  = false;
bool typeBusy = false;
bool shortBusy = false;
bool lengthBusy = false;
bool poeBusy = false;

// 时间戳
uint32_t lastStartEventMs     = 0;
uint32_t lastDcrStartEventMs  = 0;
uint32_t lastTypeStartEventMs = 0;
uint32_t lastShortStartEventMs = 0;
uint32_t lastLengthStartEventMs = 0;
uint32_t lastPoeStartEventMs = 0;

// 串口接收字母流缓冲（更稳的事件解析）
String tjcAsciiBuf;

// ============================================================
// Wire Map 状态定义
// ============================================================

enum LineState : uint8_t {
  ST_IDLE = 0,
  ST_OK,
  ST_OPEN,
  ST_MISWIRE,
  ST_MULTI
};

struct LineResult {
  LineState state;
  uint8_t mateCount;
  uint8_t mates[NUM_LINES];
  uint8_t rawMask;
};

LineResult gResults[NUM_LINES];

// ============================================================
// 工具函数
// ============================================================

String escapeForTjc(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  return s;
}

void tjcEnd() {
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
}

void tjcCmd(const String& cmd) {
  Serial1.print(cmd);
  tjcEnd();
}

void tjcPage(const char* pageName) {
  tjcCmd(String("page ") + pageName);
}

void tjcSetText(const char* obj, const String& text) {
  tjcCmd(String(obj) + ".txt=\"" + escapeForTjc(text) + "\"");
}

void tjcSetPco(const char* obj, uint16_t color565) {
  tjcCmd(String(obj) + ".pco=" + String(color565));
}

void tjcSetBco(const char* obj, uint16_t color565) {
  tjcCmd(String(obj) + ".bco=" + String(color565));
}

void tjcSetStaSolid(const char* obj) {
  tjcCmd(String(obj) + ".sta=1");
}

void tjcSetBkcmdOff() {
  tjcCmd("bkcmd=0");
}

// ============================================================
// 页面切换
// ============================================================

void mainPageReset() {
  tjcPage("main");
  delay(60);

  tjcSetText(MAIN_T0, "Ethernet Cable Tester");
  tjcSetText(MAIN_T1, "READY");

  currentPage = PAGE_MAIN;
}

void enterWirePage() {
  wirePageReset();
  currentPage = PAGE_WIRE;
}

void enterDcrPage() {
  dcrPageReset();
  currentPage = PAGE_DCR;
}

void enterTypePage() {
  typePageReset();
  currentPage = PAGE_TYPE;
}

void enterShortPage() {
  shortPageReset();
  currentPage = PAGE_SHORT;
}

void enterLengthPage() {
  lengthPageReset();
  currentPage = PAGE_LENGTH;
}

void enterPoePage() {
  poePageReset();
  currentPage = PAGE_POE;
}

// ============================================================
// Wire Map 工具
// ============================================================

String lineStateText(LineState st) {
  switch (st) {
    case ST_OK:      return "OK";
    case ST_OPEN:    return "OPEN";
    case ST_MISWIRE: return "MISWIRE";
    case ST_MULTI:   return "MULTI";
    default:         return "IDLE";
  }
}

uint16_t lineStateColor(LineState st) {
  switch (st) {
    case ST_OK:      return COL_GREEN;
    case ST_OPEN:    return COL_RED;
    case ST_MISWIRE: return COL_ORANGE;
    case ST_MULTI:   return COL_CYAN;
    default:         return COL_WHITE;
  }
}

String resultMateText(const LineResult& r) {
  if (r.mateCount == 0) return "OPEN";

  String s = "";
  for (uint8_t i = 0; i < r.mateCount; i++) {
    if (i) s += ",";
    s += String(r.mates[i]);
  }
  return s;
}

String oneMapText(uint8_t idx, bool processed) {
  String s = String(lineNums[idx]) + "->";
  if (!processed) return s + "?";

  const LineResult& r = gResults[idx];
  if (r.mateCount == 0) return s + "OPEN";

  for (uint8_t i = 0; i < r.mateCount; i++) {
    s += String(r.mates[i]);
    if (i + 1 < r.mateCount) s += ",";
  }
  return s;
}

String buildMapString(uint8_t processedCount) {
  String s = "";
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    if (i) s += "  ";
    s += oneMapText(i, i < processedCount);
  }
  return s;
}

String buildSummary(uint8_t processedCount) {
  uint8_t ok = 0, op = 0, mis = 0, mul = 0;

  for (uint8_t i = 0; i < processedCount; i++) {
    switch (gResults[i].state) {
      case ST_OK: ok++; break;
      case ST_OPEN: op++; break;
      case ST_MISWIRE: mis++; break;
      case ST_MULTI: mul++; break;
      default: break;
    }
  }

  return "OK=" + String(ok) +
         " OPEN=" + String(op) +
         " MIS=" + String(mis) +
         " MULTI=" + String(mul);
}

bool isAllPass() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    if (gResults[i].state != ST_OK) return false;
  }
  return true;
}

void clearResults() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    gResults[i].state = ST_IDLE;
    gResults[i].mateCount = 0;
    gResults[i].rawMask = 0;
    memset(gResults[i].mates, 0, sizeof(gResults[i].mates));
  }
}

// ============================================================
// GPIO 控制
// ============================================================

void releaseAllLines() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    digitalWrite(testPins[i], LOW);
    pinMode(testPins[i], INPUT_PULLDOWN);
  }
}

void driveOneLine(uint8_t driveIdx) {
  releaseAllLines();
  delay(RELEASE_SETTLE_MS);

  pinMode(testPins[driveIdx], OUTPUT);
  digitalWrite(testPins[driveIdx], HIGH);
  delay(DRIVE_SETTLE_MS);
}

void setupLinePins() {
  releaseAllLines();
}

// ============================================================
// Wire Map UI
// ============================================================

void initPinBlocks() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    tjcSetStaSolid(OBJ_PL[i]);
    tjcSetStaSolid(OBJ_PR[i]);

    tjcSetText(OBJ_PL[i], String(lineNums[i]));
    tjcSetText(OBJ_PR[i], String(lineNums[i]));

    tjcSetBco(OBJ_PL[i], COL_WHITE);
    tjcSetBco(OBJ_PR[i], COL_WHITE);

    tjcSetPco(OBJ_PL[i], COL_BLACK);
    tjcSetPco(OBJ_PR[i], COL_BLACK);
  }
}

void resetPinBlocks() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    tjcSetBco(OBJ_PL[i], COL_WHITE);
    tjcSetBco(OBJ_PR[i], COL_WHITE);
    tjcSetPco(OBJ_PL[i], COL_BLACK);
    tjcSetPco(OBJ_PR[i], COL_BLACK);
  }
}

void highlightStep(uint8_t driveIdx, uint8_t hitMask, LineState st) {
  (void)st;
  resetPinBlocks();

  tjcSetBco(OBJ_PL[driveIdx], COL_GREEN);
  tjcSetPco(OBJ_PL[driveIdx], COL_BLACK);

  for (uint8_t i = 0; i < NUM_LINES; i++) {
    if (hitMask & (1 << i)) {
      tjcSetBco(OBJ_PR[i], COL_GREEN);
      tjcSetPco(OBJ_PR[i], COL_BLACK);
    }
  }
}

void showFinalBlocks() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    uint16_t c = lineStateColor(gResults[i].state);
    tjcSetBco(OBJ_PL[i], c);
    tjcSetBco(OBJ_PR[i], c);
    tjcSetPco(OBJ_PL[i], COL_BLACK);
    tjcSetPco(OBJ_PR[i], COL_BLACK);
  }
}

void wirePageReset() {
  tjcPage("p_wire");
  delay(60);

  tjcSetText(OBJ_T0, "Wire Map");
  tjcSetText(OBJ_T1, "READY");
  tjcSetText(OBJ_T2, "L: -");
  tjcSetText(OBJ_T3, "R: -");
  tjcSetText(OBJ_T4, "Press Start");
  tjcSetText(OBJ_T7, "1->?  2->?  3->?  4->?  5->?  6->?  7->?  8->?");
  tjcSetText(OBJ_T8, "WAIT");
  tjcSetText(OBJ_T9, "Fixture: 1-2 3-6 4-5 7-8");

  initPinBlocks();
  resetPinBlocks();
}

void wireShowStep(uint8_t idx) {
  const LineResult& r = gResults[idx];

  highlightStep(idx, r.rawMask, r.state);

  tjcSetText(OBJ_T1, "TESTING");
  tjcSetText(OBJ_T2, "L: " + String(lineNums[idx]));
  tjcSetText(OBJ_T3, "R: " + resultMateText(r));
  tjcSetText(OBJ_T4, oneMapText(idx, true));
  tjcSetText(OBJ_T7, buildMapString(idx + 1));
  tjcSetText(OBJ_T8, lineStateText(r.state));
  tjcSetText(OBJ_T9, "Checking Pin " + String(lineNums[idx]));
}

void wireShowFinal() {
  bool pass = isAllPass();

  showFinalBlocks();

  tjcSetText(OBJ_T1, "DONE");
  tjcSetText(OBJ_T2, "L: ALL");
  tjcSetText(OBJ_T3, "R: DONE");
  tjcSetText(OBJ_T4, buildSummary(NUM_LINES));
  tjcSetText(OBJ_T7, buildMapString(NUM_LINES));
  tjcSetText(OBJ_T8, pass ? "PASS" : "FAIL");
  tjcSetText(OBJ_T9, pass ? "Cable OK" : "Check cable / remote fixture");
}

// ============================================================
// DCR 页面
// ============================================================

const char* dcrPairName(uint8_t idx) {
  switch (idx) {
    case 0: return "1-2";
    case 1: return "3-6";
    case 2: return "4-5";
    case 3: return "7-8";
    default: return "--";
  }
}

const char* dcrPairResultObj(uint8_t idx) {
  switch (idx) {
    case 0: return DCR_T7;    // R12
    case 1: return DCR_T9;    // R36
    case 2: return DCR_T11;   // R45
    case 3: return DCR_T13;   // R78
    default: return DCR_T7;
  }
}

void dcrClearFourPairResults() {
  dcrPairIndex = 0;

  for (uint8_t i = 0; i < DCR_PAIR_COUNT; i++) {
    dcrPairR[i] = 0.0f;
    dcrPairOk[i] = false;
  }

  tjcSetText(DCR_T7,  "--");
  tjcSetText(DCR_T9,  "--");
  tjcSetText(DCR_T11, "--");
  tjcSetText(DCR_T13, "--");
}

void dcrShowCurrentPrompt() {
  if (dcrPairIndex < DCR_PAIR_COUNT) {
    tjcSetText(DCR_T3, dcrPairName(dcrPairIndex));
    tjcSetText(DCR_T5, "--");
    tjcSetText(DCR_T15, String("CONNECT ") + dcrPairName(dcrPairIndex) + " THEN START");
  } else {
    tjcSetText(DCR_T3, "--");
    tjcSetText(DCR_T5, "--");
    tjcSetText(DCR_T15, "ALL PAIRS FINISHED");
  }
}

void dcrPageReset() {
  tjcPage("p_dcr");
  delay(60);

  tjcSetText(DCR_T0,  "DC Resistance");
  tjcSetText(DCR_T1,  "WAIT TEST");

  tjcSetText(DCR_T2,  "PAIR");
  tjcSetText(DCR_T3,  "1-2");

  tjcSetText(DCR_T4,  "RLOOP");
  tjcSetText(DCR_T5,  "--");

  tjcSetText(DCR_T6,  "R12");
  tjcSetText(DCR_T7,  "--");

  tjcSetText(DCR_T8,  "R36");
  tjcSetText(DCR_T9,  "--");

  tjcSetText(DCR_T10, "R45");
  tjcSetText(DCR_T11, "--");

  tjcSetText(DCR_T12, "R78");
  tjcSetText(DCR_T13, "--");

  tjcSetText(DCR_T14, "INFO");
  tjcSetText(DCR_T15, "CONNECT 1-2 THEN START");

  dcrClearFourPairResults();
  dcrShowCurrentPrompt();
}

void dcrShowTesting() {
  tjcSetText(DCR_T1, "TESTING");

  tjcSetText(DCR_T3, dcrPairName(dcrPairIndex));
  tjcSetText(DCR_T5, "--");

  tjcSetText(DCR_T15, String("ADC SAMPLING ") + dcrPairName(dcrPairIndex));
}

void dcrShowManualOk() {
  tjcSetText(DCR_T1,  "TEST OK");

  tjcSetText(DCR_T3,  "1-2");
  tjcSetText(DCR_T5,  "1.84 OHM");
  tjcSetText(DCR_T7,  "4.63 V");
  tjcSetText(DCR_T9,  "0.177 V");
  tjcSetText(DCR_T11, "46.1 OHM");
  tjcSetText(DCR_T13, "MANUAL");

  tjcSetText(DCR_T15, "DCR LOOP TEST FINISHED");
}

void dcrPreparePins() {
  // DCR测试前释放 Wire Map 使用的8个GPIO，避免影响分压测量
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    pinMode(testPins[i], INPUT);
  }

  pinMode(PIN_DCR_ADC, INPUT);
  delay(80);
}

float readDcrAdcVoltageV() {
  uint32_t sumMv = 0;

  for (uint16_t i = 0; i < DCR_ADC_SAMPLES; i++) {
    sumMv += analogReadMilliVolts(PIN_DCR_ADC);
    delay(3);
  }

  float avgMv = (float)sumMv / (float)DCR_ADC_SAMPLES;
  return avgMv / 1000.0f;
}

void dcrShowMeasuredOk(float vcable, float rloop) {
  tjcSetText(DCR_T1,  "TEST OK");

  tjcSetText(DCR_T3,  "1-2");
  tjcSetText(DCR_T5,  String(rloop, 2) + " OHM");

  tjcSetText(DCR_T7,  String(DCR_VSRC_V, 2) + " V");
  tjcSetText(DCR_T9,  String(vcable, 3) + " V");

  tjcSetText(DCR_T11, String(DCR_RREF_OHM, 1) + " OHM");
  tjcSetText(DCR_T13, "ADC");

  tjcSetText(DCR_T15, "DCR ADC TEST FINISHED");
}

void dcrShowFail(const String& info, float vcable) {
  tjcSetText(DCR_T1,  "TEST FAIL");

  tjcSetText(DCR_T3,  "1-2");
  tjcSetText(DCR_T5,  "--");

  tjcSetText(DCR_T7,  String(DCR_VSRC_V, 2) + " V");
  tjcSetText(DCR_T9,  String(vcable, 3) + " V");

  tjcSetText(DCR_T11, String(DCR_RREF_OHM, 1) + " OHM");
  tjcSetText(DCR_T13, "ADC");

  tjcSetText(DCR_T15, info);
}

void runDcrTest() {
  dcrBusy = true;
  dcrStartFlag = false;

  Serial.println("====================================");
  Serial.println("DCR four-pair manual test start");

  // 如果四组已经完成，再按 Start 就重新开始一轮
  if (dcrPairIndex >= DCR_PAIR_COUNT) {
    Serial.println("DCR reset four-pair results");

    tjcSetText(DCR_T1, "WAIT TEST");
    dcrClearFourPairResults();
    dcrShowCurrentPrompt();

    dcrBusy = false;
    return;
  }

  dcrShowTesting();
  delay(300);

  dcrPreparePins();

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_DCR_ADC, ADC_11db);

  delay(100);

  float vcable = readDcrAdcVoltageV();

  Serial.print("PAIR   = ");
  Serial.println(dcrPairName(dcrPairIndex));

  Serial.print("ADC PIN = GPIO");
  Serial.println(PIN_DCR_ADC);

  Serial.print("VCABLE = ");
  Serial.print(vcable, 4);
  Serial.println(" V");

  // 开路判断
  if (vcable > DCR_VSRC_V * 0.85f) {
    tjcSetText(DCR_T1, "TEST FAIL");
    tjcSetText(DCR_T5, "--");
    tjcSetText(DCR_T15, String("OPEN ") + dcrPairName(dcrPairIndex) + ", CHECK WIRING");

    Serial.println("DCR FAIL: open loop or no remote short");
    Serial.println("DCR test end");
    Serial.println("====================================");

    dcrBusy = false;
    return;
  }

  float rloop = 0.0f;

  if (vcable > 0.003f) {
    float denominator = DCR_VSRC_V - vcable;

    if (denominator <= 0.01f) {
      tjcSetText(DCR_T1, "TEST FAIL");
      tjcSetText(DCR_T5, "--");
      tjcSetText(DCR_T15, "ADC ERROR");

      Serial.println("DCR FAIL: denominator too small");
      Serial.println("DCR test end");
      Serial.println("====================================");

      dcrBusy = false;
      return;
    }

    rloop = vcable * DCR_RREF_OHM / denominator;
  }

  // 零点修正
  rloop -= DCR_OFFSET_OHM;
  if (rloop < 0.0f) {
    rloop = 0.0f;
  }

  // 异常值判断
  if (rloop > 30.0f) {
    tjcSetText(DCR_T1, "TEST FAIL");
    tjcSetText(DCR_T5, "--");
    tjcSetText(DCR_T15, String("R TOO HIGH ") + dcrPairName(dcrPairIndex));

    Serial.print("DCR FAIL: Rloop too high = ");
    Serial.print(rloop, 2);
    Serial.println(" OHM");

    Serial.println("DCR test end");
    Serial.println("====================================");

    dcrBusy = false;
    return;
  }

  // 当前结果显示
  String rText = String(rloop, 2) + " OHM";

  tjcSetText(DCR_T1, "TEST OK");
  tjcSetText(DCR_T3, dcrPairName(dcrPairIndex));
  tjcSetText(DCR_T5, rText);

  // 写入对应的四线对结果位置：R12/R36/R45/R78
  tjcSetText(dcrPairResultObj(dcrPairIndex), rText);

  dcrPairR[dcrPairIndex] = rloop;
  dcrPairOk[dcrPairIndex] = true;

  Serial.print("VSRC   = ");
  Serial.print(DCR_VSRC_V, 3);
  Serial.println(" V");

  Serial.print("RREF   = ");
  Serial.print(DCR_RREF_OHM, 2);
  Serial.println(" OHM");

  Serial.print("OFFSET = ");
  Serial.print(DCR_OFFSET_OHM, 2);
  Serial.println(" OHM");

  Serial.print("RLOOP  = ");
  Serial.print(rloop, 3);
  Serial.println(" OHM");

  Serial.println("METHOD = ADC");
  Serial.println("OFFSET CAL APPLIED");

  // 进入下一组
  uint8_t finishedIndex = dcrPairIndex;
  dcrPairIndex++;

  if (dcrPairIndex < DCR_PAIR_COUNT) {
    tjcSetText(DCR_T1, "WAIT NEXT");
    tjcSetText(DCR_T15,
               String(dcrPairName(finishedIndex)) +
               " OK, CONNECT " +
               dcrPairName(dcrPairIndex));

    tjcSetText(DCR_T3, dcrPairName(dcrPairIndex));
    tjcSetText(DCR_T5, "--");

    Serial.print("NEXT   = ");
    Serial.println(dcrPairName(dcrPairIndex));
  } else {
    tjcSetText(DCR_T1, "FINISHED");
    tjcSetText(DCR_T3, "--");
    tjcSetText(DCR_T5, "--");
    tjcSetText(DCR_T15, "ALL PAIRS FINISHED");

    Serial.println("FINAL  = ALL PAIRS FINISHED");
  }

  Serial.println("DCR test end");
  Serial.println("====================================");

  dcrBusy = false;
}

// ============================================================
// Type 页面
// ============================================================

void typePageReset() {
  tjcPage("p_type");
  delay(60);

  tjcSetText(TYPE_T0, "Cable Type");
  tjcSetText(TYPE_T1, "WAIT TEST");

  tjcSetText(TYPE_T2, "SHIELD");
  tjcSetText(TYPE_T3, "--");

  tjcSetText(TYPE_T4, "TYPE");
  tjcSetText(TYPE_T5, "--");

  tjcSetText(TYPE_T6, "METHOD");
  tjcSetText(TYPE_T7, TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER" : "RJ45 SHELL");

  tjcSetText(TYPE_T8, "INFO");
  tjcSetText(TYPE_T9, TYPE_USE_DEMO_JUMPER ? "NO JUMPER=UTP, GND=SHIELDED" : "PRESS START");
}

void shortPageReset() {
  tjcPage("p_short");
  delay(60);

  tjcSetText(SHORT_T0, "Short Detect");
  tjcSetText(SHORT_T1, "WAIT TEST");

  tjcSetText(SHORT_T2, "RESULT");
  tjcSetText(SHORT_T3, "--");
  tjcSetPco(SHORT_T3, COL_WHITE);

  tjcSetText(SHORT_T4, "PAIRS");
  tjcSetText(SHORT_T5, "--");

  tjcSetText(SHORT_T6, "METHOD");
  tjcSetText(SHORT_T7, "GPIO SCAN");

  tjcSetText(SHORT_T8, "INFO");
  tjcSetText(SHORT_T9, "REMOVE REMOTE FIXTURE");
}

void lengthPageReset() {
  tjcPage("p_length");
  delay(60);

  tjcSetText(LENGTH_T0, "Cable Length");
  tjcSetText(LENGTH_T1, "WAIT TEST");

  tjcSetText(LENGTH_T2, "PAIR");
  tjcSetText(LENGTH_T3, "1-2");

  tjcSetText(LENGTH_T4, "LENGTH");
  tjcSetText(LENGTH_T5, "--");

  tjcSetText(LENGTH_T6, "METHOD");
  tjcSetText(LENGTH_T7, "SDR PHASE");

  tjcSetText(LENGTH_T8, "INFO");
  tjcSetText(LENGTH_T9, "PRESS START");
}

void poePageReset() {
  tjcPage("p_poe");
  delay(60);

  tjcSetText(POE_T0, "PoE Detect");
  tjcSetText(POE_T1, "WAIT TEST");

  tjcSetText(POE_T2, "PoE");
  tjcSetText(POE_T3, "--");

  tjcSetText(POE_T4, "MODE");
  tjcSetText(POE_T5, "--");

  tjcSetText(POE_T6, "VA");
  tjcSetText(POE_T7, "-- V");

  tjcSetText(POE_T8, "VB");
  tjcSetText(POE_T9, "N/A");

  tjcSetText(POE_T10, "STATUS");
  tjcSetText(POE_T11, "--");
}

void typeShowTesting() {
  tjcSetText(TYPE_T1, "TESTING");
  tjcSetText(TYPE_T3, "...");
  tjcSetText(TYPE_T5, "...");
  tjcSetText(TYPE_T7, TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER" : "RJ45 SHELL");
  tjcSetText(TYPE_T9, "CHECKING CABLE...");
}

void typeShowNoCable() {
  tjcSetText(TYPE_T1, "TEST FAIL");
  tjcSetText(TYPE_T3, "N/A");
  tjcSetText(TYPE_T5, "UNKNOWN");
  tjcSetText(TYPE_T7, TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER" : "RJ45 SHELL");
  tjcSetText(TYPE_T9, "NO CABLE / NO REMOTE");
}

void typeShowReadingShield() {
  tjcSetText(TYPE_T1, "TESTING");
  tjcSetText(TYPE_T3, "CHECK");
  tjcSetText(TYPE_T5, "CHECK");
  tjcSetText(TYPE_T7, TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER" : "RJ45 SHELL");
  tjcSetText(TYPE_T9, "READING SHIELD...");
}

bool typeReadShieldConnected() {
  uint8_t lowCount = 0;
  const uint8_t N = 20;

  pinMode(PIN_TYPE_SENSE, INPUT_PULLUP);

  for (uint8_t i = 0; i < N; i++) {
    if (digitalRead(PIN_TYPE_SENSE) == LOW) {
      lowCount++;
    }
    delay(2);
  }

  return (lowCount >= 15);
}

void typeShowResult(bool shieldConnected) {
  tjcSetText(TYPE_T1, "TEST OK");
  tjcSetText(TYPE_T7, TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER" : "RJ45 SHELL");

  if (shieldConnected) {
    tjcSetText(TYPE_T3, "CONTINUITY");
    tjcSetText(TYPE_T5, "SHIELDED");
    tjcSetText(TYPE_T9, "SHELL CONNECTED");
  } else {
    tjcSetText(TYPE_T3, "OPEN");
    tjcSetText(TYPE_T5, "UTP");
    tjcSetText(TYPE_T9, "NO SHELL CONTINUITY");
  }
}

// 先判定是否存在有效线缆/远端夹具，避免“没插线=UTP”
bool quickCablePresent() {
  for (uint8_t i = 0; i < NUM_LINES; i++) {
    uint8_t mask = scanStableMask(i);
    if (mask != 0) {
      return true;
    }
  }
  return false;
}

void runTypeTest() {
  typeBusy = true;
  typeStartFlag = false;

  Serial.println("====================================");
  Serial.println("Type test start");

  typeShowTesting();
  delay(250);

  typeShowReadingShield();
  delay(150);

  // 第一步：先直接读屏蔽连续性
  bool shieldConnected = typeReadShieldConnected();

  // 如果已经检测到屏蔽连续，直接判为 SHIELDED
  if (shieldConnected) {
    typeShowResult(true);

    Serial.println("TYPE METHOD = RJ45 SHELL");
    Serial.println("SHIELD      = CONTINUITY");
    Serial.println("TYPE        = SHIELDED");
    Serial.println("Type test end");
    Serial.println("====================================");

    typeBusy = false;
    return;
  }

  // 第二步：只有在没有检测到屏蔽连续时，再判断是否真的插了线
  bool cablePresent = true;

  if (!TYPE_USE_DEMO_JUMPER) {
    cablePresent = quickCablePresent();
  }

  if (!cablePresent) {
    typeShowNoCable();

    Serial.println("TYPE METHOD = RJ45 SHELL");
    Serial.println("SHIELD      = OPEN");
    Serial.println("CABLE       = NOT PRESENT");
    Serial.println("TYPE        = UNKNOWN");
    Serial.println("Type test end");
    Serial.println("====================================");

    typeBusy = false;
    return;
  }

  // 能检测到线缆存在，但没有屏蔽连续 => 判为 UTP
  typeShowResult(false);

  Serial.println("TYPE METHOD = RJ45 SHELL");
  Serial.println("SHIELD      = OPEN");
  Serial.println("CABLE       = PRESENT");
  Serial.println("TYPE        = UTP");
  Serial.println("Type test end");
  Serial.println("====================================");

  typeBusy = false;
}

// ============================================================
// 扫描防抖增强逻辑
// ============================================================

uint8_t scanOneRoundMask(uint8_t driveIdx) {
  uint8_t votes[NUM_LINES];
  memset(votes, 0, sizeof(votes));

  driveOneLine(driveIdx);

  for (uint8_t s = 0; s < INNER_SAMPLES; s++) {
    for (uint8_t j = 0; j < NUM_LINES; j++) {
      if (j == driveIdx) continue;
      if (digitalRead(testPins[j]) == HIGH) {
        votes[j]++;
      }
    }
    delay(INNER_SAMPLE_DELAY_MS);
  }

  releaseAllLines();
  delay(RELEASE_SETTLE_MS);

  uint8_t mask = 0;
  const uint8_t threshold = (INNER_SAMPLES + 1) / 2;

  for (uint8_t j = 0; j < NUM_LINES; j++) {
    if (j == driveIdx) continue;
    if (votes[j] >= threshold) {
      mask |= (1 << j);
    }
  }

  return mask;
}

uint8_t scanStableMask(uint8_t driveIdx) {
  uint8_t bitVotes[NUM_LINES];
  memset(bitVotes, 0, sizeof(bitVotes));

  for (uint8_t r = 0; r < OUTER_ROUNDS; r++) {
    uint8_t mask = scanOneRoundMask(driveIdx);

    for (uint8_t j = 0; j < NUM_LINES; j++) {
      if (mask & (1 << j)) {
        bitVotes[j]++;
      }
    }

    delay(OUTER_ROUND_GAP_MS);
  }

  uint8_t finalMask = 0;
  const uint8_t threshold = (OUTER_ROUNDS + 1) / 2;

  for (uint8_t j = 0; j < NUM_LINES; j++) {
    if (j == driveIdx) continue;
    if (bitVotes[j] >= threshold) {
      finalMask |= (1 << j);
    }
  }

  return finalMask;
}

void runShortTest() {
  shortBusy = true;
  shortStartFlag = false;

  bool shortMatrix[NUM_LINES][NUM_LINES];
  memset(shortMatrix, 0, sizeof(shortMatrix));

  String pairText = "";
  uint8_t pairCount = 0;

  Serial.println("====================================");
  Serial.println("Short test start");

  tjcSetText(SHORT_T1, "TESTING");
  tjcSetText(SHORT_T3, "--");
  tjcSetPco(SHORT_T3, COL_WHITE);
  tjcSetText(SHORT_T5, "--");
  tjcSetText(SHORT_T7, "GPIO SCAN");
  tjcSetText(SHORT_T9, "SCANNING WIRES...");

  releaseAllLines();
  delay(20);

  for (uint8_t i = 0; i < NUM_LINES; i++) {
    uint8_t mask = scanStableMask(i);

    Serial.print("Drive line ");
    Serial.print(lineNums[i]);
    Serial.print(" mask=0x");
    Serial.println(mask, HEX);

    for (uint8_t j = 0; j < NUM_LINES; j++) {
      if (j == i) continue;
      if ((mask & (1 << j)) == 0) continue;

      uint8_t a = i;
      uint8_t b = j;
      if (a > b) {
        uint8_t tmp = a;
        a = b;
        b = tmp;
      }

      if (!shortMatrix[a][b]) {
        shortMatrix[a][b] = true;
        shortMatrix[b][a] = true;
        pairCount++;

        String onePair = String(lineNums[a]) + "-" + String(lineNums[b]);
        if (pairText.length() > 0) {
          pairText += " ";
        }
        pairText += onePair;
      }
    }
  }

  releaseAllLines();

  if (pairCount == 0) {
    tjcSetText(SHORT_T1, "TEST OK");
    tjcSetText(SHORT_T3, "NO SHORT");
    tjcSetPco(SHORT_T3, COL_GREEN);
    tjcSetText(SHORT_T5, "--");
    tjcSetText(SHORT_T9, "NO WIRE-TO-WIRE SHORT");

    Serial.println("Short result: NO SHORT");
  } else {
    String displayPairs = pairText;
    if (displayPairs.length() > 20) {
      displayPairs = "MULTI SHORT";
    }

    tjcSetText(SHORT_T1, "TEST FAIL");
    tjcSetText(SHORT_T3, "SHORT");
    tjcSetPco(SHORT_T3, COL_RED);
    tjcSetText(SHORT_T5, displayPairs);
    tjcSetText(SHORT_T9, "CHECK SHORTED PAIRS");

    Serial.print("Short result: SHORT PAIRS ");
    Serial.println(pairText);
  }

  Serial.println("Short test end");
  Serial.println("====================================");

  shortBusy = false;
}

SdrWifiStatus ensureSdrWifi() {
  if (strlen(WIFI_SSID) == 0 || strcmp(WIFI_SSID, "YOUR_WIFI_SSID") == 0) {
    Serial.println("SDR TCP WiFi not configured");
    return SDR_WIFI_CONFIG_MISSING;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return SDR_WIFI_READY;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t startMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startMs < SDR_TCP_CONNECT_TIMEOUT_MS) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    return SDR_WIFI_READY;
  }

  Serial.print("WiFi status after timeout: ");
  Serial.println(WiFi.status());
  return SDR_WIFI_CONNECT_FAILED;
}

bool readSdrTcpLine(WiFiClient& client, String& line, uint32_t timeoutMs) {
  uint32_t startMs = millis();
  line = "";

  while (millis() - startMs < timeoutMs) {
    while (client.available()) {
      char c = (char)client.read();

      if (c == '\n' || c == '\r') {
        if (line.length() > 0) {
          return true;
        }
      } else if (line.length() < 48) {
        line += c;
      }
    }

    if (!client.connected() && !client.available()) {
      break;
    }

    delay(5);
  }

  return false;
}

bool showLengthProtocolLine(const String& rawLine) {
  String line = rawLine;
  line.trim();

  if (line.startsWith("LEN,")) {
    String value = line.substring(4);
    tjcSetText(LENGTH_T1, "TEST OK");
    tjcSetText(LENGTH_T5, value + " m");
    tjcSetText(LENGTH_T9, "SDR DONE");
    return true;
  }

  if (line.startsWith("ERR,")) {
    tjcSetText(LENGTH_T1, "TEST FAIL");
    tjcSetText(LENGTH_T5, "--");
    tjcSetText(LENGTH_T9, line);
    return true;
  }

  return false;
}

void runLengthTestTcp() {
  Serial.println("Length test SDR TCP start");
  tjcSetText(LENGTH_T9, "WAIT WIFI");

  SdrWifiStatus wifiStatus = ensureSdrWifi();
  if (wifiStatus != SDR_WIFI_READY) {
    tjcSetText(LENGTH_T1, "TEST FAIL");
    tjcSetText(LENGTH_T5, "--");
    if (wifiStatus == SDR_WIFI_CONFIG_MISSING) {
      tjcSetText(LENGTH_T9, "ERR,WIFI_CFG");
    } else {
      tjcSetText(LENGTH_T9, "ERR,WIFI_CONN");
    }
    Serial.println("SDR TCP WiFi failed");
    return;
  }

  WiFiClient client;
  tjcSetText(LENGTH_T9, "WAIT SDR");

  if (!client.connect(SDR_HOST, SDR_PORT)) {
    tjcSetText(LENGTH_T1, "TEST FAIL");
    tjcSetText(LENGTH_T5, "--");
    tjcSetText(LENGTH_T9, "ERR,CONNECT");
    Serial.println("SDR TCP connect failed");
    return;
  }

  client.print("START\n");
  Serial.println("SDR TCP TX: START");

  String line = "";
  if (readSdrTcpLine(client, line, SDR_RESPONSE_TIMEOUT_MS)) {
    Serial.print("SDR TCP RX: ");
    Serial.println(line);
    client.stop();

    if (showLengthProtocolLine(line)) {
      return;
    }

    tjcSetText(LENGTH_T1, "TEST FAIL");
    tjcSetText(LENGTH_T5, "--");
    tjcSetText(LENGTH_T9, "ERR,BAD_REPLY");
    return;
  }

  client.stop();
  tjcSetText(LENGTH_T1, "TEST FAIL");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T9, "ERR,TIMEOUT");
  Serial.println("SDR TCP RX timeout");
}

void runLengthTestUart() {
  Serial.println("Length test SDR UART start");
  tjcSetText(LENGTH_T9, "WAIT SDR");

  while (Serial2.available()) {
    Serial2.read();
  }

  Serial2.print("START\n");
  Serial.println("SDR UART TX: START");

  String line = "";
  uint32_t startMs = millis();
  const uint32_t timeoutMs = 15000;

  while (millis() - startMs < timeoutMs) {
    while (Serial2.available()) {
      char c = (char)Serial2.read();

      if (c == '\n' || c == '\r') {
        if (line.length() > 0) {
          Serial.print("SDR UART RX: ");
          Serial.println(line);

          if (showLengthProtocolLine(line)) {
            return;
          }

          line = "";
        }
      } else if (line.length() < 48) {
        line += c;
      }
    }

    delay(5);
  }

  tjcSetText(LENGTH_T1, "TEST FAIL");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T9, "ERR,TIMEOUT");
  Serial.println("SDR UART RX timeout");
}

void runLengthTest() {
  lengthBusy = true;
  lengthStartFlag = false;

  Serial.println("====================================");
  tjcSetText(LENGTH_T1, "TESTING");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T9, "WAIT SDR");

  if (LENGTH_USE_SDR_TCP) {
    runLengthTestTcp();
  } else {
    runLengthTestUart();
  }

  Serial.println("Length test end");
  Serial.println("====================================");
  lengthBusy = false;
}

float readPoeAdcVoltageV(uint8_t pin) {
  uint32_t sumMv = 0;

  for (uint16_t i = 0; i < POE_ADC_SAMPLES; i++) {
    sumMv += analogReadMilliVolts(pin);
    delayMicroseconds(POE_ADC_SAMPLE_DELAY_US);
  }

  return (float)sumMv / (float)POE_ADC_SAMPLES / 1000.0f;
}

float estimatePoeInputVoltageV(float adcV) {
  if (adcV < POE_ADC_NOISE_FLOOR_V) {
    return 0.0f;
  }

  return adcV * POE_DIVIDER_RATIO + POE_BRIDGE_DROP_COMP_V;
}

const char* poeStatusText(float va) {
  if (va > POE_OVER_RANGE_V) {
    return "OVER_RANGE";
  }

  if (va >= POE_PRESENT_MIN_V) {
    return "OK";
  }

  if (va <= NO_POE_MAX_V) {
    return "NO_POE";
  }

  return "UNSAFE";
}

const char* poeModeText(float va) {
  if (va >= POE_PRESENT_MIN_V && va <= POE_OVER_RANGE_V) {
    return "A";
  }

  return "UNKNOWN";
}

void poeShowTesting() {
  tjcSetText(POE_T1, "TESTING");
  tjcSetText(POE_T3, "--");
  tjcSetText(POE_T5, "--");
  tjcSetText(POE_T7, "-- V");
  tjcSetText(POE_T9, POE_MODE_B_AVAILABLE ? "-- V" : "N/A");
  tjcSetText(POE_T11, "--");
}

void poeShowResult(float va, const String& vbText, const String& mode, const String& status) {
  if (status == "OK") {
    tjcSetText(POE_T1, "TEST OK");
    tjcSetText(POE_T3, "YES");
  } else if (status == "NO_POE") {
    tjcSetText(POE_T1, "TEST OK");
    tjcSetText(POE_T3, "NO");
  } else {
    tjcSetText(POE_T1, "TEST FAIL");
    tjcSetText(POE_T3, "--");
  }

  tjcSetText(POE_T5, mode);
  tjcSetText(POE_T7, String(va, 1) + " V");
  tjcSetText(POE_T9, vbText);
  tjcSetText(POE_T11, status);
}

void runPoeTest() {
  poeBusy = true;
  poeStartFlag = false;

  Serial.println("====================================");
  Serial.println("PoE test start");

  poeShowTesting();
  delay(120);

  float va = estimatePoeInputVoltageV(readPoeAdcVoltageV(PIN_POE_ADC_A));
  String vbText = POE_MODE_B_AVAILABLE ? "-- V" : "N/A";
  String status = poeStatusText(va);
  String mode = poeModeText(va);

  poeShowResult(va, vbText, mode, status);

  Serial.print("POE_MAIN,VA=");
  Serial.print(va, 2);
  Serial.print(",VB=");
  Serial.print(vbText);
  Serial.print(",mode=");
  Serial.print(mode);
  Serial.print(",status=");
  Serial.println(status);
  Serial.println("PoE test end");
  Serial.println("====================================");

  poeBusy = false;
}

void evaluateMask(uint8_t idx, uint8_t mask) {
  LineResult& r = gResults[idx];
  r.state = ST_IDLE;
  r.mateCount = 0;
  r.rawMask = mask;
  memset(r.mates, 0, sizeof(r.mates));

  for (uint8_t j = 0; j < NUM_LINES; j++) {
    if (j == idx) continue;
    if (mask & (1 << j)) {
      r.mates[r.mateCount++] = lineNums[j];
    }
  }

  if (r.mateCount == 0) {
    r.state = ST_OPEN;
    return;
  }

  if (r.mateCount > 1) {
    r.state = ST_MULTI;
    return;
  }

  if (r.mates[0] == expectMate[idx]) {
    r.state = ST_OK;
  } else {
    r.state = ST_MISWIRE;
  }
}

void printOneResultToSerial(uint8_t idx) {
  const LineResult& r = gResults[idx];

  Serial.print("Line ");
  Serial.print(lineNums[idx]);
  Serial.print(" -> ");
  Serial.print(resultMateText(r));
  Serial.print("   [");
  Serial.print(lineStateText(r.state));
  Serial.print("]  mask=0x");
  Serial.println(r.rawMask, HEX);
}

// ============================================================
// 更稳的 TJC 事件处理
// 说明：直接读取字母流，实时匹配 wireenter / typestart / back
// ============================================================

void processAsciiTokenBuffer() {
  uint32_t now = millis();

  // 先匹配更长的词，避免 typestart 被 start 截走
  if (tjcAsciiBuf.endsWith("wireenter")) {
    wireEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] wireenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("dcrenter")) {
    dcrEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] dcrenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("typeenter")) {
    typeEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] typeenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("shortenter")) {
    shortEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] shortenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("lengthenter")) {
    lengthEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] lengthenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("poeenter")) {
    poeEnterFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] poeenter");
    return;
  }

  if (tjcAsciiBuf.endsWith("dcrstart")) {
    if (!dcrBusy && (now - lastDcrStartEventMs >= START_DEBOUNCE_MS)) {
      dcrStartFlag = true;
      lastDcrStartEventMs = now;
      Serial.println("[EVENT] dcrstart accepted");
    } else {
      Serial.println("[EVENT] dcrstart ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("typestart")) {
    if (!typeBusy && (now - lastTypeStartEventMs >= START_DEBOUNCE_MS)) {
      typeStartFlag = true;
      lastTypeStartEventMs = now;
      Serial.println("[EVENT] typestart accepted");
    } else {
      Serial.println("[EVENT] typestart ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("shortstart")) {
    if (!shortBusy && (now - lastShortStartEventMs >= START_DEBOUNCE_MS)) {
      shortStartFlag = true;
      lastShortStartEventMs = now;
      Serial.println("[EVENT] shortstart accepted");
    } else {
      Serial.println("[EVENT] shortstart ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("lengthstart")) {
    if (!lengthBusy && (now - lastLengthStartEventMs >= START_DEBOUNCE_MS)) {
      lengthStartFlag = true;
      lastLengthStartEventMs = now;
      Serial.println("[EVENT] lengthstart accepted");
    } else {
      Serial.println("[EVENT] lengthstart ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("poestart")) {
    if (!poeBusy && (now - lastPoeStartEventMs >= START_DEBOUNCE_MS)) {
      poeStartFlag = true;
      lastPoeStartEventMs = now;
      Serial.println("[EVENT] poestart accepted");
    } else {
      Serial.println("[EVENT] poestart ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("start")) {
    if (!wireBusy && (now - lastStartEventMs >= START_DEBOUNCE_MS)) {
      startFlag = true;
      lastStartEventMs = now;
      Serial.println("[EVENT] wire start accepted");
    } else {
      Serial.println("[EVENT] wire start ignored");
    }
    tjcAsciiBuf = "";
    return;
  }

  if (tjcAsciiBuf.endsWith("back")) {
    backFlag = true;
    tjcAsciiBuf = "";
    Serial.println("[EVENT] back");
    return;
  }
}

void checkTjcEvent() {
  while (Serial1.available()) {
    char c = (char)Serial1.read();

    if (c >= 'A' && c <= 'Z') {
      c = (char)(c - 'A' + 'a');
    }

    if (c >= 'a' && c <= 'z') {
      tjcAsciiBuf += c;

      if (tjcAsciiBuf.length() > 32) {
        tjcAsciiBuf.remove(0, tjcAsciiBuf.length() - 32);
      }

      processAsciiTokenBuffer();
    }
  }
}

// ============================================================
// 主测试流程
// ============================================================

void runWireTest() {
  wireBusy = true;
  startFlag = false;

  clearResults();
  wirePageReset();
  delay(80);

  Serial.println("====================================");
  Serial.println("Wire Map test start");

  for (uint8_t i = 0; i < NUM_LINES; i++) {
    uint8_t stableMask = scanStableMask(i);
    evaluateMask(i, stableMask);
    printOneResultToSerial(i);
    wireShowStep(i);
    delay(STEP_UI_DELAY_MS);
  }

  releaseAllLines();
  wireShowFinal();

  Serial.println(buildSummary(NUM_LINES));
  Serial.println(isAllPass() ? "FINAL: PASS" : "FINAL: FAIL");
  Serial.println("Wire Map test end");
  Serial.println("====================================");

  wireBusy = false;
}

// ============================================================
// Arduino
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial1.begin(TJC_BAUD, SERIAL_8N1, TJC_RX_PIN, TJC_TX_PIN);
  delay(200);

  Serial2.begin(SDR_BAUD, SERIAL_8N1, SDR_RX_PIN, SDR_TX_PIN);
  delay(100);

  setupLinePins();
  clearResults();

  pinMode(PIN_TYPE_SENSE, INPUT_PULLUP);
  pinMode(PIN_POE_ADC_A, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_POE_ADC_A, ADC_11db);

  delay(300);
  tjcSetBkcmdOff();
  delay(50);

  mainPageReset();

  Serial.println("System ready.");
  Serial.println("Main page ready.");
  Serial.print("Type mode: ");
  Serial.println(TYPE_USE_DEMO_JUMPER ? "DEMO JUMPER on GPIO12" : "REAL RJ45 SHELL CONTINUITY on GPIO12");
}

void loop() {
  checkTjcEvent();

  // 页面进入事件
  if (wireEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    wireEnterFlag = false;
    enterWirePage();
  }

  if (dcrEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    dcrEnterFlag = false;
    enterDcrPage();
  }

  if (typeEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    typeEnterFlag = false;
    enterTypePage();
  }

  if (shortEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    shortEnterFlag = false;
    enterShortPage();
  }

  if (lengthEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    lengthEnterFlag = false;
    enterLengthPage();
  }

  if (poeEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    poeEnterFlag = false;
    enterPoePage();
  }

  if (backFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy && !poeBusy) {
    backFlag = false;
    mainPageReset();
  }

  // 启动事件
  if (startFlag && !wireBusy) {
    runWireTest();
  }

  if (dcrStartFlag && !dcrBusy) {
    runDcrTest();
  }

  if (typeStartFlag && !typeBusy) {
    runTypeTest();
  }

  if (shortStartFlag && !shortBusy) {
    runShortTest();
  }

  if (lengthStartFlag && !lengthBusy) {
    runLengthTest();
  }

  if (poeStartFlag && !poeBusy) {
    runPoeTest();
  }

  delay(5);
}
