

msg = "<html><body><h1>Hello from CGI</h1></body></html>"

print("HTTP/1.0 200 OK")
print("Content-Type: text/html")
print("Content-Length: " + str(len(msg)) )
print("")
print(msg)

