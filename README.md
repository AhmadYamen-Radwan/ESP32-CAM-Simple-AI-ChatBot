# ESP32-CAM Simple AI ChatBot

A simple AI chatbot project for the ESP32-CAM, built with [PlatformIO](https://platformio.org/).

## Hardware note

This project was built and tested using a spare ESP32-CAM whose PSRAM and camera sensor are broken. As a result, camera functionality may not be available or reliable on this hardware.

## Large-context limitation

When processing a large context, the device may return `null` or an HTTP `-11` error. This is expected on hardware with limited or faulty memory resources, especially when PSRAM is unavailable or defective. Try using a smaller context if this occurs.
