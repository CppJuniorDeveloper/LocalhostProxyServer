import socket
import argparse

parser = argparse.ArgumentParser(description='TCP log client')
parser.add_argument('host', help='Server host (e.g., 127.0.0.1)')
parser.add_argument('port', type=int, help='Server port (e.g., 12345)')
parser.add_argument('--name', '-n', default='python-client',
                    help='Client name (default: python-client)')

args = parser.parse_args()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((args.host, args.port))
        s.sendall(f'{args.name}\n'.encode())

        print(f'Connected to {args.host}:{args.port} as {args.name}.')
        print('Type messages (Ctrl+C to exit):')

        while True:
            msg = input('> ')
            s.sendall((msg + '\n').encode())

            file = s.makefile('rb')
            response = file.readline();

            print("[Server] " + response.decode('utf-8').strip())

    except KeyboardInterrupt:
        print('\nDisconnected.')

    except ConnectionResetError:
        print('\n[!] Connection reset by server. The server may have crashed or closed the connection abruptly.')

    except ConnectionRefusedError:
        print('\n[!] Failed to connect: server is not running or port is closed.')

    except ConnectionAbortedError:
        print('\n[!] Connection aborted: the server closed the connection unexpectedly.')
