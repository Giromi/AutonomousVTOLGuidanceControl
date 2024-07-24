import asyncio
import websockets
import argparse

connected = set()

async def chat_handler(websocket, path):
    connected.add(websocket)
    try:
        # 정기적으로 핑 보내야 함
        # asyncio.create_task(ping_interval(websocket))
        while True:
            message = await websocket.recv()
            broadcast_message = f"{websocket.remote_address}: {message}"
            print(broadcast_message)
            await broadcast(broadcast_message, websocket.remote_address)
    except websockets.exceptions.ConnectionClosed:
        print(f"Connection closed: {websocket.remote_address}")
    finally:
        connected.remove(websocket)

# async def ping_interval(websocket):
#     while True:
#         try:
#             await websocket.ping()  # 핑 보내기
#             await asyncio.sleep(10)  # 10초마다 핑 보내기
#             print(f"Sent ping to {websocket.remote_address}")
#         except websockets.exceptions.ConnectionClosed:
#             break

async def broadcast(message, remote_address):
    for websocket in connected:
        if websocket.remote_address == remote_address:
            continue
        await websocket.send(message)

async def start_server(host, port):
    async with websockets.serve(chat_handler, host, port, ping_interval=10, ping_timeout=20):
        print(f"서버가 {host}:{port}에서 시작됨")
        await asyncio.Future()  # 서버가 계속 실행되도록 유지

def main():
    parser = argparse.ArgumentParser(description="Run a websocket server.")
    parser.add_argument('--host', type=str, default='0.0.0.0',
                        help='Host to run the websocket server on')
    parser.add_argument('--port', type=int, default=9998,
                        help='Port to run the websocket server on')
    
    args = parser.parse_args()

    asyncio.run(start_server(args.host, args.port))

if __name__ == "__main__":
    main()

