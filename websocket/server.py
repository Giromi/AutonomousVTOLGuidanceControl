import asyncio
import websockets
import argparse

async def echo(websocket, path):
    print("클라이언트 연결됨")
    try:
        async for message in websocket:
            print(f"클라이언트로부터 받은 메시지: {message}")
            await websocket.send(f"Echo: {message}")
    except websockets.exceptions.ConnectionClosed:
        print("클라이언트 연결 종료")

async def start_server(host, port):
    async with websockets.serve(echo, host, port):
        print(f"서버가 {host}:{port}에서 시작됨")
         # 서버가 영구적으로 실행
        await asyncio.Future() 

def main():
    parser = argparse.ArgumentParser(description="Run a websocket server.")
    parser.add_argument('--host', type=str, default='0.0.0.0',
                        help='Host to run the websocket server on')
    parser.add_argument('--port', type=int, default=9998,
                        help='Port to run the websocket server on')
    
    args = parser.parse_args()

    print('host:', args.host)
    print('port:', args.port)

    asyncio.run(start_server(args.host, args.port))


if __name__ == "__main__":
    main()

'''
python3 server.py --port 8765
'''

