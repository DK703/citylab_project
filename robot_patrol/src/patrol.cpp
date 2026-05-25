#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
class PatrolNode : public rclcpp :: Node
{



public:
    PatrolNode(const std::string& node_name, double timer_period = 2.0) : Node(node_name), node_name_(node_name)
    {
        timer_ = this->create_wall_timer(
            //std::chrono::milliseconds(static_cast<int>(timer_period * 1000)),
            //std::bind(&ObstacleDetectorNode::timer_callback, this));
            std::chrono::milliseconds(100),  // 100ms = 10 Hz
            std::bind(&PatrolNode::timer_callback, this));

    }

    
private:

    void timer_callback()
    {
        auto ros_time_stamp = this->get_clock()->now();
        RCLCPP_INFO(this->get_logger(), "%s is alive...Time(nanoseconds=%ld, clock_type=ROS_TIME)",
                    node_name_.c_str(),
                    ros_time_stamp.nanoseconds());
    }

    std::string node_name_;
    rclcpp::TimerBase::SharedPtr timer_;


};

int main(int argc, char** argv)
{

    rclcpp::init(argc, argv);
    auto node = std::make_shared<PatrolNode>("PatrolNode", 1.0);
    rclcpp::spin(node);
    rclcpp::shutdown();


    return 0;
}