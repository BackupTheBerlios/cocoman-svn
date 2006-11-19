# please note that the hash function is __builtins__.hash
from urlparse import urlparse
import BaseHTTPServer
import SimpleHTTPServer
import SocketServer
import cgi
import os
import select
import threading
import urllib


def debug(s):
    print s

class Handler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def split_arg(self, arg):
        parts = arg.split('=', 1)
        map(urllib.unquote, parts)
        if len(parts) != 2:
            return (parts[0], None)
        return parts

    def split_args(self, args):
        args = args.split('&')
        args = map(self.split_arg, args)
        return dict(args) 

    def do_GET(self):
        scheme, netloc, path, parameters, query, fragment = urlparse(self.path)
        argdict = self.split_args(query)
        host, port = self.client_address
        path = urllib.unquote(path)
        
        if path.strip('/') == "":
            # Serve index.html
            html_content = open("html/index.html").read()
            self.send_response(200, html_content)
        else:
            self.send_error(404, "Error 404: Page not found")

    # if someone sends us a post request through our upload form, let's save the file.
    def do_POST(self):
        ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
        fs = cgi.FieldStorage(fp=self.rfile, \
        headers=self.headers, environ = {'REQUEST_METHOD':'POST'}, \
        keep_blank_values = 1)
        fileField = fs['fileField']


def handle_request(server, request, client_address):
    if server.verify_request(request, client_address):
        server.process_request(request, client_address)
        server.close_request(request)

class Server:
    # TODO: set transfer to be cleaner than just a []
    def __init__(self):
        self.PORT=8000
        self.Handler = Handler 

    def serve(self):
        server = BaseHTTPServer.HTTPServer(('', self.PORT), self.Handler)
        debug("Starting server on %s" % (self.PORT))
        fd = server.fileno()
        poller = select.poll()
        poller.register(fd)
        while True:
            poller.poll()
            request, client_address = server.get_request()
            request_thread = threading.Thread(target=handle_request, args=(server, request, client_address))
            request_thread.start()
            # TODO clean up old threads

        server.server_close()
        del server

if __name__ == "__main__":
    server = Server()
    server.serve()
