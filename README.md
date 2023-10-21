# wifi-smart-controller

WiFi Smart Controller with NodeMCU(ESP-12E) &amp; Home Assistant

## What is this?

**WiFi Smart Controller** is a project that help you integrate the non-intelligent devices to your Home Asistant!

We can control the devices easily via Home Assistant by our smart phones, like iPhone. With the Home Assistant addons, you can also use Siri to control them!

## How we can do this?

Here are three parts of this project:

- firmware: NodeMCU(ESP-12E) firmware developed with Arduino
- pcb: PCB designed with LCEDA Pro
- home_assistant: update `configure.yaml`

```text
HomeKit --> Home Assistant --> MQTT Broker --> NodeMCU --> Serial Port --> Serial Sender(Infrared, 315MHz/433MHz RF, Bluetooth)
```

## How to use?

- clone the project
- modify & compile the firmware by Arduino
- modify & update Home Assistant `configaure.yaml` file

## Advanced

YOU CAN FORK THE PROJECT AND MAKE AMAZING JOB BY YOURSELF!

## Thanks

Thanks for provding free test circuit board making service by [J@LC](https://www.jlc.com)

## LICENSE

Herbert He &copy; MIT
