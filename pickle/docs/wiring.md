# Pickle — Wiring Notes

## Power architecture

```
3× 18650 Li-Ion (11.1V)
        │
        ├──→ L298N 12V pin (motors)
        │
        └──→ Buck converter IN (LM2596)
                    │
                    └──→ 5V OUT → ESP32 VIN
                    └──→ 5V OUT → HC-SR04 VCC
```

Keep ESP32 on its own regulated 5V line from the buck converter.
Never power ESP32 from L298N's onboard 5V — motor surges will reset it.

## Common ground

All of these must share a single GND connection:
- Battery negative
- L298N GND
- Buck converter GND
- ESP32 GND
- HC-SR04 GND
- OLED GND

## Motor direction fix

If a motor spins the wrong way, swap its two wires at the L298N output terminals (OUT1/OUT2 or OUT3/OUT4). No code change needed.

## HC-SR04 mounting

Mount inside the chassis with two small holes (~16mm apart, ~8mm diameter each) drilled in the front face for the transducers. This keeps the OLED as the only visible "face" of the robot.

## L298N heatsink

Always attach the included heatsink to the L298N chip. It drops ~2V as heat even at idle and gets warm during operation.
