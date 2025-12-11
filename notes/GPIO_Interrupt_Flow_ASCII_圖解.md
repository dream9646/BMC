============================================================

GPIO Interrupt Flow：Pin → IRQ → ISR 全流程 ASCII 圖解（正式版）

============================================================

1) Interrupt 全流程概念（總覽）
------------------------------------------------------------
GPIO interrupt 不是「腳位跳動」那麼簡單，而是一條完整的事件路徑：

  Physical Pin
    → Pinmux（決定歸屬）
    → GPIO Controller（偵測 edge/level）
    → IRQ line（bank 對應 IRQ number）
    → GIC（Generic Interrupt Controller）
    → CPU interrupt entry
    → Kernel ISR handler
    → Driver callback（若有）
    → User space（epoll/poll/signal）

世界觀重點：
- GPIO interrupt = 一條跨越硬體 × OS 的完整事件流。
- 任何一步錯誤都會導致「中斷不來」或「中斷風暴」。
------------------------------------------------------------

2) ASCII 流程圖（Pin → IRQ → ISR）
------------------------------------------------------------
             +-----------------------+
             |    Physical Pin       |
             |   (電壓上升/下降)      |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |        Pinmux         |
             | (決定功能：GPIO/I2C..) |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |    GPIO Controller    |
             |  - Edge detect        |
             |  - Level detect       |
             |  - Interrupt enable   |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |      IRQ Line         |
             | (bank 對應 IRQ number)|
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |         GIC           |
             | (Interrupt routing)   |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |    CPU Interrupt      |
             |     Entry Point       |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |    Linux ISR (irq)    |
             | request_irq() 綁定函式 |
             +-----------+-----------+
                         |
                         v
             +-----------+-----------+
             |  Driver Handler/Task  |
             | - 清 flag             |
             | - 狀態更新            |
             +-----------+-----------+
                         |
                         v
             +-----------------------+
             |     User Space API    |
             | epoll / poll / sysfs  |
             +-----------------------+

------------------------------------------------------------

3) Trigger Type（邊緣/電平）的真實行為
------------------------------------------------------------
1. Rising edge：0 → 1  
   - 常見於按鍵放開、信號跳升、PGOOD 拉高。

2. Falling edge：1 → 0  
   - 常見於按鍵壓下、Fault pin、Alarm pin。

3. Both edge：兩種變化都觸發  
   - 常用於 fan tach, encoder。

4. Level trigger：維持該狀態就算中斷
   - High level：pin=1 → 一直有效
   - Low level：pin=0 → 一直有效

錯誤設定會導致：
- 事件來晚（edge 相反）
- 中斷永不觸發（level 設錯 polarity）
- 無限觸發（level 無法清除 pending bit）
------------------------------------------------------------

4) Interrupt Polarity（active high / active low）
------------------------------------------------------------
Polarity 決定「哪個電位代表有效」。

- Active high：pin = 1 → 表示中斷
- Active low：pin = 0 → 表示中斷

常見配合方式：
- Falling edge + active low（Button、Fault）
- Rising edge + active high（Power Good）

錯誤設定會導致：
- IRQ 無法清除 → 中斷風暴
- IRQ 永不成立 → ISR 完全沒被呼叫
------------------------------------------------------------

5) IRQ Mapping（bank → IRQ number）
------------------------------------------------------------
GPIO controller 不是「每個 pin 都有 IRQ」。

它是：

- 每個 bank（A/B/C…）有一條 IRQ output line  
- 每個 bank → 對應一個 IRQ number（DTS 定義）
- 在 bank 裡，由 interrupt status bit 判斷哪一個 pin 觸發

示例（假設值）：
  GPIOA → IRQ 40  
  GPIOB → IRQ 41  
  GPIOC → IRQ 42  

當 GPIOB3 觸發中斷：
- GPIOB bank 把 IRQ 41 拉高  
- GIC 收到 IRQ 41  
- Kernel ISR(41) 被呼叫  
- Handler 再檢查 GPIOB 的 status bit → 確認 B3  

這就是「IRQ number ≠ pin number」的原因。
------------------------------------------------------------

6) Kernel ISR（Interrupt Service Routine）
------------------------------------------------------------
ISR 是 CPU 接到 IRQ 時執行的最底層函式。

流程：
1. 儲存 CPU 狀態  
2. 呼叫對應的 ISR（你在 driver request_irq() 綁定的）  
3. 清除 interrupt pending bit  
4. 結束後返回被打斷的程式  

示例 handler（最小、純示意）:

```c
static irqreturn_t gpio_isr(int irq, void *dev) {
  pr_info("IRQ %d triggered\n", irq);
  // Clear interrupt status here (SCU or controller reg)
  return IRQ_HANDLED;
}
