import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class MinimalPublisher(Node):

    def __init__(self):
        super().__init__('minimal_publisher')
        self.publisher_ = self.create_publisher(String, 'topic', 10)
    
    def getch(self):
        import termios
        import sys, tty
        def _getch():
            fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(fd)
            try:
                tty.setraw(fd)
                ch = sys.stdin.read(1)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            return ch
        return _getch()
    
    def loop(self):
        while True:
            ch= self.getch()
            print('ch=',ch)
            if ch=='e':
                raise SystemExit
                break
            else:
                 msg = String()
                 msg.data = ch
                 self.publisher_.publish(msg)
                 self.get_logger().info('Publishing: "%s"' % msg.data)
        pass


def main(args=None):
    rclpy.init(args=args)
    minimal_publisher = MinimalPublisher()
    print('start spin')
    try:
        minimal_publisher.loop()
        rclpy.spin(minimal_publisher)
    except SystemExit: 
         rclpy.logging.get_logger("Quitting").info('Done')
    print('end spin')
    minimal_publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
