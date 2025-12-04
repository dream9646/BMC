                ┌───────────────────────────────┐
                │        User Program           │
                │  (Shell echo / C tool)        │
                └───────────────┬───────────────┘
                                │
                                ▼
                ┌───────────────────────────────┐
                │    Sysfs (/sys/class/gpio)    │
                └───────────────┬───────────────┘
                                │
                                ▼
        ┌───────────────────────┴──────────────────────┐
        │           GPIO Driver (kernel space)         │
        │  - probe()                                   │
        │  - use reg from DTS                          │
        │  - register gpiochip                         │
        └───────────────┬──────────────────────────────┘
                        │
                        ▼
                ┌───────────────────────────────┐
                │   GPIO Registers (SoC/CPLD)   │
                │  - direction / value / mode   │
                └───────────────┬───────────────┘
                                │
                                ▼
                ┌───────────────────────────────┐
                │ Physical GPIO Pin             │
                │ (LED / FAN / PSU / Case-Open) │
                └───────────────────────────────┘


        ┌───────────────────────────────┐
        │      Device Tree (DTS)        │
        │  - pinmux                     │
        │  - reg                        │
        │  - interrupts                 │
        └───────────────┬───────────────┘
                        │ compatible + reg
                        ▼
                  [ GPIO Driver ]
