#!/usr/bin/env python3
"""
server.py — Tiny helper server to bridge HTML ↔ C++ binary
Usage: python3 server.py
Then open: http://localhost:8080/index.html

Endpoints:
  POST /write-input   — writes request body to input.txt
  GET  /run           — compiles (if needed) & runs ./agri
  GET  /output.json   — serves output.json produced by C++
  GET  /*             — serves static files (html, css, js)
"""
import http.server, subprocess, os, sys

PORT = 8080
CWD  = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        print(f"  {self.command} {self.path}", *args)

    def send(self, code, ctype, body):
        if isinstance(body, str): body = body.encode()
        self.send_response(code)
        self.send_header('Content-Type', ctype)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-Length', len(body))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_POST(self):
        if self.path == '/write-input':
            length = int(self.headers.get('Content-Length', 0))
            body   = self.rfile.read(length).decode()
            with open(os.path.join(CWD, 'input.txt'), 'w') as f:
                f.write(body)
            self.send(200, 'text/plain', 'OK')
        else:
            self.send(404, 'text/plain', 'Not found')

    def do_GET(self):
        if self.path == '/run' or self.path.startswith('/run?'):
            binary = os.path.join(CWD, 'agri')
            # Auto-compile if binary missing
            if not os.path.exists(binary):
                src = os.path.join(CWD, 'main.cpp')
                result = subprocess.run(
                    ['g++', '-o', binary, src, '-std=c++17'],
                    capture_output=True, text=True
                )
                if result.returncode != 0:
                    self.send(500, 'text/plain', 'Compile error:\n' + result.stderr)
                    return
            # Run the binary (reads input.txt, writes output.json)
            result = subprocess.run([binary], cwd=CWD, capture_output=True, text=True)
            if result.returncode != 0:
                self.send(500, 'text/plain', 'Runtime error:\n' + result.stderr)
                return
            self.send(200, 'text/plain', result.stdout)

        elif self.path.startswith('/output.json'):
            path = os.path.join(CWD, 'output.json')
            if os.path.exists(path):
                with open(path, 'rb') as f:
                    self.send(200, 'application/json', f.read())
            else:
                self.send(404, 'text/plain', 'output.json not found')

        else:
            # Serve static files
            fname = self.path.lstrip('/')
            if not fname or fname == '/':
                fname = 'index.html'
            # strip query string
            fname = fname.split('?')[0]
            fpath = os.path.join(CWD, fname)
            if os.path.exists(fpath) and os.path.isfile(fpath):
                ctype = {
                    '.html': 'text/html',
                    '.css':  'text/css',
                    '.js':   'application/javascript',
                    '.json': 'application/json',
                }.get(os.path.splitext(fname)[1], 'application/octet-stream')
                with open(fpath, 'rb') as f:
                    self.send(200, ctype, f.read())
            else:
                self.send(404, 'text/plain', f'File not found: {fname}')

if __name__ == '__main__':
    os.chdir(CWD)
    print(f"\n🌱 AgroSense server running at http://localhost:{PORT}")
    print(f"   Open http://localhost:{PORT}/index.html in your browser\n")
    with http.server.HTTPServer(('', PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")