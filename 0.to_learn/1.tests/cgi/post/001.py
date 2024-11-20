
print "Content-Type: text/html\n\n"

import cgi

form = cgi.FieldStorage()
username = form.getvalue("username")

print username