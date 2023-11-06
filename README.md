# Beacon-alerting

This project handles alerts coming from opsgenie turn on beacon placed in Pilsen offices.

> GOAL: When alert emerge, beacon start blink. It keeps blinking until alert is acknowledged.

![System architecture](assets/architecture.svg)

# How to make it work?

It will consist of bunch of chapters - at first we need to create PoC and, grab all components (software and hardware) and check if it is feasible. After that we can move forward and start to figuring out how to solve power, soldering (or create custom board..) and make advanced prototype - the Alpha. At the end of Alpha it will work, but will not be beauty - that will be last part.

## 🛫 Chapter I - PoC  ![IN_PROGRESS]

Outcome of this chapter will be functional model with components connected on breadboard - beacon will be simulated with Light-Emitting Diode - which will react on external signal (sended through wifi or GSM) - diode will start blink.

- [ ] Decide wifi vs GSM approach
  - [ ] 📶 PoC of GSM approach ![IN_PROGRESS]
    - [ ] Create simulation (https://wokwi.com/)
    - [ ] Gather all components
    - [ ] Assemble components on breadboard
    - [ ] Kick-off firmware and upload it to board  
  - [ ] 🛜 PoC of Wifi approach ![TODO]
    - [ ] Create simulation (https://wokwi.com/)
    - [ ] Gather all components
    - [ ] Assemble components on breadboard
    - [ ] Kick-off firmware and upload it to board
- [ ] Present functional prototype
- [ ] Get a beer

## 🔮 Chapter II - Alpha

Outcome of this chapter will be functional model with components connected through soldering or custom board - beacon will be real beacon - which will react on external signal (sended through wifi or GSM) from opsgenie when alert emerge.

>TBA Chapter I. could reveal something hidden - will be added

- [ ] Find hardware board creator (china?)
- [ ] Create simulation model including power
- [ ] Create integration in opsgenie
- [ ] Present functional Alpha
- [ ] Get a beer

## 🚨 Chapter III - The Beacon v1.0.0

>TBA Chapter II. could reveal something hidden - will be added

- [ ] Put all things together
  - [ ] Create 3D model which will hold all components together
  - [ ] Print/Solder/Emerge base
  - [ ] Put components together in base
- [ ] Present functional Beacon
- [ ] Get a lot of beer

---
---

# Work notes

## Micro-controllers

2 approaches how to do this:

### 1) Wifi based approach 🛜
![WONT_DO]

This will need wi-fi module (is part NodeMCU). Firmware will need be connected with opsgenie through API. This should be handled by some ospgenie integration.

Then firmware will turn-on pins which will init beacon.

👍 Pros:
- one micro-controller

👎 Cons:
- need to setup wifi
- wifi has to work all the time - manage reconnection
- handling opsgenie integration

### 2) GSM based approach 📶
![IN_PROGRESS]

This will need GSM module and main micro-controller(arduinoNano). Firmware will need to handle incomming calls or SMS from opsgenie.

Then firmware will turn-on pins which will init beacon.

👍 Pros:
- simpler communication channel with opsgenie
- GSM is everywhere wifi is not

👎 Cons:
- one micro-controller + second GSM module
- need to setup GSM (tel number, sim card)
- GSM has to work all the time - manage reconnection

# Development tools

- Firmware development, build, deploy -> https://www.arduino.cc/en/software
- Hardware modeling https://wokwi.com/arduino

# Troubleshooting

🐛 - for chinesse Arduino Nano use Board Type `Arduino Duemilanove or Diecimila`

# Progress

`03-11-2023`
![Party started](assets/getting_ready_03_11_2023.jpg)


# Used components
- Arduino Nano: https://dratek.cz/arduino/823-arduino-nano-v3.0-atmega328-precizni-klon.html
> TBA


<!-- Metadata -->

<!-- Image links -->
[IN_PROGRESS]: https://img.shields.io/badge/IN_PROGRESS-green.svg
[WONT_DO]: https://img.shields.io/badge/WONT_DO-inactive.svg
[TODO]: https://img.shields.io/badge/TODO-blue.svg