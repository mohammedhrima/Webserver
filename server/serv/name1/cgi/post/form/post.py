#!/usr/bin/python3

import cgi

print("Content-type: text/html\n")
print("<html><head><title>Your Resume</title><link rel='stylesheet' href='/style.css'></head><body>")
print("<div class='container'>")

form = cgi.FieldStorage()

name = form.getvalue("name")
email = form.getvalue("email")
phone = form.getvalue("phone")
skills = form.getvalue("skills")
experience = form.getvalue("experience")
education = form.getvalue("education")

if name and email and phone and skills and experience and education:
    print(f"<h1>Resume for {name}</h1>")
    print(f"<p><strong>Email:</strong> {email}</p>")
    print(f"<p><strong>Phone:</strong> {phone}</p>")
    print("<h3>Skills:</h3>")
    print(f"<p>{skills}</p>")
    print("<h3>Work Experience:</h3>")
    print(f"<p>{experience}</p>")
    print("<h3>Education:</h3>")
    print(f"<p>{education}</p>")
else:
    print("<h2>Error: All fields are required!</h2>")

print("</div></body></html>")
