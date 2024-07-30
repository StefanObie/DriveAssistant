# Drive Assistant

## Overview
![Discovery Movement Report](movement_report.png)


## Notes

How to get accurate [Accurate Time](https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/) to an ESP32 through wifi.

The compiled sketch is too big if it is compiled normally. Use the following partition scheme instead:

```
Tools -> partition scheme -> "Hugh app (3MB no OTA/ 1MB SPIFFS)"
```

### 26 July 2024
There is 2 speed limit APIs:
1. **Google Roads API**: Paid API, USD 0.02 per request. It is the easier API. Free trail $300 and 87 days remaining.
2. **Overpass API**: More compilacted, but free. It uses nodes to identify a road. With the API call I get the nodes, but not the road, and no speed limit. More testing is required. [Docs](https://wiki.openstreetmap.org/wiki/Overpass_API).
Other useful links:
    - [ExampleRoad](https://www.openstreetmap.org/way/4279932#map=16/-24.9017/27.7493) - Example of a road in OpenStreetMap
    - [OverpassTurbo](https://overpass-turbo.eu/) - A tool to test Overpass API queries
    - Get coordinates from google maps with right click.