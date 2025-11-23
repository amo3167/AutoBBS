import sys, threading
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer


class WebServer(threading.Thread):
    def __init__(self, port=25000):
        try:
            threading.Thread.__init__(self)
            self.port = port
        except Exception:
            e = Exception
            print sys.exc_info()[1]
            self.error = e
            return

    def run(self):
        server = HTTPServer(('', self.port), myHandler)
        print 'Started httpserver on port ', self.port
        server.serve_forever()


class myHandler(BaseHTTPRequestHandler):
    #Handler for the GET requests
    def do_GET(self):
        if self.path == "/":
            self.path = "/index.html"

        try:
            #Check the file extension required and
            #set the right mime type

            sendReply = False
            if self.path.endswith(".html"):
                mimetype = 'text/html'
                sendReply = True
            if self.path.endswith(".png"):
                mimetype = 'image/png'
                sendReply = True
            if self.path.endswith(".gif"):
                mimetype = 'image/gif'
                sendReply = True
            if self.path.endswith(".js"):
                mimetype = 'application/javascript'
                sendReply = True
            if self.path.endswith(".css"):
                mimetype = 'text/css'
                sendReply = True

            if sendReply == True:
                #Open the static file requested and send it
                f = open('plots/' + self.path, 'rb')
                self.send_response(200)
                self.send_header('Content-type', mimetype)
                self.end_headers()
                self.wfile.write(f.read())
                f.close()
            return

        except IOError:
            self.send_error(404, 'File Not Found: %s' % self.path)


    def log_message(self, format, *args):
        pass
