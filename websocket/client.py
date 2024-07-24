import asyncio
import websockets
import argparse

async def connect_to_websocket(host, port):
    uri = f"ws://{host}:{port}"
    async with websockets.connect(uri) as websocket:
        await websocket.send("Hello Server!")
        response = await websocket.recv()
        print(f"Received from server: {response}")

def main():
    parser = argparse.ArgumentParser(description="Connect to a websocket server.")
    parser.add_argument('--ip', type=str, default='localhost',
                        help='Host of the websocket server')
    parser.add_argument('--port', type=int, default=9998,
                        help='Port of the websocket server')
    
    args = parser.parse_args()

    asyncio.run(connect_to_websocket(args.ip, args.port))

if __name__ == "__main__":
    main()
