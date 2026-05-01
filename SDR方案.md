下面这段可以直接复制到新窗口。新窗口发完后，再说：

**“继续从 7020-SDR / AD9361 双发双收方案，先把 PC 版双回环 multitone 自检调到接近 0m，然后再做网线 1-2 线对测长。”**

------

# 项目上下文摘要：以太网双绞线测试仪毕设，Length 模块从 AD9959/AD8310 切换到 7020-SDR/AD9361 双发双收方案

我正在做本科毕设《以太网双绞线测试仪的设计与实现》。任务书要求实现双绞线类型识别、线对连接关系测量、直流电阻测量、电缆长度、短路、PoE 等参数测量及显示。当前目标是尽快形成可演示、可写论文的工程样机。

整体系统方案原本是：

```text
ESP32-S3-N16R8 主控
+ TJC 串口屏
+ RJ45/SMA 引出夹具
+ 多个分时测试模块
```

已完成模块：

```text
1. Wire Map 线序检测
2. Cable Type 类型识别，目前做 UTP / Shielded 初步区分
3. DCR 直流电阻测量，已完成四线对手动实测
4. Short 短路检测，已完成任意两线短路检测
```

当前重点：

```text
Length 电缆长度测量模块
原 AD9959 + AD8310 方案已暂停
改用 7020-SDR / Z7020-AD9361 双发双收方案
目标：先做 1 组线对手动测长演示版
```

------

## 一、固定硬件与已知 GPIO / 接线

主控：

```text
ESP32-S3-N16R8
```

TJC 串口屏：

```text
ESP32 GPIO17 -> 屏 RX
ESP32 GPIO18 <- 屏 TX
Serial1.begin(115200, SERIAL_8N1, 18, 17);
```

Wire Map：

```text
RJ45 1 -> GPIO4
RJ45 2 -> GPIO5
RJ45 3 -> GPIO6
RJ45 4 -> GPIO8
RJ45 5 -> GPIO9
RJ45 6 -> GPIO7
RJ45 7 -> GPIO10
RJ45 8 -> GPIO11
```

Cable Type：

```text
GPIO12 = 屏蔽连续性检测输入 INPUT_PULLUP
```

DCR：

```text
GPIO1 = ADC 输入
Rref = 46.4 Ω
Vsrc = 3.08 V
Offset = 0.25 Ω
```

原 AD8310 ADC：

```text
GPIO2 = ADC 输入
TPVout 经 10k / 20k 分压后送入 GPIO2
```

注意：

```text
不要使用 GPIO35/36/37，因为 ESP32-S3-N16R8 可能和内部 PSRAM 相关。
```

------

## 二、AD9959 / AD8310 方案现状：暂停

原计划：

```text
AD9959 输出扫频 RF
AD8310 做对数检波
ESP32 ADC 采集 TPVout
通过幅频响应估算线长
```

AD9959 控制线：

```text
ESP32 GPIO13 → AD9959 SCK
ESP32 GPIO14 → AD9959 SD0
ESP32 GPIO15 → AD9959 CS
ESP32 GPIO16 → AD9959 UP / IO_UPDATE
ESP32 GPIO21 → AD9959 RST

AD9959 PDC → GND
AD9959 DC5V → 5V
AD9959 GND → ESP32 GND
P0/P1/P2/P3 → GND
SD1/SD2/SD3 → GND
```

AD9959 已验证：

```text
使用 PlutoSDR / AD9361 接收验证：
AD9959 CH0 输出 100.1 MHz
SDR 中心频率 100 MHz 时，在 +100 kHz 看到明显峰
改 AD9959 到 100.5 MHz 后，峰移动到 +500 kHz
PDC=3V3 或断开 AD9959 后，峰消失 / 明显下降

结论：
AD9959 有输出
ESP32 控制 AD9959 基本成功
```

AD8310 排查结果：

```text
AD8310 共有 2 块
两块都接 PlutoSDR TX1A 后，TPVout 万用表直接测量无明显变化
ESP32 GPIO2 采样也无明显变化
AD8310_OUT_est 长时间只在约 2.64V 附近小幅漂移
变化量约 0.01~0.02V，不到有效检波阈值
```

用 Python 控制 PlutoSDR 打 AD8310 的测试已经做过：

```text
TX LO = 400 MHz
Tone = 1 MHz
Tone Scale 从 -30 dBFS 到 -12 dBFS
TX Gain 从 -40 dB 到 -10 dB
AD8310 TPVout 仍然无明显变化
```

结论：

```text
PlutoSDR TX 正常
AD8310 两块均无有效响应
AD9959/AD8310 方案暂停
不要继续在 AD8310 上消耗时间
```

------

## 三、切换到 7020-SDR / AD9361 双发双收方案

新的 Length 方案：

