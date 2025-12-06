# How GPIO → LED → Driver → DTS connect

1. GPIO is a pin.
2. LED is a device connected to a GPIO pin.
3. GPIO behavior is controlled by kernel driver (gpiochip).
4. User space uses sysfs (deprecated) or character device.
5. Driver reads DTS to know pin function and mapping.
6. DTS decides pinmux, direction, pull-up, alternate function.

User Space (sysfs)  
↓  
Kernel GPIO driver  
↓  
Pinctrl / pinmux controller  
↓  
SoC register  
↓  
Physical pin / LED  

【BMC 五層】                       【GPIO 五層】

Application (CLI / Web / Redfish)   ←─ User Space (sysfs/libgpiod)
       │
Daemon (ledd / gpiod / psud)
       │
Middleware (Param Table / Mapping)  ←─（GPIO 無這層）
       │
Driver (GPIO / I2C / CPLD driver)   ←─ Kernel GPIO driver
       │
Hardware (SoC / CPLD / LED)         ←─ Pinctrl → Reg → Physical Pin

GPIO 五層主要描述 signal path：user space → kernel → pinctrl → register → LED。
但 BMC 五層描述的是整體 architecture，包括 daemon、mapping、driver、硬體。
GPIO 五層算是 BMC 五層裡 Driver + Hardware 的詳細拆解。

關鍵字: direction=in/out 本質差異
內容:
- in = 感測 → read-only → Hi-Z，不會影響外部電路
- out = 控制 → write-only(+read) → SoC 主動輸出
- in 用於 detect；out 用於 control
- 方向決定資料流：外→內（in），內→外（out）