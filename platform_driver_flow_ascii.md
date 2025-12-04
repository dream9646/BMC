===============================================================
 Device Tree → Platform Driver → Regmap → IRQ → Ready
===============================================================

┌─────────────────────────────────────────────────────────────┐
│                    Device Tree (DTS)                        │
│-------------------------------------------------------------│
│  node: mydev@1e6e2000                                      │
│    compatible = "vendor,mydev";                            │
│    reg = <0x1e6e2000 0x1000>;   // base address + length   │
│    interrupts = <24>;           // IRQ number              │
└───────────────┬─────────────────────────────────────────────┘
                │  (of/platform core parses DTS)
                ▼
┌─────────────────────────────────────────────────────────────┐
│        of_match_table / platform_device / platform_driver   │
│-------------------------------------------------------------│
│  1. DTS node → of_device_id (match by compatible)           │
│  2. 建立 platform_device（帶著 reg / irq 資訊）             │
│  3. 找到對應 platform_driver（.driver.name / of_match）     │
└───────────────┬─────────────────────────────────────────────┘
                │  (match 成功 → 呼叫 .probe())
                ▼
┌─────────────────────────────────────────────────────────────┐
│                 mydev_probe(struct platform_device *pdev)   │
│-------------------------------------------------------------│
│  1. platform_get_resource(..., IORESOURCE_MEM)  → reg range │
│  2. devm_ioremap_resource()          → 映射寄存器 base      │
│  3. platform_get_irq()               → 取得 IRQ 編號        │
│  4. clk / reset 控制（clk_prepare_enable / deassert）       │
│  5. devm_kzalloc()                   → 配置私有資料結構     │
│  6. devm_request_irq()               → 建立 IRQ handler     │
│  7. 子系統註冊（gpiochip / i2c adapter / hwmon / etc.）     │
└───────────────┬─────────────────────────────────────────────┘
                │  (probe 成功 → 裝置 ready)
                ▼
┌─────────────────────────────────────────────────────────────┐
│             /dev nodes / sysfs entries / adapters           │
│-------------------------------------------------------------│
│  • /sys/class/gpio/...                                     │
│  • /dev/i2c-X                                              │
│  • /sys/class/hwmon/...                                   │
└─────────────────────────────────────────────────────────────┘

===============================================================
 心法：DTS 負責「宣告硬體」，probe 負責「讓硬體真的活起來」。
===============================================================
