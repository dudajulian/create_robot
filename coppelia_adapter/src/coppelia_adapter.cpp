#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <std_msgs/msg/float64.hpp>

class DiffDriveAdapter : public rclcpp::Node
{
public:
    DiffDriveAdapter() : Node("diff_drive_adapter")
    {
        // Subscribe to diff_drive_controller's wheel commands
        wheel_cmd_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "wheel_cmd", 10,
            std::bind(&DiffDriveAdapter::wheelCmdCallback, this, std::placeholders::_1));

        // Create publishers for CoppeliaSim wheel speeds
        left_wheel_pub_ = this->create_publisher<std_msgs::msg::Float64>(
            "left_wheel_speed", 10);
        right_wheel_pub_ = this->create_publisher<std_msgs::msg::Float64>(
            "right_wheel_speed", 10);
    }

private:
    void wheelCmdCallback(const std_msgs::msg::Float64MultiArray::SharedPtr msg)
    {
        if (msg->data.size() < 2) {
            RCLCPP_ERROR(this->get_logger(), "Received wheel command with insufficient data");
            return;
        }

        // Create and publish messages
        auto left_msg = std::make_unique<std_msgs::msg::Float64>();
        auto right_msg = std::make_unique<std_msgs::msg::Float64>();
        
        // diff_drive_controller publishes [left_wheel_velocity, right_wheel_velocity]
        left_msg->data = msg->data[0];
        right_msg->data = msg->data[1];

        left_wheel_pub_->publish(std::move(left_msg));
        right_wheel_pub_->publish(std::move(right_msg));
    }

    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_cmd_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr left_wheel_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr right_wheel_pub_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DiffDriveAdapter>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}