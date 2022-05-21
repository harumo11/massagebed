# Massage Bed System

This repository has the source code and document for next generation auto massage bed system. The source code is mainly to control hardwares and written by C++. In this system, there are three hardwares as follows.

| Hardware            | Maker    | Ready to use |
| ------------------- | -------- | ------------ |
| Robot arm           | Techman  | Yes          |
| Force torque sensor | Leptrino | Yes          |
| Liner slider        | N/A      | No           |

You can control the robot arm and linear slider and receive force and torque data from force torque sensor via TCP socket.

## Dependency

- [Poco library](https://pocoproject.org/)
- Windows/Linux(Ubuntu)

## Available functions

### Robot arm

- Real-time joint velocity control.
- Real-time end effector velocity control.

### Force torque sensor

- Force and torque measurement.

### Linear slider

- Not yet implementing.