```text
使用 7020-SDR / Z7020-AD9361
直接利用 AD9361 的双发双收能力
通过 IQ 相位差测量线缆传输时延
再换算线长
```

推荐测量结构：

```text
TX1A → 被测网线某一组线对 → RX1A      被测通道
TX2A → 短 SMA 回环线        → RX2A      参考通道
```

原理：

```text
TX1 / TX2 同时发相同信号
RX1 接收经过网线后的 IQ 信号
RX2 接收短回环参考 IQ 信号
计算 RX1 / RX2 的复数比值
观察相位随频率变化的斜率
相位斜率 → 群时延 → 线缆长度
```

长度估算公式：

```text
phase(f) ≈ intercept - 2πfτ

τ = -slope / (2π)

L = τ × VF × c

其中：
VF 为双绞线速度因子，先取 0.66，后续用已知长度网线标定。
```

------

## 四、Python / IIO / PlutoSDR 当前状态

Windows PC 上已经能通过 Python 访问 SDR。

`scan_iio.py` 扫描结果：

```text
ip:pluto.local => 192.168.2.1
usb:2.6.5
```

优先使用：

```python
URI = "ip:192.168.2.1"
```

单通道回环验证已成功：

```text
TX1A -> RX1A

Loopback result:
Peak frequency offset: 0.996 MHz
Peak power: 108.9 dB
Median power: 51.5 dB
Peak - Median: 57.4 dB
```

结论：

```text
PlutoSDR / AD9361 的 TX1A 正常发射
Python 配置有效
TX1A → RX1A 物理射频链路正常
```

------

## 五、双发双收测长脚本当前问题

尝试过逐频点扫频脚本：

```text
TX1A -> RX1A
TX2A -> RX2A
```

双回环自检输出异常：

```text
1 MHz 点较强
后续大多数频点幅度掉到 -35 ~ -58 dB
相位随机
最终估算：
Extra delay relative to reference = -32.099 ns
Estimated cable length = 6.351 m
```

判断：

```text
双回环本应接近 0m
现在算出 6.351m 是错误结果
不是线真的有 6m
而是逐频点 TX buffer 刷新或接收提取方式有问题
可能 tx_cyclic_buffer=True 后没有真正刷新到新频点
```

下一步应改为：

```text
multitone 一次性多音发射
一次采集 RX1/RX2
从接收数据中分别提取每个 tone 的复数幅相
再拟合相位斜率
```

当前下一步脚本名称建议：

```text
pluto_2x2_multitone_selftest.py
```

目标：

```text
先双回环：
TX1A -> RX1A
TX2A -> RX2A

要求：
Estimated extra length 接近 0m
幅度不要只有 1MHz 强、后面全弱
如果双回环不能接近 0m，不要接网线
```

------

## 六、SDR 板本机环境状态

已经登录到 SDR 板 Linux：

```text
Linux pluto 5.15.0 ... armv7l GNU/Linux
```

板上没有 Python：

```text
python3: not found
```

板上没有 gcc：

```text
gcc: not found
```

板上没有 iio.h：

```text
ls /usr/include/iio.h
No such file or directory
```

但板上有 libiio 运行库：

```text
/usr/lib/libiio.so
/usr/lib/libiio.so.0
/usr/lib/libiio.so.0.25
```

板上有 IIO 工具：

```text
/usr/bin/iio_readdev
/usr/bin/iio_writedev
/usr/bin/iio_attr
```

`iio_info -s` 显示可用 contexts：

```text
0: 169.254.9.243 ... [ip:pluto.local]
1: 192.168.2.1 ... [ip:pluto.local]
2: local: cf-ad9361-lpc, xadc, cf-ad9361-dds-core-lpc, ad9361-phy
```

本机 IIO 设备存在：

```text
/sys/bus/iio/devices/
iio:device0
iio:device1
iio:device2
iio:device3
```

注意：

```text
在 SDR 板本机执行 iio_attr / iio_info 时，需要加 -u local:
否则会提示 Multiple contexts found
```

例如：

```bash
iio_info -u local:
iio_attr -u local: -d ad9361-phy
iio_attr -u local: -d cf-ad9361-lpc
iio_attr -u local: -d cf-ad9361-dds-core-lpc
```

结论：

```text
SDR 板本机不能直接跑 Python
也不能直接板上编译 C
但具备 libiio 运行环境
最终“不用 PC”方案应走交叉编译 C 程序
```

------

## 七、最终不用 PC 的可行架构

不建议让 ESP32 直接控制 AD9361。

推荐架构：

```text
7020-SDR / AD9361：
负责射频发射、IQ 接收、相位拟合、长度计算

ESP32-S3：
负责 TJC 屏按钮、人机交互、测量触发、结果显示

TJC 串口屏：
显示测试状态和长度结果
```

