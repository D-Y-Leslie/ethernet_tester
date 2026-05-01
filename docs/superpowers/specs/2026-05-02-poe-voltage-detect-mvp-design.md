# PoE Voltage Detect MVP Design

更新时间：2026-05-02

## 1. 目标

在现有 ESP32-S3 + TJC 以太网双绞线测试仪上新增 PoE 检测的第一版能力。第一版只做安全检测、供电模式识别和电压显示，不承诺完整 IEEE 802.3af/at/bt 分类。

第一版输出：

```text
PoE: YES / NO
MODE: A / B / A+B / UNKNOWN
VA: xx.x V
VB: xx.x V
STATUS: OK / NO_POE / OVER_RANGE / UNSAFE
```

其中：

- `VA` 表示 Mode A 候选供电对之间的电压，即 RJ45 1/2 与 3/6 之间。
- `VB` 表示 Mode B 候选供电对之间的电压，即 RJ45 4/5 与 7/8 之间。
- `MODE_A+B` 只表示两组候选对都检测到高压，不等同于完整 802.3bt 判定。

## 2. 范围边界

本 MVP 不做以下内容：

- 不做 802.3af/at/bt 功率等级分类。
- 不做 PSE/PD 完整握手。
- 不给被测设备供电。
- 不让带电 PoE 进入现有 Wire Map RJ45 接口。
- 不直接把 48 V 级 PoE 电压接到 ESP32 GPIO/ADC。

标准主动式 PoE 交换机通常不会在未检测到 PD 签名时长期输出 48 V。因此本 MVP 的第一验收源是限流直流电源或已带电的 PoE injector/passive injector。若后续必须直接测试标准主动式 PoE 交换机，需要在第二阶段增加可控 PD 签名负载。

## 3. 推荐硬件架构

采用独立 PoE 测试口，不复用现有线序检测 RJ45。原因是现有 RJ45 的 1-8 芯已直接连到 GPIO4、GPIO5、GPIO6、GPIO8、GPIO9、GPIO7、GPIO10、GPIO11，用它接带电 PoE 有烧毁 ESP32 的风险。

推荐前端为“两路差分整流 + 高阻分压”：

```text
Mode A input:
  MVP wiring: RJ45 pin 1 -> bridge AC1
  MVP wiring: RJ45 pin 3 -> bridge AC2
  bridge + -> divider top
  bridge - -> ESP32 GND
  divider midpoint -> ESP32 ADC_A

Mode B input:
  MVP wiring: RJ45 pin 4 -> bridge AC1
  MVP wiring: RJ45 pin 7 -> bridge AC2
  bridge + -> divider top
  bridge - -> ESP32 GND
  divider midpoint -> ESP32 ADC_B
```

Mode A 的 1/2 与 3/6、Mode B 的 4/5 与 7/8 是供电模式上的线组概念。MVP 面包板阶段先取每组的一根代表线，避免把同一差分线对直接短在一起。若后续做成固定前端板，可把同组两根线分别通过高阻电阻汇合到同一个桥式整流输入点。

优点：

- 能测量 A/B 候选供电对之间的差分电压。
- 不依赖 PoE 源与 ESP32 的地参考关系。
- 对供电极性不敏感，避免普通对地分压在浮地源下产生 ADC 负压。
- 只需要 2 路 ADC，便于先在现有 ESP32-S3 板上落地。

代价：

- 第一版不能判断正负极性，因此不显示 `REVERSE`。
- 由于桥式整流有二极管压降，低压段读数需要校准；PoE 48 V 量级下影响较小。

## 4. 建议元件值

每个 Mode 通道使用一组桥式整流和一组分压。

推荐分压：

```text
R_TOP = 2 x 510 kOhm 串联，合计约 1.02 MOhm
R_BOTTOM = 56 kOhm
分压系数 K = (R_TOP + R_BOTTOM) / R_BOTTOM = 19.214
ADC 输入估算:
  48 V -> 2.50 V
  57 V -> 2.97 V
```

推荐保护和滤波：

```text
ADC midpoint -> 1 kOhm -> ESP32 ADC pin
ADC pin -> 100 nF -> ESP32 GND
ADC pin -> 3.3 V / GND 钳位保护，优先用低漏电 TVS 或肖特基钳位
桥式整流二极管耐压 >= 100 V
分压电阻耐压按串联电阻分担核算
```

功耗估算：

```text
57 V 输入时，分压电流约 57 / 1.076M = 53 uA
R_TOP 总功耗约 57^2 / 1.076M = 3.0 mW
单颗 510 kOhm 功耗约 1.5 mW
```

该功耗很低，但高阻前端容易受噪声影响，所以软件需要平均采样，并用 100 nF 做低通滤波。

## 5. ESP32 引脚

根据当前文件夹里的 `AD8310方案.md`、`SDR方案.md` 和 0502 阶段总结，GPIO2 和 GPIO13 不能再直接写成 PoE 默认脚：

```text
GPIO2  = 旧 AD8310 TPVout ADC
GPIO13 = 旧 AD9959 SCK
GPIO14 = 旧 AD9959 SD0
GPIO15 = 旧 AD9959 CS
GPIO16 = 旧 AD9959 UP / IO_UPDATE
GPIO21 = 旧 AD9959 RST
```

