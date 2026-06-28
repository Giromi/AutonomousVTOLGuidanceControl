# KRAC – Autonomous VTOL Aircraft Development

> Design, integration, and validation of an autonomous VTOL aircraft for the **Korean Robot Aircraft Competition (KRAC)** using **ROS2, PX4, MAVROS, vector field guidance, mission management, and autonomous flight control**.

## Highlights

| ROS2 Gazebo Simulation | Real VTOL Flight Test | Autonomous Waypoint Flight |
|:--------------------:|:---------------------:|:----------------------------:|
| <img src="https://github.com/user-attachments/assets/9343b1f8-c63b-4d8a-8c1f-4e12d4ea50d6" width="320"> | <img width="320" height="180" alt="KakaoTalk_Video_2026-06-28-23-40-39" src="https://github.com/user-attachments/assets/7ca368d2-fb40-4ce8-889d-c5e0b57f7ffe" /> | <img src="https://github.com/user-attachments/assets/1c41e4a2-a686-4f5f-95e7-2a38c3d0155c" width="320"> |



## Overview

This project presents the end-to-end development of an autonomous VTOL aircraft for the Korean Robot Aircraft Competition (KRAC).

The project covered the full engineering workflow, including aircraft design, aerodynamic analysis, onboard electronic integration, autonomous flight software development, guidance and control, mission management, simulation validation, and real flight testing.

The final system was designed to execute autonomous takeoff, forward transition, fixed-wing flight, target observation, back transition, multicopter mission execution, obstacle avoidance, disturbance rejection, and landing.

---

## Technology Stack

| Flight Control | Robotics | Guidance & Planning | Perception | Programming |
|:--------------:|:--------:|:-------------------:|:----------:|:-----------:|
| PX4 | ROS2 | Vector Field Guidance | OpenCV | C++ |
| MAVROS | Gazebo | Mission Planning | Camera System | Python |
| QGroundControl | RViz | Path Following | Gimbal Control | MATLAB |

---


## Project Overview

- Designed and built a custom autonomous VTOL aircraft from prototype development to real flight testing.
- Designed and built a custom autonomous VTOL aircraft.
- Developed a ROS2–PX4 autonomous flight framework.
- Implemented autonomous mission protocols for VTOL operation.
- Integrated vector field guidance into the flight system.
- Performed aerodynamic analysis using CFD, OpenVSP, and XFLR5.
- Validated autonomous missions through simulation and outdoor flight tests.
---


<p align="center">
  <img src="https://github.com/user-attachments/assets/88ef9dce-c37c-408b-b64c-ab47ba33c1c6" width="900"/>
</p>

The KRAC team developed a custom VTOL aircraft capable of autonomously executing competition missions requiring both fixed-wing and multicopter flight.

The project involved the entire engineering workflow, from conceptual aircraft design and aerodynamic analysis to onboard electronics integration, autonomous software development, simulation-based verification, and outdoor flight testing.

---
## My Contributions

### Autonomous Flight Software

- Developed a ROS2-based autonomous mission framework for a PX4-controlled VTOL aircraft.
- Implemented the complete autonomous mission protocol, including takeoff, waypoint navigation, forward transition, fixed-wing flight, back transition, multicopter operation, and autonomous landing.
- Developed mission state management for reliable autonomous mission execution.

### Guidance Integration

- Integrated an existing vector field guidance algorithm into the autonomous VTOL flight framework.
- Implemented waypoint-following and mission sequencing using the vector field guidance module.
- Verified guidance performance through ROS2 simulation and outdoor flight testing.

### VTOL Simulation

- Built a ROS2–PX4 SITL simulation environment for autonomous VTOL development.
- Implemented MAVROS communication between the companion computer and the PX4 flight controller.
- Validated autonomous mission logic through simulation before real flight experiments.

### System Integration

- Integrated onboard sensors, telemetry, cameras, and companion computing into the autonomous flight system.
- Developed ROS2 nodes for mission management and hardware interfaces.
- Participated in aircraft integration, field deployment, and autonomous mission validation.

---



## Simulation Environment

<p align="center">
  <img src="https://github.com/user-attachments/assets/0d5b033d-bf2c-469c-b160-671e2c4b4dd0" width="900"/>
</p>

Before real-world flight experiments, the complete autonomous mission workflow was validated through a simulation environment.

