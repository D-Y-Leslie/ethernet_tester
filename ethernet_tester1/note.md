# Ethernet Twisted-pair Tester 项目说明

本科毕设：《以太网双绞线测试仪的设计与实现》

当前目标：新增 Short Detect 短路检测模块。

## 已完成模块
- Wire Map 已完成
- Cable Type 已完成
- DCR 实测已完成

## 固定 GPIO
TJC:
- GPIO17 -> 屏 RX
- GPIO18 <- 屏 TX
- Serial1.begin(115200, SERIAL_8N1, 18, 17)

Wire Map:
- RJ45 1 -> GPIO4
- RJ45 2 -> GPIO5
- RJ45 3 -> GPIO6
- RJ45 4 -> GPIO8
- RJ45 5 -> GPIO9
- RJ45 6 -> GPIO7
- RJ45 7 -> GPIO10
- RJ45 8 -> GPIO11

Cable Type:
- GPIO12 INPUT_PULLUP

DCR:
- GPIO1 ADC
- Rref = 46.4 ohm
- Vsrc = 3.08 V
- offset = 0.25 ohm

## Short Detect 需求
- 页面名：p_short
- 分辨率：800×480
- 使用 b0 Back、b1 Start、t0~t9
- b0 弹起事件：prints "back",0
- b1 弹起事件：prints "shortstart",0
- main 进入 Short 页面按钮事件：prints "shortenter",0

Short 检测逻辑：
- 复用 Wire Map 的 8 根 GPIO
- 远端保持开路
- 依次驱动每根线 HIGH
- 读取其他 7 根线
- 若非当前驱动线也读到 HIGH，则判定线间短路
- 不要破坏 Wire Map、DCR、Cable Type 已有功能