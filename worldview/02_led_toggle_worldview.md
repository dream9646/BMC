# LED Toggle - Worldview

1. LED is a GPIO device.
2. Toggling LED = toggling GPIO output.
3. GPIO driver uses DTS settings (bias, direction, function).
4. User space sysfs -> driver -> SoC -> LED.
5. LED toggle is the first "time-based" operation.
6. This is the base for future: blink patterns, debounce, interrupt.