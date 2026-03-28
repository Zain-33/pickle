# 🥒 Pickle — ESP32 WiFi Robot

Pickle is a small autonomous WiFi-controlled robot built on the ESP32 DevKit V1. It features animated OLED eyes, three behaviour modes, obstacle avoidance, and a web-based control panel accessible from any phone or browser.

---

## ✨ Features

- **WiFi control** — connect to Pickle's hotspot and drive from any browser, no app needed
- **Animated OLED eyes** — RoboEyes library brings Pickle's face to life with blinking and idle animations
- **Sleep mode** — OLED shows closed eyes with floating zzZ
- **Wiggle mode** — Pickle does a rhythmic dance: left-right shake, forward-back bounce, half-spin finish
- **Autonomous mode** — Pickle explores freely with real-time obstacle avoidance using HC-SR04
- **Obstacle avoidance** — stops, backs up, and randomly turns when something is detected within 20cm

---

## 🛒 Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 DevKit V1 |
| Motor driver | L298N |
| Display | SSD1306 OLED 128×64 (I2C) |
| Obstacle sensor | HC-SR04 Ultrasonic |
| Motors | 2× DC hobby motors (TT or N20) |
| Battery | 3× 18650 Li-Ion in series (11.1V) |
| Power regulation | LM2596 Buck converter (11.1V → 5V for ESP32) |

---

## 📌 Wiring

### ESP32 → OLED (SSD1306)
| OLED | ESP32 |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 3.3V |
| GND | GND |

### ESP32 → L298N
| L298N | ESP32 |
|---|---|
| IN1 | GPIO 25 |
| IN2 | GPIO 26 |
| ENA | GPIO 32 |
| IN3 | GPIO 27 |
| IN4 | GPIO 14 |
| ENB | GPIO 33 |

### ESP32 → HC-SR04
| HC-SR04 | ESP32 |
|---|---|
| VCC | 5V (from buck converter) |
| TRIG | GPIO 18 |
| ECHO | GPIO 19 |
| GND | GND |

### Power
| From | To |
|---|---|
| 3× 18650 (11.1V) + | L298N 12V pin |
| 3× 18650 (11.1V) + | Buck converter IN+ |
| Buck converter OUT (5V) | ESP32 VIN |
| All GNDs | Common ground |

> ⚠️ **Important:** Power the ESP32 from a buck converter, NOT from the L298N 5V pin. Motor current surges will cause the ESP32 to reset if they share power.

> ⚠️ **Important:** All GNDs must be connected together — battery, L298N, buck converter, ESP32, HC-SR04, and OLED.

---

## 🔧 Software Setup

### 1. Add ESP32 board support
In Arduino IDE go to **File → Preferences** and add this URL to "Additional Board Manager URLs":
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then go to **Tools → Board → Board Manager**, search `esp32` and install **esp32 by Espressif Systems**.

### 2. Install libraries
Go to **Sketch → Include Library → Manage Libraries** and install:

| Library | Author |
|---|---|
| Adafruit SSD1306 | Adafruit |
| Adafruit GFX Library | Adafruit |
| FluxGarage RoboEyes | FluxGarage |

### 3. Board settings
| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Upload Speed | 921600 |
| Flash Size | 4MB |
| Partition Scheme | Default |

### 4. Upload
Open `pickle/pickle.ino` in Arduino IDE, select your COM port, and click Upload.

---

## 📱 How to use

1. Power on Pickle
2. On your phone or laptop, connect to the WiFi network: **`pickle`**
3. Open a browser and go to **`192.168.4.1`**
4. Use the control panel to drive Pickle or switch modes

### Modes
| Mode | Behaviour |
|---|---|
| 😴 Sleep | Pickle stops, OLED shows closed eyes and zzZ |
| 🕺 Wiggle | Pickle performs a rhythmic dance pattern periodically |
| 🤖 Autonomous | Pickle explores freely and avoids obstacles automatically |

---

## 📁 Project Structure

```
pickle/
├── pickle/
│   └── pickle.ino        # Main Arduino sketch
├── docs/
│   └── wiring.md         # Detailed wiring notes
├── .gitignore
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

---

## 🔩 Tuning

You can adjust these values at the top of `pickle.ino`:

```cpp
#define MOTOR_SPEED          200   // 0-255, motor speed
#define OBSTACLE_DISTANCE_CM  20   // obstacle detection range in cm
```

For wiggle dance timing, edit the `wiggleDance()` function delay values.
For autonomous movement frequency, change `random(100)` — smaller = more frequent.

---

## ⚠️ Known issues

- L298N drops ~2V as heat — normal, use a heatsink
- HC-SR04 may give false readings very close to walls (<3cm) — this is a sensor limitation
- If motors spin wrong direction on first build, swap the two wires on one motor at the L298N output terminals

---

## 📄 License

MIT — see [LICENSE](LICENSE)

---

## 🙏 Credits

- [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) — animated robot eyes library
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) — OLED display driver
- Built with ❤️ and Arduino