### Simulation Features

- ROS2-based autonomous mission system
- PX4 SITL simulation
- MAVROS communication interface
- Mission sequence verification
- Vector field guidance validation
- Controller performance evaluation
- Pre-flight logic testing before outdoor experiments

---

## Mission System Architecture

<p align="center">
  <img src="https://github.com/user-attachments/assets/6c3bc6b2-d51b-46ea-bb04-972284552dc4" width="900"/>
</p>

The autonomous mission system coordinates flight control, mission management, object detection, communication, and camera operation through a ROS2-based software architecture.

### Major Components

- Autonomous Mission Manager
- PX4 Flight Controller
- ROS2 Mission Nodes
- MAVROS Interface
- Mission State Machine
- Object Detection Module
- Camera Gimbal Control
- Telemetry and Communication System

---

## Electronic System Integration

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
- Battery Management System

---

## Prototype Design Evolution

<p align="center">
  <img src="https://github.com/user-attachments/assets/4faf6eac-a8ba-4994-a0c4-1868c2d5e1a4" width="900"/>
</p>

Prototype 2 was developed by improving the limitations identified during the flight tests of Prototype 1. The redesigned airframe improved structural reliability, flight stability, and mission performance.

### Key Improvements

- Improved wing attachment mechanism
- Pusher propulsion configuration
- Redesigned tail structure
- Enhanced quadrotor stability
- Passenger cabin integration
- Improved maintainability for field testing

---

## Control Surface Design and Aerodynamic Analysis

The control surface was designed through an integrated workflow combining aerodynamic simulation, analytical modeling, and mechanical linkage analysis.

First, aerodynamic characteristics were evaluated using CFD, OpenVSP, and XFLR5 to estimate lift, drag, and stability derivatives. These results were incorporated into the aircraft dynamic equations to determine the required control authority.

Based on the desired roll and pitch performance, the practical control surface deflection range was calculated, and the neutral servo position was calibrated to maximize usable servo travel while avoiding mechanical saturation.

Finally, a 2D servo linkage kinematic analysis was performed to optimize the horn geometry and linkage ratio before applying the design to the actual aircraft.

<p align="center">
  <img src="https://github.com/user-attachments/assets/703aad4e-fa28-467c-bd16-c342d9e146ec" width="900"/>
</p>

### Design Workflow

```text
Aerodynamic Analysis
        │
        ▼
CFD / OpenVSP / XFLR5
        │
        ▼
Aircraft Dynamic Model
        │
        ▼
Required Control Authority
        │
        ▼
Control Surface Deflection Range
        │
        ▼
Servo Neutral Calibration
        │
        ▼
2D Linkage Kinematic Analysis
        │
        ▼
Final Aircraft Application
```

Key Contributions

* Performed aerodynamic analysis using CFD, OpenVSP, and XFLR5.
* Estimated aerodynamic coefficients for control surface sizing.
* Calculated practical control surface deflection limits.
* Optimized the neutral servo angle for maximum usable authority.
* Designed servo horn and linkage geometry through 2D kinematic analysis.
* Applied the optimized control surface mechanism to the final VTOL aircraft.

⸻


---
---

## FCU Mission Workflow

<p align="center">
  <img src="https://github.com/user-attachments/assets/5cb52992-ddb2-4cf4-9bac-25729a7c7c82" width="900"/>
</p>

The Flight Control Unit (FCU) autonomously manages the complete mission sequence from takeoff to landing while coordinating fixed-wing flight, multicopter flight, perception, and obstacle avoidance.

### Mission Sequence

```mermaid
flowchart TD
    A[Takeoff]
    --> B[Forward Transition]
    --> C[Fixed-Wing Mission]
    --> D[Target Observation]

    D --> E[Target Detection]
    E --> F[Camera Gimbal Control]

    F --> G[Back Transition]
    G --> H[Multicopter Mission]

    H --> I[Obstacle Avoidance]
    I --> J[Wind Disturbance Rejection]
    J --> K[Landing]
```

---

## Keywords

`ROS2` `PX4` `MAVROS` `VTOL` `Vector Field Guidance`
`Autonomous Flight` `Mission Planning` `Path Following`
`Flight Control` `Aircraft Design` `Aerodynamic Analysis`
`Robotics` `Navigation` `System Integration`
