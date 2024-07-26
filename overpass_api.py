import requests
import json

def get_speed_limit(lat, lon):
    overpass_url = "http://overpass-api.de/api/interpreter"
    overpass_query = f"""
    [out:json];
    way(around:50,{lat},{lon})[maxspeed];
    out body;
    """
    
    response = requests.post(overpass_url, data=overpass_query)
    
    if response.status_code == 200:
        data = response.json()
        return data
    else:
        return None

def main():
    coords = input("Enter latitude and longitude separated by a comma: ")
    lat, lon = map(str.strip, coords.split(','))

    data = get_speed_limit(lat, lon)
    
    if data:
        print(json.dumps(data, indent=2))
    else:
        print("Error fetching data from Overpass API")

if __name__ == "__main__":
    main()
