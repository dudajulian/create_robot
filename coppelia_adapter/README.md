# Coppelia Adapter for Ros2 Control

## Setup Coppelia
The `sim_ros2_interface` for CoppeliaSim does not work with topics of type `sensor_msgs/msg/JointState` out of the box. All used topic types have to be added to the list in `meta/inerfaces.txt` and the package has to be recompiled following the instructions from the ROS2 Tutorial of the [CoppeliaSim Manual](https://manual.coppeliarobotics.com/).
> TIPP: If the Coppelia directory is inside our ROS2 workspace, it can be ignored at compilation by adding an empty file named `COLCON_IGNORE` to it. THis avoids conflicts of duplicated packages.
