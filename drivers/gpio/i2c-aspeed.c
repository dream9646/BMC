File: drivers/i2c/busses/i2c-aspeed.c   (Linux kernel)

1. 主要資料結構
   - struct aspeed_i2c_bus
     • void __iomem *base;      // I2C controller registers
     • struct i2c_adapter adap; // Linux I2C bus interface
     • struct clk *bus_clk;     // 此 I2C 控制器使用的 clock
     • int irq;                 // interrupt number
     • 狀態機（state, msg, msg_idx, 等）

2. register / 功能
   - 控制 register：啟動 / 停止 bus、master/slave 模式
   - clock divider：SCL 速度
   - FIFO / command / status / interrupt flag
   - 有 clock stretching / arbitration lost / bus timeout bit

3. aspeed_i2c_probe()
   - 從 DTS 抓 reg → ioremap base
   - 從 DTS 抓 clocks / resets → enable controller
   - 取得 IRQ → 註冊中斷 handler
   - 初始化 struct aspeed_i2c_bus
   - 設定 i2c_adapter callback：
       master_xfer / functionality / algo 等
   - 將 adapter 註冊成一條 I2C bus：
       i2c_add_numbered_adapter()

4. 與 DTS 的關係
   - DTS 中的：
       compatible = "aspeed,ast2600-i2c-bus";
       reg = <...>;
       clocks = <...>;
       interrupts = <...>;
     會直接影響：
       • probe 能不能成功
       • I2C bus 編號（i2c-X）
       • 這條 bus 上能不能正常做 PMBus/SMBus。

5. Debug 觀點
   - PSU PMBus 不通時，要看：
     • 這條 bus 的 aspeed_i2c_probe() 有沒有成功？
     • SCL/SDA 在 scope 上的實際波形？
     • status register 裡是不是 clock stretch / arbitration lost？