完整流程：

```text
用户按 TJC 屏上的 Length 按钮
        ↓
ESP32 收到按钮事件
        ↓
ESP32 通过 UART / GPIO 通知 SDR：开始测长
        ↓
SDR 本机运行 length_measure 程序
        ↓
SDR 控制 AD9361 完成双通道测量
        ↓
SDR 计算长度
        ↓
SDR 返回 LEN,xx.xx
        ↓
ESP32 更新 TJC 屏显示 Length = xx.xx m
```

推荐通信协议：

```text
ESP32 -> SDR: START\n
SDR   -> ESP32: LEN,8.43\n
SDR   -> ESP32: ERR,NO_SIGNAL\n
```

论文表述建议：

```text
ESP32-S3 作为系统主控与人机交互控制器；
Z7020-AD9361 模块作为射频测量子系统；
两者通过串口或 GPIO 完成测量触发和结果回传。
```

不要写成：

```text
ESP32 直接驱动 AD9361
```

因为 AD9361 的高速 IQ 采集依赖 Zynq / Linux / IIO 驱动，ESP32 不适合直接承担这一层。

------

## 八、接下来工作重点

### 工作重点 1：先把 PC 版双回环 multitone 自检调通

接线：

```text
TX1A -> RX1A
TX2A -> RX2A
```

目标：

```text
Estimated extra length ≈ 0m
允许 0.1m~1m 内的固定偏差
不允许再出现 5m、10m 这种假结果
```

如果双回环不准：

```text
不要接网线
继续修 Python 算法
重点检查：
1. 多音信号是否真的同时发出
2. RX1/RX2 是否正确对应
3. 相位 unwrap 是否稳定
4. tone 提取是否准确
5. RX 是否饱和或信号过弱
```

### 工作重点 2：双回环成功后，接入 1-2 线对测长

接线：

```text
TX1A
  ↓
近端 RJ45 Pin1 / Pin2
  ↓
被测网线
  ↓
远端 RJ45 Pin1 / Pin2
  ↓
RX1A

TX2A -> RX2A 短回环保持不变
```

目标：

```text
换短线 → 结果较短
换长线 → 结果变大
先追求趋势正确
再用已知长度网线标定 VF_CABLE
```

### 工作重点 3：标定速度因子

初始：

```python
VF_CABLE = 0.66
```

如果已知长度为 10m，测得 8.5m，则：

```text
VF_new = VF_old × 已知长度 / 测得长度
       = 0.66 × 10 / 8.5
       = 0.776
```

更新脚本中的 `VF_CABLE`。

### 工作重点 4：PC 算法稳定后再移植到 SDR 板

由于 SDR 板：

```text
没有 python3
没有 gcc
有 libiio.so.0.25
```

所以最终应：

```text
在 PC / WSL 上写 C 程序
使用 arm-linux-gnueabihf-gcc 交叉编译
复制到 SDR 板运行
```

大致路线：

```bash
# WSL Ubuntu
sudo apt update
sudo apt install gcc-arm-linux-gnueabihf make cmake git scp

# 从 SDR 拷出运行库
scp root@192.168.2.1:/usr/lib/libiio.so.0.25 .

# 准备 iio.h 和 C 源码后交叉编译
arm-linux-gnueabihf-gcc length_measure.c -I./include -L. -liio -lm -o length_measure

# 复制到 SDR
scp length_measure root@192.168.2.1:/root/

# SDR 上运行
chmod +x /root/length_measure
/root/length_measure
```

但注意：

```text
在 PC 版 Python 算法没有稳定前，不要急着写 C。
否则只是把错误算法搬到 SDR 板上。
```

------

## 九、下一句继续工作时可以这样说

```text
继续从 7020-SDR / AD9361 双发双收方案开始。当前 AD9959/AD8310 方案已暂停：AD9959 已被 SDR 证明有输出，但两块 AD8310 用 Pluto TX1A 打入后，TPVout 万用表直接测量都无变化。现在改用 7020-SDR 的 AD9361 双发双收做长度测量。单通道 TX1A->RX1A 回环已成功，Peak-Median=57.4dB。逐频点双回环测长脚本失败，双短线却算出 6.351m，判断是 TX buffer 刷新或相位提取问题。下一步要先跑 multitone 一次性多音双回环自检脚本，把 TX1A->RX1A、TX2A->RX2A 的 Estimated extra length 调到接近 0m，然后再接网线 1-2 线对测长。SDR 板本机没有 python3/gcc/iio.h，但有 libiio.so.0.25 和 iio_readdev/iio_writedev/iio_attr，所以最终不用 PC 的方案应走交叉编译 C 程序到 SDR 板本机运行，ESP32 只负责触发和 TJC 显示。
```