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

============================================================
W2：GPIO 深層世界觀（Pinmux × SCU × Interrupt × Driver）
============================================================

1) GPIO 在 SoC 中的位置（世界觀）
------------------------------------------------------------
- GPIO 不是一根線，而是一整條「事件通道」。
- 它橫跨硬體與軟體六層：

  Physical Pin
    → Pinmux（決定腳位歸屬）
    → SCU（方向 / 電氣 / 驅動 / 中斷）
    → GPIO Controller（bank/bit）
    → GIC（中斷匯流排）
    → Kernel Driver（抽象化 API）
    → User Space（sysfs / ioctl / gpiochip）

- 若任何一層有錯 → ↯ 整條鏈就中斷。

世界觀結論：
    GPIO = 一條完整的 data path，不是 sysfs 的那顆檔案。
------------------------------------------------------------

2) GPIO 全流程 ASCII 架構圖
------------------------------------------------------------
            +-------------------+
            |   Physical Pin    |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |    Pinmux MUX     |
            |  (GPIO / I2C...)  |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |        SCU        |
            | DIR / DATA / PULL |
            | OD / DRV / DEB    |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |  GPIO Controller  |
            |  bank / bit map   |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |        GIC        |
            | Interrupt Routing |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |   Kernel Driver   |
            | gpiochip / ISR    |
            +---------+---------+
                      |
                      v
            +---------+---------+
            |    User Space     |
            | sysfs / ioctl     |
            +-------------------+

世界觀結論：
    GPIO 的行為 ≠ sysfs 的行為
    GPIO 的真實行為 = 上面整條路徑的合成結果。
------------------------------------------------------------

3) Pinmux 世界觀（為什麼它是最重要的一層）
------------------------------------------------------------
Pinmux 的功能只有一個：
    「決定這個腳位最終被哪個模組擁有。」

錯誤示例：
    腳位被設定為 UART → SCU 設 direction/out 也無用。

實務經驗（鴻海 RD 常見 Bug）：
    60% 的 GPIO 問題都是 pinmux 設錯造成。

世界觀心法：
    Pinmux 不是選功能，是選命運。
------------------------------------------------------------

4) SCU 世界觀（GPIO 的內核）
------------------------------------------------------------
SCU 控制所有「電氣行為」：
- direction（input / output）
- data（高低）
- pull-up / pull-down
- open-drain
- drive strength
- debounce
- interrupt type（rising / falling / both）

SCU 就是：
    「GPIO 內部的 Linux Kernel」

sysfs 只是 API，不是控制核心。
------------------------------------------------------------

5) Interrupt 世界觀（事件通道）
------------------------------------------------------------
中斷不是：
    「某件事出錯」

而是：
    「電壓邊緣發生 → 通知 CPU 處理」

三大類型：
- Rising：0 → 1
- Falling：1 → 0
- Both：兩邊都觸發

Level：
- High level：pin 一直高 → 一直觸發
- Low level：pin 一直低 → 一直觸發

世界觀心法：
    Edge = 事件
    Level = 狀態監控
------------------------------------------------------------

6) Debounce 世界觀（訊號過濾器）
------------------------------------------------------------
Debounce = 多次 sample → 一致才算有效。

目的：
    消除按鍵、PGOOD、熱插拔、風扇 Tach 的抖動。

世界觀心法：
    Debounce 不是 delay，而是 digital filtering。
------------------------------------------------------------

7) Drive Strength／EMI／Overshoot（電氣層真正的難點）
------------------------------------------------------------
Drive Strength：
    推高/拉低的電流能力 → 決定 rise/fall time。

Rise Time：
    0 → 1 需要多久。

Fall Time：
    1 → 0 需要多久。

EMI：
    電流太大 → 線路變成天線。

Overshoot：
    電壓衝過頭（3.3V → 4.1V）。

世界觀心法：
    SCU 實際上在控制「訊號完整性」。
------------------------------------------------------------

8) 正確的 Debug Path（鴻海 RD 用的那一套）
------------------------------------------------------------
永遠按照以下順序檢查：

(1) Pinmux  
(2) SCU DIR  
(3) SCU DATA（是否變化）  
(4) Pull-up / Pull-down 是否符合需求  
(5) Open-drain 是否需要  
(6) Drive strength 是否太弱/太強  
(7) Interrupt type 是否正確  
(8) GIC 是否收到 IRQ  
(9) DTS 是否綁錯 irq/bank/bit  
(10) sysfs / gpiochip 是否對應正確  

世界觀心法：
    sysfs 只能用來驗證，不可用來 debug。
------------------------------------------------------------

9) 面試口條（你未來在鴻海要講的版本）
------------------------------------------------------------
「GPIO 控制並不是 sysfs 那層，而是一條跨越硬體與軟體的完整通路。

任何 GPIO 問題，我都會沿著 Physical → Pinmux → SCU → GIC → Driver 的路徑檢查。

Pinmux 決定腳位歸屬，SCU 決定電氣行為，GIC 負責中斷轉發，Driver 才是抽象層。

所以 debug 永遠不能從 sysfs 開始，而是從 pinmux 和 SCU 開始。」

（這段可以直接背給鴻海、Quanta、Wistron 面試官）
------------------------------------------------------------

============================================================

