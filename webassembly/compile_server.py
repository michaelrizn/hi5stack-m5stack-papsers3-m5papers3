#!/usr/bin/env python3
import http.server
import socketserver
import json
import subprocess
import os
import threading
from urllib.parse import urlparse, parse_qs

class CompileHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=os.getcwd(), **kwargs)
    
    def do_POST(self):
        if self.path == '/compile-game':
            self.handle_compile_request()
        else:
            self.send_error(404)
    
    def handle_compile_request(self):
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            game_name = data.get('game')
            if not game_name:
                self.send_error(400, 'Game name required')
                return
            
            if not self.is_valid_game_name(game_name):
                self.send_error(400, 'Invalid game name')
                return
            
            success = self.compile_game(game_name)
            
            if success:
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                response = {'status': 'success', 'message': f'Game {game_name} compiled successfully'}
                self.wfile.write(json.dumps(response).encode())
            else:
                self.send_error(500, f'Failed to compile game {game_name}')
                
        except Exception as e:
            print(f'Error handling compile request: {e}')
            self.send_error(500, str(e))
    
    def is_valid_game_name(self, name):
        valid_games = ['test', 'sudoku', 'minesweeper']
        return name in valid_games and name.replace('_', '').isalnum()
    
    def compile_game(self, game_name):
        try:
            print(f'Compiling game: {game_name}')
            
            result = subprocess.run(
                ['make', game_name],
                cwd=os.getcwd(),
                capture_output=True,
                text=True,
                timeout=60
            )
            
            if result.returncode == 0:
                print(f'Successfully compiled {game_name}')
                return True
            else:
                print(f'Failed to compile {game_name}: {result.stderr}')
                return False
                
        except subprocess.TimeoutExpired:
            print(f'Compilation timeout for {game_name}')
            return False
        except Exception as e:
            print(f'Error compiling {game_name}: {e}')
            return False
    
    def do_GET(self):
        if self.path == '/compile-status':
            self.handle_status_request()
        elif self.path.startswith('/check-source'):
            # Парсим параметр game из URL
            parsed_url = urlparse(self.path)
            params = parse_qs(parsed_url.query)
            game_name = params.get('game', [None])[0]
            
            if not game_name:
                self.send_response(400)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps({'error': 'Game name required'}).encode())
                return
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            # Проверяем существование исходного файла
            source_path = os.path.join('..', 'src', 'games', game_name, 'game.cpp')
            exists = os.path.exists(source_path)
            
            self.wfile.write(json.dumps({'exists': exists, 'path': source_path}).encode())
        else:
            super().do_GET()
    
    def handle_status_request(self):
        try:
            games_status = {}
            valid_games = ['test', 'sudoku', 'minesweeper']
            
            for game in valid_games:
                source_path = f'../src/games/{game}/game.cpp'
                compiled_path = f'games/{game}/game.wasm'
                
                source_exists = os.path.exists(source_path)
                compiled_exists = os.path.exists(compiled_path)
                
                needs_compilation = False
                if source_exists and compiled_exists:
                    source_mtime = os.path.getmtime(source_path)
                    compiled_mtime = os.path.getmtime(compiled_path)
                    needs_compilation = source_mtime > compiled_mtime
                elif source_exists and not compiled_exists:
                    needs_compilation = True
                
                games_status[game] = {
                    'source_exists': source_exists,
                    'compiled_exists': compiled_exists,
                    'needs_compilation': needs_compilation
                }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(games_status).encode())
            
        except Exception as e:
            print(f'Error handling status request: {e}')
            self.send_error(500, str(e))
    
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

def start_server(port=8001):
    try:
        with socketserver.TCPServer(("", port), CompileHandler) as httpd:
            print(f"Compile server running on port {port}")
            print(f"Access at: http://localhost:{port}")
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped")
    except Exception as e:
        print(f"Server error: {e}")

if __name__ == "__main__":
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8001
    start_server(port)