**============================================================
BMC W2：GPIO 深度篇（pinmux + SCU）總綱（世界觀日）
============================================================

Pinmux 本質

Pinmux = Pin Function Multiplexer

一根腳位可能屬於 UART/I2C/SGPIO/GPIO/SD/eSPI

Pinmux register 決定「腳位最終被路由給哪個模組」

若 pinmux 設錯：
→ GPIO driver 看不到它
→ SCU direction/data 全部無效
→ interrupt 永遠不會進來（因為腳位根本沒被分配給 GPIO）

SCU GPIO 控制重點 bit

DIR：0=input / 1=output

DATA：寫 output 值或讀 input 值

PULL-UP / PULL-DOWN：腳位預設的靜態狀態

OPEN-DRAIN：輸出是否只拉低、不推高

DRIVE STRENGTH：推動電流大小

DEBOUNCE：避免 button / PGOOD 抖動

INTERRUPT TYPE：rising/falling/both

Input Mode 世界觀

Debounce = 連續多次 sample 一致後才算有效

Interrupt routing = GPIO event → SCU → GIC → Kernel ISR

若 pinmux 沒設定成 GPIO → interrupt 無法路由

Output Mode 世界觀

Push-pull：能推高、能拉低 → LED、一般訊號

Open-drain：只能拉低，拉高靠外部電阻 → 用在 I2C / Alert

Drive Strength：訊號上升、下降速度的根本

Edge Trigger 實際 Timing

Edge = 需要「前一個 sample」＋「現在 sample」

Rising：0→1

Falling：1→0

Both：兩邊都觸發

Kernel Driver Path

DTS → 生成 pinctrl + interrupt mapping

Driver probe → 綁定 SCU register

User space → sysfs/gpiochip → 最終寫入 SCU

最終一切都是寫 register，不是 sysfs magic

============================================================**
============================================================
BMC  W2：GPIO 深度篇（pinmux + SCU）問題解析（世界觀日）
============================================================

1) SGPIO / GPIO / LPC / SD / eSPI 是什麼？
------------------------------------------------------------
- SGPIO：序列化 GPIO，伺服器大量 LED / FAN 狀態同步使用
- GPIO：最基本腳位
- LPC：舊式 BIOS / SuperIO 匯流排，被 eSPI 取代
- SD/SDIO：SD 卡與 WiFi/BT 模組介面
- eSPI：SPI 進階版，用於 BIOS ↔ BMC 的主要通訊
------------------------------------------------------------

2) open-drain / pull-up / pull-down（超淺白）
------------------------------------------------------------
open-drain：只能拉低（靠外部電阻拉高）
pull-up：沒人管 → 預設 1
pull-down：沒人管 → 預設 0
------------------------------------------------------------

3) pinmux 設錯 → interrupt 不會來
------------------------------------------------------------
腳位未被分配給 GPIO → 事件不會送到 GPIO controller → GIC → ISR
→ callback 永遠不會執行
------------------------------------------------------------

4) interrupt trigger type
------------------------------------------------------------
rising：0 → 1（上升沿）
falling：1 → 0（下降沿）
both：兩邊都觸發
------------------------------------------------------------

5) interrupt polarity（edge vs level）
------------------------------------------------------------
edge：只在變化瞬間觸發（事件）
level：維持該電位就一直觸發（fault）
------------------------------------------------------------

6) Kernel ISR 是什麼？
------------------------------------------------------------
ISR = Interrupt Service Routine
硬體事件 → GIC → ISR → driver handler → user space（epoll/poll）
------------------------------------------------------------

7) rise time / fall time
------------------------------------------------------------
- rise time：0 → 1 需要多久
- fall time：1 → 0 需要多久
決定訊號邊緣是否清晰，影響高速介面可靠度
------------------------------------------------------------

8) EMI / overshoot
------------------------------------------------------------
EMI：電磁干擾，訊號像天線亂射
Overshoot：電壓 overshoot，例如應到 3.3V 卻衝到 4.2V
→ 噪聲、邏輯錯誤、影響壽命
------------------------------------------------------------

9) 「從訊號走的路 debug」的真正含義
------------------------------------------------------------
正確 debug 順序：

Physical Pin
 → Pinmux
 → SCU（DIR / DATA / PULL / OD / DRV）
 → Interrupt → GIC
 → kernel driver（gpiochip）
 → user space

sysfs 是最外層 API，不能用來 debug 底層問題。
------------------------------------------------------------

============================================================
