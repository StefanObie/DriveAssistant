import ipinfo

import Include.keys as keys
access_token = keys.IPINFO_ACCESS_TOKEN
handler = ipinfo.getHandler(access_token)

# Function to get IP details
def get_ip_details():
    details = handler.getDetails()
    return details.all

# Test with an example IP address
ip_details = get_ip_details()

# Print out the details
print("IP Address Details:")
for key, value in ip_details.items():
    print(f"{key}: {value}")
