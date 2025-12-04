File: drivers/gpio/gpio-aspeed.c   (Linux kernel)

1. 主要資料結構
   - struct aspeed_gpio
     • void __iomem *base;      // GPIO controller base register
     • struct gpio_chip gc;     // Linux GPIO framework interface
     • struct irq_chip  irqc;   // interrupt controller for GPIO
     • spinlock_t lock;         // 保護 register 存取

2. regmap / register layout
   - 多個 bank，每個 bank 有：
     • data in / data out
     • direction
     • interrupt enable / status
     • debounce / type / polarity (依 SoC 版本變化)

3. aspeed_gpio_probe()
   - 從 DTS 抓 reg → ioremap 成 base
   - 取得 IRQ（有些版本是一個 shared IRQ）
   - 初始化 struct aspeed_gpio
   - 設定 gpio_chip callback：
       gc.get / gc.set / gc.direction_input / gc.direction_output ...
   - 註冊 gpiochip 到 kernel：
       gpiochip_add_data()
   - 設定 irq_chip 結構，掛到每個 GPIO line：
       irq_set_chip_and_handler()
   - 啟動 debounce / edge detect 相關的 default 設定

4. 與 SCU / pinmux 的關係
   - pinmux 不一定在這支 driver 裡做，
     但這個 driver 假設：對應的 pin 已在 DTS/pinctrl 設成 GPIO mode。
   - 也就是說：**如果 pinmux 錯，這支 driver 再完美也救不了。**

5. Debug 觀點
   - 若 /sys/class/gpio/gpioXX 存在但腳位沒動：
     → 先看 aspeed_gpio_probe 是否成功
     → 再看 bank 對應 register 是否有變
     → 再回頭查 DTS pinmux。
