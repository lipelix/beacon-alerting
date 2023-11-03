# Beacon-alerting

This project handles alerts coming from opsgenie turn on beacon placed in Pilsen offices.

> GOAL: When alert emerge, beacon start blink. It keeps blinking until alert is acknowledged.

![System architecture](assets/architecture.svg)

## How it will work?

### Micro-controllers

2 approaches how to do this:

### 1) Wi-fi based approach
<img src="https://img.shields.io/badge/NOT_THE_WAY-inactive.svg">

This will need wi-fi module (is part NodeMCU). Firmware will need be connected with opsgenie through API. This should be handled by some ospgenie integration.

Then firmware will turn-on pins which will init beacon.

Pros:
- one micro-controller

Cons:
- need to setup wifi
- wifi has to work all the time - manage reconnection
- handling opsgenie integration

### 2) GSM based approach
<img src="https://img.shields.io/badge/IN_PROGRESS-green.svg">

This will need GSM module and main micro-controller(arduinoNano). Firmware will need to handle incomming calls or SMS from opsgenie.

Then firmware will turn-on pins which will init beacon.

Pros:
- simpler communication channel with opsgenie

Cons:
- one micro-controller + second GSM module
- need to setup GSM
- GSM has to work all the time - manage reconnection


# Progress

`03-11-2023`
![Party started](assets/getting_ready_03_11_2023.jpg)
