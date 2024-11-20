

msg = ""
msg2 = "Hello from python CGI"

i = 0
while i < 1:
    msg = msg + msg2 + "\n"
    i += 1

print("HTTP/1.0 200 OfffK\r")
print("Content-Type: text/html\r")
print("Content-Length: " + str(len(msg)) + "\r" )
print("\r")
print(msg, end="")