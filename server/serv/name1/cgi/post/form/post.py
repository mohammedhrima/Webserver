#!/usr/bin/python3

import cgi

form = cgi.FieldStorage()

msg = "<html><body>"
msg += "<h1>Python response</h1>\n"
if "name" and "age" in form:
    header = "HTTP/1.1 200 OK"
    name = form.getvalue("name")
    age = form.getvalue("age")
    msg += "<p>user has name: " + name + "</p>"
    msg += "<p>and age: " + age + "</p>"
else:
    header = "HTTP/1.1 400 NOK"
    msg += "<h2>name, age, are required!</h2>"
msg += "</body></html>"


print("Content-type:text/html")
print("Content-Length:", len(msg), "")
print("")
print(msg)