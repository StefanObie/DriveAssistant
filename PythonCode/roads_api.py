import googlemaps
from googlemaps.exceptions import ApiError, TransportError

# Replace with your API key
api_key = "AIzaSyA7p7Iv0Ty7zYcPEkIfUr6FqVjd3nckpC0"

gmaps = googlemaps.Client(key=api_key)

# Define a location (latitude and longitude)
location = [(-25.851650022795447, 28.147669913968162)]  # Example coordinates for San Francisco

try:
    # Request speed limits for the given location
    result = gmaps.speed_limits(location)
    print(result)
except ApiError as e:
    print(f"API error: {e}")
except TransportError as e:
    print(f"Transport error: {e}")
except Exception as e:
    print(f"General error: {e}")