主程序当前没有使用 GPIO13，但如果旧 AD9959 排线还接在板子上，GPIO13/14/15/16/21 都应视为硬件占用。PoE 接线时应优先避开这些旧实验线，除非已经明确拆线并记录。

当前固定占用：

```text
Wire Map: GPIO4, GPIO5, GPIO6, GPIO8, GPIO9, GPIO7, GPIO10, GPIO11
TJC UART: GPIO18, GPIO17
SDR UART fallback: GPIO39, GPIO40
Type sense: GPIO12
DCR ADC: GPIO1
旧 AD8310 ADC: GPIO2
旧 AD9959 control: GPIO13, GPIO14, GPIO15, GPIO16, GPIO21
```

因此 PoE MVP 分两档推进。

第一档，内部 ADC 最小验证，只做 Mode A：

```text
PIN_POE_ADC_A = GPIO3
PIN_POE_ADC_B = not connected
```

GPIO3 在当前主程序和阶段文档中未被占用，适合先验证分压、二极管整流和电压换算。该档只能显示 `NO_POE / MODE_A / UNKNOWN`，不完整覆盖 Mode B。

第二档，完整 A/B 检测，推荐加外置 ADC：

```text
ADS1115 A0 = Mode A 分压点
ADS1115 A1 = Mode B 分压点
ADS1115 SDA = GPIO35
ADS1115 SCL = GPIO36
ADS1115 VDD = 3V3
ADS1115 GND = ESP32 GND
```

GPIO35/GPIO36 当前不在项目文档和主程序占用表里。使用 ADS1115 后，PoE A/B 两路不再消耗 ESP32 内部 ADC 脚，也能避开旧 AD8310/AD9959 线。若没有 ADS1115，又必须做完整 A/B，则需要先物理拆除旧 AD9959 排线，再在 `GPIO14/15/16` 中选一个作为 `PIN_POE_ADC_B`，但这不是推荐默认方案。

## 6. 电压换算

ADC 读数先转换为 ADC 脚电压，再换算为 PoE 候选对电压。

```text
adc_v = analogReadMilliVolts(pin) / 1000.0
poe_v = adc_v * POE_DIVIDER_RATIO + POE_BRIDGE_DROP_COMP
```

第一版常量：

```text
POE_DIVIDER_RATIO = 19.214
POE_BRIDGE_DROP_COMP = 1.2 V
POE_ADC_SAMPLES = 120
```

桥压降补偿只用于显示。模式判定应以换算后的 `poe_v` 为准，但阈值留足余量，避免二极管压降导致边界误判。

## 7. 模式判定

建议阈值：

```text
NO_POE_MAX_V = 5.0 V
POE_PRESENT_MIN_V = 30.0 V
POE_OVER_RANGE_V = 60.0 V
```

判定规则：

```text
if VA > 60 or VB > 60:
  STATUS = OVER_RANGE
  MODE = UNKNOWN
elif VA >= 30 and VB >= 30:
  STATUS = OK
  MODE = A+B
elif VA >= 30:
  STATUS = OK
  MODE = A
elif VB >= 30:
  STATUS = OK
  MODE = B
elif VA <= 5 and VB <= 5:
  STATUS = NO_POE
  MODE = UNKNOWN
else:
  STATUS = UNSAFE
  MODE = UNKNOWN
```

`UNSAFE` 用于显示 5-30 V 的不确定电压，例如接线错误、非标准电源、正在启动/掉电的电源或测试夹具不稳定。

## 8. 软件接入设计

主程序 `ethernet_tester1/ethernet_tester1.ino` 新增页面：

```text
PAGE_POE
poeEnterFlag
poeStartFlag
poeBusy
lastPoeStartEventMs
```

新增函数：

```text
void poePageReset();
void enterPoePage();
void runPoeTest();
float readPoeAdcVoltageV(uint8_t pin);
float readPoeInputVoltageV(uint8_t pin);
void poeShowTesting();
void poeShowResult(float va, float vb, const String& mode, const String& status);
```

新增 TJC 事件：

```text
main page PoE button: prints "poeenter",0
p_poe Start button: prints "poestart",0
p_poe Back button: prints "back",0
```

按当前 TJC 工程截图的写法，按钮事件建议直接使用 `printh` 发送小写 ASCII。每个按钮只在“按下事件”或“弹起事件”里填一处，不要两个事件同时填，避免重复触发。

```text
p_poe b0 Back event:
  printh 62 61 63 6B

p_poe b1 Start event:
  printh 70 6F 65 73 74 61 72 74

main page PoE enter button event:
  printh 70 6F 65 65 6E 74 65 72
```

十六进制对应关系：

```text
back     = 62 61 63 6B
poestart = 70 6F 65 73 74 61 72 74
poeenter = 70 6F 65 65 6E 74 65 72
```

事件匹配顺序中，`poeenter` 和 `poestart` 必须放在通用 `start` 之前，避免被截断为 Wire Map start。

## 9. TJC 页面对象

建议页面名：

