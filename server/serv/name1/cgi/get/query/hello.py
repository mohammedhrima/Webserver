import os
import cgi
import cgitb
import html

cgitb.enable()
form = cgi.FieldStorage()

name = form.getvalue('name')
msg=""

# Ensure name is a string
if name is None:
    print("HTTP/1.1 400 NOK")
    msg = "<h1>name query is required<h1>"
else:
    print("HTTP/1.1 200 NOK")
    name = html.escape(name)
    msg = f"<h1>hello {name}</h1>"

print("Content-Type: text/html")
print("Content-Length: " + str(len(msg)))
print()
print(msg)
