import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from mavsdk import System
from mavsdk.offboard import PositionNedYaw
from pynput import keyboard
import asyncio
import nest_asyncio

class MinimalSubscriber(Node):

    def __init__(self):
        super().__init__('minimal_subscriber')
        self.subscription = self.create_subscription(
            String,
            'topic',
            self.listener_callback,
            10)
        self.subscription  # prevent unused variable warning
        pass

    def listener_callback(self, msg):
        self.get_logger().info('I heard: "%s"' % msg.data)
        if msg.data == 'w': 
            print("드론 전진")
            # asyncio.run(drone_move(5.0, 0.0, -2.5, 0.0))
            asyncio.run(drone_move(5.0, 0.0, 0.0, 0.0))
        elif msg.data == 's': 
            print("드론 후진")
            # asyncio.run(drone_move(-5.0, 0.0, -2.5, 0.0))
            asyncio.run(drone_move(-5.0, 0.0, 0.0, 0.0))
        elif msg.data == 'a': 
            print("드론 왼쪽으로 이동")
            # asyncio.run(drone_move(0.0, -5.0, -2.5, 0.0))
            asyncio.run(drone_move(0.0, -5.0, 0.0, 0.0))
        elif msg.data == 'd': 
            print("드론 오른쪽으로 이동")
            # asyncio.run(drone_move(0.0, 5.0, -2.5, 0.0))
            asyncio.run(drone_move(0.0, 5.0, 0.0, 0.0))
        pass

# async def drone_move(north,east,downward,yaw):
#     loop.run_until_complete(drone.offboard.set_position_ned(PositionNedYaw(north,east,downward,yaw)))
#     pass

# 드론의 현재 위치를 저장할 변수
current_position = {"north": 0.0, "east": 0.0, "down": -2.5}

async def drone_move(relative_north, relative_east, relative_down, yaw):
    # 현재 위치에 상대적인 이동을 계산
    target_north = current_position["north"] + relative_north
    target_east = current_position["east"] + relative_east
    target_down = current_position["down"] + relative_down

    # 드론의 목표 위치를 설정하고 이동을 시작
    await drone.offboard.set_position_ned(PositionNedYaw(target_north, target_east, target_down, yaw))

    # 현재 위치 업데이트
    current_position["north"] = target_north
    current_position["east"] = target_east
    current_position["down"] = target_down

async def drone_init():
    print('drone init..')
    await drone.connect(system_address="udp://:14540")
    
    print('drone init #2..')
    async for state in drone.core.connection_state():
        if state.is_connected:
            print(f"-- Connected to drone!")
            break

    print("Waiting for drone to have a global position estimate...")
    async for health in drone.telemetry.health():
        if health.is_global_position_ok and health.is_home_position_ok:
            print("-- Global position estimate OK")
            break

    print("-- Arming")
    await drone.action.arm()

    # 드론 이륙
    print("드론 이륙 중...")
    await drone.action.takeoff()
    await asyncio.sleep(10)

    # 드론이 offboard 모드 준비 완료 상태인지 확인
    print("드론 offboard 모드 준비 중...")
    await drone.offboard.set_position_ned(PositionNedYaw(0.0, 0.0, -2.5, 0.0))

    try:
        await drone.offboard.start()
    except:
        print("Offboard 모드 시작 실패")
        return
    print('Now offboard start ...')


def main(args=None):
    rclpy.init(args=args)
    minimal_subscriber = MinimalSubscriber()

    asyncio.run(drone_init())

    rclpy.spin(minimal_subscriber)
    minimal_subscriber.destroy_node()
    rclpy.shutdown()


drone = System()
nest_asyncio.apply() #현재 이벤트 루프에 중첩 루프를 허용
loop = asyncio.get_event_loop()

if __name__ == '__main__':
    main()
    loop.close()
    asyncio.run(drone.action.land())
    pass
