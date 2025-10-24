import os, sys

def show(key):
    print(f"{key}: {os.environ.get(key,'')}")

length = int(os.environ.get("CONTENT_LENGTH","0") or 0)
body   = sys.stdin.read(length) if length > 0 else ""
show("GATEWAY_INTERFACE")
show("SERVER_PROTOCOL")
show("SERVER_SOFTWARE")
show("REQUEST_METHOD")
show("SCRIPT_FILENAME")
show("SCRIPT_NAME")
show("QUERY_STRING")
show("SERVER_NAME")
show("SERVER_PORT")
show("REMOTE_ADDR")
show("CONTENT_TYPE")
show("CONTENT_LENGTH")

print("\nBody:")
print(body)

# import os

# for key, value in os.environ.items():
#     print(f"{key}: {value}")