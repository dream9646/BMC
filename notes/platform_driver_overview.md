======================================================================

  世界觀思維（必記）
  
======================================================================

  DTS = 宣告硬體資訊（地址、IRQ、clock、reset、pinmux）
  
  driver = 讀取 DTS → 啟動硬體（probe）
  
  probe = ioremap + IRQ + clock + reset + register subsystem
  
  sysfs/dev = probe 建立的結果，不是硬體天生存在的
  
  pinctrl = 決定這根腳位是 GPIO/I2C/PWM/JTAG（單週最重要概念）
  
  IRQ routing = interrupt-parent/interrupts 決定 interrupt flow
  
  reg = address length = memory map 的實際位置

======================================================================

  Debug 心法（未來你遇到任何問題都用這三行）
  
======================================================================

  1. DTS 寫對嗎？（compatible/reg/irq/pinctrl）

  2. probe 跑起來嗎？（printk/dev_info）

  3. register 有 mapping 成功嗎？（ioremap → readl）

  *只要你照這三步查，70% BMC 問題都能在 20 秒內定位。*

======================================================================