```text
p_poe
```

建议控件布局按 800 x 480 页面放置：

```text
name  type    x    y    w    h    initial text
b0    button  30   30   100  50   Back
b1    button  670  30   100  50   Start
t0    text    260  45   280  40   PoE Detect
t1    text    300  145  200  40   WAIT TEST
t2    text    150  220  90   36   PoE
t3    text    250  220  130  36   --
t4    text    430  220  110  36   MODE
t5    text    540  220  160  36   --
t6    text    150  285  90   36   VA
t7    text    250  285  130  36   -- V
t8    text    430  285  90   36   VB
t9    text    540  285  160  36   -- V
t10   text    150  350  140  36   STATUS
t11   text    300  350  400  36   --
```

建议使用 `t0` 到 `t11`：

```text
t0: "PoE Detect"
t1: state, e.g. "WAIT TEST" / "TESTING" / "TEST OK" / "TEST FAIL"
t2: "PoE"
t3: "YES" / "NO" / "--"
t4: "MODE"
t5: "A" / "B" / "A+B" / "UNKNOWN" / "--"
t6: "VA"
t7: "xx.x V"
t8: "VB"
t9: "xx.x V"
t10: "STATUS"
t11: "OK" / "NO_POE" / "OVER_RANGE" / "UNSAFE"
```

如果 TJC 页面暂时只做了 `t0~t9`，代码可以先通过串口输出 `STATUS`，但正式 PoE 页面建议补齐 `t10/t11`，避免把电压和状态挤在同一个文本对象里。

## 10. 独立诊断草图

在合入主程序前，先创建独立草图验证硬件读数：

```text
poe_voltage_detect_diag/poe_voltage_detect_diag.ino
```

诊断草图串口输出格式：

```text
POE_DIAG,ms=<time>,adcA=<v>,adcB=<v>,VA=<v>,VB=<v>,mode=<mode>,status=<status>
```

内部 ADC / GPIO3 第一档验收顺序：

```text
1. 不接输入：VA 接近 0 V，VB 显示 N/A，status=NO_POE
2. 限流电源 12 V 接 Mode A：VA 显示约 12 V，status=UNSAFE
3. 限流电源 48 V 接 Mode A：VA 显示约 48 V，mode=A，status=OK
4. 限流电源 57 V 接 Mode A：GPIO3 前端 ADC 点不超过 3.3 V，mode=A，status=OK
5. 模拟超过 60 V 前先断开 ESP32，只用万用表确认 ADC 点不会超过 3.3 V
```

ADS1115 第二档验收顺序：

```text
1. 不接输入：VA/VB 接近 0 V，status=NO_POE
2. 限流电源 48 V 接 Mode A：VA 显示约 48 V，mode=A，status=OK
3. 限流电源 48 V 接 Mode B：VB 显示约 48 V，mode=B，status=OK
4. A/B 同时输入 48 V：mode=A+B，status=OK
```

## 11. 主程序验收

合入 `ethernet_tester1/ethernet_tester1.ino` 后验收：

```text
1. main 页面新增 PoE 入口，发送 poeenter 能进入 p_poe。
2. p_poe 页面按 Start，发送 poestart，页面显示 TESTING。
3. 不接 PoE：显示 PoE=NO，MODE=UNKNOWN，STATUS=NO_POE。
4. Mode A 输入 48 V：显示 PoE=YES，MODE=A，VA 约 48 V。
5. 若只完成 GPIO3 内部 ADC 第一档，VB 显示 N/A，Mode B 相关验收暂缓。
6. 若完成 ADS1115 第二档，Mode B 输入 48 V：显示 PoE=YES，MODE=B，VB 约 48 V。
7. 若完成 ADS1115 第二档，同时给 A/B 输入：显示 MODE=A+B。
8. 其他页面 Wire/DCR/Type/Short/Length 仍可进入，事件不互相抢占。
```

## 12. 后续阶段

第二阶段可选增强：

```text
1. 可控 25 kOhm PD 签名负载，用于让标准主动式 PoE 交换机进入供电。
2. 极性识别，用于显示 NORMAL / REVERSE。
3. 独立过压比较器或光耦隔离，提高前端安全边界。
4. 标定系数保存，把桥压降和分压误差写成校准参数。
```

第二阶段开始前，需要重新评估功耗、发热、PSE 行为和隔离安全，不应直接在第一版 MVP 代码上追加高压负载控制。

## 13. 设计决策记录

已放弃的方案：

```text
四路普通对地分压:
  优点是可以尝试判断极性。
  缺点是面对隔离浮地 PoE 时，某些 ADC 节点可能被拉到负压区，读数和保护都不稳。
  因此不作为第一版 MVP。

完整 PoE PD 控制器:
  优点是更接近标准 PoE 流程。
  缺点是需要额外芯片、负载和供电路径设计，超过当前答辩前的最小可执行范围。
  因此留到第二阶段。
```

最终决策：

```text
第一版采用独立 PoE 测试口 + 两路差分整流 + 高阻分压 + ESP32 ADC。
先用限流电源完成 0/12/48/57 V 验证，再合入 TJC 页面。
```
