======================================================================

                  Platform Driver Worldview (AST2600)
                  
                Device Tree → Probe → Regmap → IRQ → Ready
                
======================================================================

                     ┌────────────────────────┐
                     │   Device Tree (DTS)    │
                     │------------------------│
                     │ compatible="aspeed,x"  │
                     │ reg=<base length>      │
                     │ interrupts=<irq n>     │
                     │ clocks / resets        │
                     │ pinctrl-0 = <&foo>     │
                     └─────────────┬──────────┘
                                   │  (match)
                 of_match_table    │
                                   ▼
                     ┌────────────────────────┐
                     │  platform_device       │
                     │------------------------│
                     │ carries: reg/irq info  │
                     │ assigned by DTS parse  │
                     └─────────────┬──────────┘
                                   │  (bind)
                     platform_driver.of_match
                                   ▼
                     ┌────────────────────────┐
                     │   driver.probe()       │
                     │------------------------│
                     │ 1. platform_get_resource(IORESOURCE_MEM)
                     │ 2. devm_ioremap_resource() → base
                     │ 3. platform_get_irq() → irq
                     │ 4. clk_prepare_enable()
                     │ 5. reset_control_deassert()
                     │ 6. devm_kzalloc() → private data
                     │ 7. devm_request_irq(handler)
                     │ 8. register subsystem (gpiochip/i2c/...)
                     └─────────────┬──────────┘
                                   │  (init OK)
                                   ▼
                     ┌────────────────────────┐
                     │   Kernel Subsystem     │
                     │------------------------│
                     │ GPIO:  gpiochip_add()  │
                     │ I2C:   i2c_add_adapter │
                     │ HWMON: sensor register │
                     │ LED:   LED classdev    │
                     └─────────────┬──────────┘
                                   │
                                   ▼
                     ┌────────────────────────┐
                     │ User-visible Interface │
                     │------------------------│
                     │ /sys/class/gpio        │
                     │ /dev/i2c-*             │
                     │ /sys/class/hwmon       │
                     │ /dev/mydev             │
                     └────────────────────────┘

