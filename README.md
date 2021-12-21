# IndicatorPixel
ESP8266 project to use a WS2812B pixel strip/ring as an indicator with values set by HTTP GET call over WiFi

Placeholder README.md

Most of the documentation is in the code.

Call it using http://<ip address of ESP8266>/?value=<value>&min=<min>&max=<max>&brightness=<brightness>

NOTE: parameters MUST be passed in order and are all mandatory currently.

value = value to set indicator to (can be any int value)
min = minimum value to use for mapping values (can be any int value)
max = maximum value to use for mapping values (can be any int value)
brightness = brightness value to use when displaying pixels (0-255 allowed, no validation yet)

value can be less than or more than min/max and appears to work as expected (eg no pixels light or all of them light)

Example parameters: http://<ip address of ESP8266>/?value=150&min=100&max=200&brightness=64
This will light half the pixels because 150 maps exactly half way between 100 and 200. Pixels will be at 25% brightness (64/255)

Internally, all values are mapped using the min and max to a percentage of the pixels to light.

Three colours are used: green, orange and red.

Thresholds for the colours is currently hard coded. Green:0-50%, Orange:50-75%, Red:75-100%
