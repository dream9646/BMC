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