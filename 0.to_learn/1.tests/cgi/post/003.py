#!/usr/bin/env python3

import cgi

# Print the HTTP header
print("Content-Type: text/html")
print()

# Get form data from the POST request
form = cgi.FieldStorage()

# Check if the form was submitted
if "name" in form and "age" in form:
    name = form.getvalue("name")
    age = form.getvalue("age")
    
    # Generate the response HTML
    response_html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Response</title>
    </head>
    <body>
        <h2>POST Data Received</h2>
        <p>Name: {name}</p>
        <p>Age: {age}</p>
    </body>
    </html>
    """
    
    # Print the response HTML
    print(response_html)
else:
    # If no data was submitted, print an error message
    print("<h2>Error: No data received</h2>")
