

msg = ""
msg2 = "Hello from POST python CGI that doesn't take input"

i = 0
while i < 1:
    msg = msg + "\n" + msg2
    i += 1

print("HTTP/1.0 200 OfffK\r")
print("Content-Type: text/html\r")
print("Content-Length: " + str(len(msg)) + "\r" )
print("\r")
print(msg, end='')