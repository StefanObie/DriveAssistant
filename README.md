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

### 8 Aug 2024
The DQ Mapper for C and S works most of the time. I can use the DQ Mapper of the planholder to get all the distance information of all the users. Still need to sign into the accounts for the drive dashboard information. 

**TO-DO**
<<<<<<< HEAD
1. Night time drive should rather be calculated, if possible.
=======
1. Access DQ Mapper through plan holder.
2. Insert trigger/wait for user to login then continue execution. The user must manually login into each of the 3 different accounts. 

## 16 September 2024

[HERE Technologies](https://developer.here.com/) offers a comprehensive platform for building location-based applications. The HERE Platform provides APIs and SDKs for geolocation services such as mapping, routing, geocoding, traffic data, and road attributes. With global coverage and high accuracy, developers can integrate rich location intelligence into their applications, including:
- Reverse Geocoding API: Converts coordinates into readable addresses.

This API returns the speed limit of the road. The speed limit is in the `speedLimits` field. The API is free and has a limit of 30,000 requests per month. 

$$\frac{30 000 \text{ calls}}{1 \text{ month}} \times \frac{2 \text{ min}}{1 \text{ call}} \times \frac{1 \text{ hour}}{60 \text{ min}} = \frac{1000 \text{ hours}}{1 \text{ month}}$$

Usage limits will most likely not be exceeded. 1000 hours of usage per month is equivalent to 1 request every 2 minutes.

The full docs can be found [here](https://developer.here.com/documentation/geocoding-search-api/dev_guide/topics/endpoint-reverse-geocode-brief.html).

New API:

```
https://revgeocode.search.hereapi.com/v1/revgeocode?at=-24.89823311476949,28.216293154714478,50&maxResults=1&apiKey=O3mrl3T9c5sWnhRMSj5g_AunesqRba3Cku0z8I6Rd0M&showNavAttributes=speedLimits&types=street
```

TODO: 
1. Protect API KEY.
2. Ensure usage does not exceed free tier. See [usage](https://platform.here.com/management/usage).
3. Try to remove payment information.

Cool websites:
1. Get speed limit from a click: [RME Speed Limit Demo](https://demo.support.here.com/examples/v3/rme_speed_limits). Open the console (F12) to see what the API call looks like. It is a older version, but still useful.
2. Get speed limit from coordinates: [Speed Limits using Geocoder + PDE](https://demo.support.here.com/examples/v3/link_speed_locator).


>>>>>>> 5a1b85d5f6439773db0f58c0a8d8a909db23fedb
