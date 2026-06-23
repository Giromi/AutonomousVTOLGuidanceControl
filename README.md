# ✈ KRAC – Autonomous VTOL Guidance and Control System

> Development of an autonomous VTOL aircraft for the **Korean Robot Aircraft Competition (KRAC)** using **ROS2, PX4, MAVROS, vector field guidance, mission management, and autonomous flight control**.

---

## 📖 Overview

The KRAC project focuses on developing a fully autonomous VTOL aircraft capable of performing both **Fixed-Wing (FW)** and **Multicopter (MC)** missions.

The system integrates **mission planning, autonomous navigation, vector field guidance, flight control, perception, communication, and onboard system integration** into a single autonomous platform. Extensive simulation and real-flight testing were conducted to validate the complete mission workflow.

---

# 📷 Project Overview

<p align="center">
  <img src="https://github.com/user-attachments/assets/88ef9dce-c37c-408b-b64c-ab47ba33c1c6" width="900"/>
</p>

The KRAC team designed, assembled, and tested a custom VTOL aircraft for autonomous mission execution. The project covered the complete development cycle, including aircraft design, electronic integration, autonomous flight software, and field testing.

---

# 🖥 Simulation Environment

<p align="center">
  <img src="https://github.com/user-attachments/assets/0d5b033d-bf2c-469c-b160-671e2c4b4dd0" width="900"/>
</p>

Before real-world flight experiments, the complete autonomous mission was validated through a simulation environment.

### Simulation Features

- ROS2-based autonomous system
- PX4 SITL simulation
- MAVROS communication
- Mission verification
- Vector field guidance validation
- Controller performance evaluation

---

# 🛰 Mission System Architecture

<p align="center">
  <img src="https://github.com/user-attachments/assets/6c3bc6b2-d51b-46ea-bb04-972284552dc4" width="900"/>
</p>

The autonomous mission system coordinates flight control, mission management, object detection, communication, and camera operation through a ROS2-based software architecture.

### Major Components

- Autonomous Mission Manager
- Flight Controller (PX4)
- ROS2 Nodes
- Mission State Machine
- Object Detection
- Camera Gimbal Control
- Telemetry & Communication

---

# ⚡ Electronic System Integration

<p align="center">
  <img src="https://github.com/user-attachments/assets/ee0143b7-8acc-4e3a-a82c-a5a2f0ad457c" width="900"/>
</p>

The onboard electronic architecture integrates flight control hardware, companion computing, sensors, cameras, communication devices, and power management into a unified autonomous platform.

### Hardware Components

- Pixhawk Flight Controller
- Companion Computer
- GPS Module
- Telemetry Radio
- Camera
- Camera Gimbal
- ESC
- Power Distribution Board
- Battery Management

---

# ✈ Prototype Design Evolution

<p align="center">
  <img src="https://github.com/user-attachments/assets/4faf6eac-a8ba-4994-a0c4-1868c2d5e1a4" width="900"/>
</p>

Prototype 2 was developed by improving the limitations identified during the flight tests of Prototype 1. The redesigned airframe enhanced structural reliability, flight stability, and mission performance.



# ✈ Control Surface Design and Aerodynamic Analysis

The control surface was designed through an integrated workflow combining aerodynamic simulation, analytical modeling, and mechanical linkage analysis.

First, aerodynamic characteristics were evaluated using CFD, OpenVSP, and XFLR5 to estimate lift, drag, and stability derivatives. These results were incorporated into the aircraft dynamic equations to determine the required control authority.

Based on the desired roll and pitch performance, the practical control surface deflection range was calculated and the neutral position was calibrated to maximize usable servo travel while avoiding mechanical saturation.

Finally, a 2D servo linkage kinematic analysis was performed to optimize the horn geometry and linkage ratio before applying the design to the actual aircraft.

<img width="1432" height="838" alt="image" src="https://github.com/user-attachments/assets/703aad4e-fa28-467c-bd16-c342d9e146ec" />

### Key Improvements

- Improved Wing Attachment Mechanism
- Pusher Propulsion Configuration
- Redesigned Tail Structure
- Enhanced Quadrotor Stability
- Passenger Cabin Integration

---

# 🚀 FCU Mission Workflow

<p align="center">
  <img src="https://github.com/user-attachments/assets/5cb52992-ddb2-4cf4-9bac-25729a7c7c82" width="900"/>
</p>

The Flight Control Unit (FCU) autonomously manages the complete mission sequence from takeoff to landing while coordinating fixed-wing flight, multicopter flight, perception, and obstacle avoidance.

### Mission Sequence
<p align="center">
<img width="1588" height="708" alt="image" src="https://github.com/user-attachments/assets/33887730-4ca7-4a9d-9419-9c601d6f946e" />
</p>

```text
Takeoff
    │
Pre-flight Check
    │
Forward Transition
    │
Fixed-Wing Mission
    │
Target Observation
    │
Target Detection & Communication
    │
Camera Gimbal Control
    │
Back Transition
    │
Multicopter Mission
    │
Obstacle Detection
    │
Obstacle Avoidance
    │
Wind Disturbance Rejection
    │
Landing
```

---

# 🛠 Technology Stack

<p align="center">

| Flight Control | Robotics | Planning | Perception | Programming |
|:--------------:|:--------:|:--------:|:----------:|:-----------:|
| PX4 | ROS2 | Vector Field Guidance | OpenCV | C++ |
| MAVROS | Gazebo | Mission Planning | Camera System | Python |
| QGroundControl | RViz | Path Following | Gimbal Control | MATLAB |

</p>

---

# 👨‍💻 My Contributions

### Autonomous Flight Control

- Developed autonomous VTOL flight software using ROS2 and PX4.
- Implemented mission state management for fully autonomous mission execution.
- Integrated communication between the companion computer and flight controller.

### Guidance & Navigation

- Designed and implemented vector field guidance for path following.
- Developed waypoint navigation and autonomous transition logic.
- Improved flight stability during transition and mission execution.

### System Integration

- Integrated onboard sensors, cameras, telemetry, and communication systems.
- Developed ROS2 nodes for mission management and hardware interfaces.
- Conducted simulation-based verification before real flight testing.

### Aircraft Development

- Participated in airframe design improvements from Prototype 1 to Prototype 2.
- Evaluated prototype performance through flight experiments.
- Contributed to system integration and autonomous mission validation.

---

# 📚 Keywords

`ROS2` `PX4` `MAVROS` `VTOL` `Vector Field Guidance` `Autonomous Flight`
`Mission Planning` `Path Following` `Flight Control`
`Robotics` `Navigation`
