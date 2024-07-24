import asyncio
import websockets
import argparse
import sys

async def send_messages(websocket):
    while True:
        # 비동기적으로 사용자 입력 받기
        message = await asyncio.get_event_loop().run_in_executor(None, sys.stdin.readline)
        message = message.strip()  # 줄바꿈 문자 제거
        if message:
            await websocket.send(message)

async def receive_messages(websocket):
    try:
        while True:
            response = await websocket.recv()
            print(f"Received: {response}")
    except websockets.exceptions.ConnectionClosed:
        print("Connection to server closed.")

async def chat_client(ip, port):
    uri = f"ws://{ip}:{port}"
    try:
        async with websockets.connect(uri) as websocket:
            send_task = asyncio.create_task(send_messages(websocket))
            receive_task = asyncio.create_task(receive_messages(websocket))
            await asyncio.gather(send_task, receive_task)
    except Exception as e:
        print(f"Failed to connect or error during the session: {e}")

def main():
    parser = argparse.ArgumentParser(description="Connect to a websocket server.")
    parser.add_argument('--ip', type=str, default='localhost', help='Host of the websocket server')
    parser.add_argument('--port', type=int, default=9998, help='Port of the websocket server')
    
    args = parser.parse_args()
    asyncio.run(chat_client(args.ip, args.port))

if __name__ == "__main__":
    main()
