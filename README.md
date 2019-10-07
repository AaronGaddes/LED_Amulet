# LED Amulet
An IoT amulet with a WiFi Access point that serves up a simple web page to control the patterns.

## Setup
This project uses the Platform.io VS Code Extension to build and upload the code to an ESP32 or ESP8266 development board. Make sure the appropriate board definition/frameworks are installed using the Platform.io extension

## The crystal
![](/readme_media/20190925_090149.jpg)
![](/readme_media/20190925_090151.jpg)
![](/readme_media/20190925_090156.jpg)
![](/readme_media/20190925_090201.jpg)

## Initial Tests
![](/readme_media/VideoCapture_20190919-203011.jpg)
![](/readme_media/VideoCapture_20190919-203037.jpg)


## Finished(?) Product
![](/readme_media/20191007_151411.jpg)
![](/readme_media/20191007_151111.jpg)

## TODOs
- [ ] Replace `processor()` function with a REST endpoint like the other state changes
    - [ ] and update ../data/index.html file to send a GET response rather than navigating to a new URL
- [ ] Update `pulse()` function to work
- [ ] Save state to EPROM/SPIFFS
- [ ] Add videos to README