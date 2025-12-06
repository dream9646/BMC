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