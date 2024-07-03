/****************************************************************************
 *
 * Copyright 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "px4_ros_com/OffboardControl.hpp"

static std::array<WayPoint, 11> _star_array = {{
    {100.0f, 0.0f, -5.0f, 0.0f},
    {40.45f, 29.39f, -5.0f, 0.0f},
    {30.9f, 95.11f, -5.0f, 0.0f},
    {-15.45f, 47.55f, -5.0f, 0.0f},
    {-80.9f, 58.78f, -5.0f, 0.0f},
    {-50.0f, 0.0f, -5.0f, 0.0f},
    {-80.9f, -58.78f, -5.0f, 0.0f},
    {-15.45f, -47.55f, -5.0f, 0.0f},
    {30.9f, -95.11f, -5.0f, 0.0f},
    {40.45f, -29.39f, -5.0f, 0.0f},
    {100.0f, 0.0f, -5.0f, 0.0f}
}};

static std::array<WayPoint, 4> _square_array2 = {{
    {100.0f, 0.0f, -10.0f, 0.0f, true},
    {100.0f, 100.0f, -15.0f, 0.0f, true},
    {0.0f, 100.0f, -5.0f, 0.0f, true},
    {0.0f, 0.0f, -5.0f, 0.0f, true}
}};

static std::array<WayPoint, 4> _square_array1 = {{
    {100.0f, 0.0f, -10.0f, 0.0f},
    {100.0f, 100.0f, -15.0f, 0.0f},
    {0.0f, 100.0f, -5.0f, 0.0f},
    {0.0f, 0.0f, -5.0f, 0.0f}
}};

static std::array<WayPoint, 2> _basic_array = {{
    {100.0f, 0.0f, -10.0f, 3.14f, true},
    {0.0f, 0.0f, -5.0f, -3.14f, true}
}};

template <typename T>
static void _iter_way_points(const T& way_points) {
    for (const WayPoint& wp : way_points) {
        OffboardControl::set_way_point(wp);
    }
}


static void _ready_for_test(void) {
    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});
    // _iter_way_points(_square_array1);
    _iter_way_points(_star_array);
    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});
    OffboardControl::set_way_point({0.0, 0.0, 0.0, -3.14});
}

int main(int argc, char *argv[])
{
	std::cout << "Starting offboard control node..." << std::endl;
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    _ready_for_test();

	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<OffboardControl>());
	rclcpp::shutdown();
	return 0;
}
