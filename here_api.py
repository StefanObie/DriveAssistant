# Secret API keys 
import Include.keys as keys
API_KEY = keys.HERE_API_KEY

# Other imports
import requests
import json

def get_location_simulate():
    lat = -24.89823311476949
    lng = 28.216293154714478

    return lat, lng

def get_location():
    # Get the current location of the device from API
    # return lat, lng
    
    # For now, return a fixed location
    return get_location_simulate()

def here_api_simulate(lat, lng, rad=50):
    return json.loads('''
            {
            "items": [
                {
                "title": "R516, Bela-Bela, South Africa",
                "id": "here:af:streetsection:aWDMxpCzSPpyuvO7OGdT.B",
                "resultType": "street",
                "address": {
                    "label": "R516, Bela-Bela, South Africa",
                    "countryCode": "ZAF",
                    "countryName": "South Africa",
                    "state": "Limpopo",
                    "county": "Bela-Bela",
                    "city": "Bela-Bela",
                    "street": "R516"
                },
                "position": {
                    "lat": -24.89827,
                    "lng": 28.21629
                },
                "distance": 5,
                "mapView": {
                    "west": 27.86041,
                    "south": -24.97766,
                    "east": 28.69329,
                    "north": -24.82742
                },
                "navigationAttributes": {
                    "speedLimits": [
                    {
                        "maxSpeed": 60,
                        "direction": "W",
                        "speedUnit": "kph",
                        "source": "posted"
                    },
                    {
                        "maxSpeed": 100,
                        "direction": "E",
                        "speedUnit": "kph",
                        "source": "posted"
                    }
                    ]
                }
                }
            ]
            }
            '''                
        )                       

def here_api(lat, lng, rad=50):
    api_url = 'https://revgeocode.search.hereapi.com/v1/revgeocode'
    parameters = {
        'at': f'{lat},{lng},{rad}',
        'maxResults': '1',
        'apiKey': API_KEY,
        'showNavAttributes': 'speedLimits',
        'types': 'street'
    }

    try:
        response = requests.get(api_url, params=parameters) 
        response.raise_for_status()  # Raise an error for bad status codes
        return response.json()
    
    except requests.exceptions.RequestException as e:
        print(f"An error occurred: {e}")
        return None

def get_speedlimit(response):
    if 'items' in response and len(response['items']) > 0:
        # Return the lowest max speed as the speed limit
        maxspeeds = [m['maxSpeed'] for m in response['items'][0]['navigationAttributes']['speedLimits']]
        return min(maxspeeds)
    
def main():
    lat, lng = get_location() # Simulated at the moment

    response = here_api(lat, lng)
    maxspeed = get_speedlimit(response)
    print(maxspeed)

def main_simulate():
    lat, lng = get_location_simulate()

    response = here_api_simulate(lat, lng)
    maxspeed = get_speedlimit(response)
    print(maxspeed)

if __name__ == "__main__":
    # main()
    main_simulate()