import requests
import json
from geopy.distance import geodesic

def get_speed_limits_nearby(lat, lon):
    overpass_url = "http://overpass-api.de/api/interpreter"
    overpass_query = f"""
    [out:json];
    (
      way(around:500,{lat},{lon})[maxspeed];
      node(around:500,{lat},{lon});
    );
    out body;
    """
    
    response = requests.post(overpass_url, data=overpass_query)
    
    if response.status_code == 200:
        data = response.json()
        return data
    else:
        return None

def get_road_distance(road, nodes, lat, lon):
    # Calculate the distance between the input coordinates and the first node of the road
    if 'nodes' in road and len(road['nodes']) > 0:
        node_id = road['nodes'][0]
        node_details = next((node for node in nodes if node['id'] == node_id), None)
        if node_details:
            node_lat = node_details['lat']
            node_lon = node_details['lon']
            return geodesic((lat, lon), (node_lat, node_lon)).meters
    return float('inf')

def main():
    coords = input("Enter latitude and longitude separated by a comma: ")
    lat, lon = map(float, map(str.strip, coords.split(',')))

    data = get_speed_limits_nearby(lat, lon)
    
    if data and 'elements' in data:
        elements = data['elements']
        
        # Filter out nodes to have only ways with speed limits and nodes
        nodes = [elem for elem in elements if elem['type'] == 'node']
        roads = [elem for elem in elements if elem['type'] == 'way' and 'maxspeed' in elem['tags']]
        
        # Calculate distances for each road and sort them
        roads_with_distance = [(road, get_road_distance(road, nodes, lat, lon)) for road in roads]
        sorted_roads = sorted(roads_with_distance, key=lambda x: x[1])

        for road, distance in sorted_roads:
            road_name = road['tags'].get('name', 'Unnamed Road')
            speed_limit = road['tags']['maxspeed']
            print(f"Road: {road_name}, Speed Limit: {speed_limit}, Distance: {distance:.2f} meters")
    else:
        print("No roads with speed limits found nearby or error fetching data from Overpass API")

if __name__ == "__main__":
    main()
