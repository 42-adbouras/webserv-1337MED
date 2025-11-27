#!/usr/bin/env python3
import time
print("Content-Type: text/plain")
print()
time.sleep(2)
print("done")

#This will stress CGI handling and waitpid(WNOHANG) logic
#siege -c50 -t30s http://localhost:8080/cgi-scripts/slow.py
