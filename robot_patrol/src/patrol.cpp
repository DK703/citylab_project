#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
//#include "sensor_msgs/msg/laser_scan.hpp"
#include <sensor_msgs/msg/laser_scan.hpp>
#include <geometry_msgs/msg/twist.hpp>
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

            auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);  

            // Topic name is fastbot_1/scan
        laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/fastbot_1/scan", //even tho we are using the sensor_msgs/msg/LaserScan type, the topic we subscribe is /fastbot_1/scan 
            qos,
            std::bind(&PatrolNode::scan_callback, this, std::placeholders::_1)
        );

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        

    }

    
private:

    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        float start = (msg->angle_min * -1 - M_PI/2) / msg->angle_increment;
        float end   = (msg->angle_min * -1 + M_PI/2) / msg->angle_increment;

        //hardcode with big number
        float min_distance = 999999.0;
        float min_index = 999999.0; //min_index is meant to show what angle we got the minimum distance;
        for(int i = start; i < end; i++)
        {
         //std::isfinite(msg->ranges[i]) is used to check for an infinite value, which can mess things up;

            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] < min_distance) {
                min_distance = msg->ranges[i];
                min_index = i;
                }
           
        }


        RCLCPP_INFO(this->get_logger(), "%f is min distance and %f is min index", min_distance, min_index);

        //step 5: find safest area!

        float max_distance = 0;
        float max_index = 0;

        for(int i = start; i < end; i++)
        {
            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] > max_distance){
            max_distance = msg->ranges[i];
            max_index = i;

            }
        }

        RCLCPP_INFO(this->get_logger(), "%f is max distance and %f is max index", max_distance, max_index);
        direction_ = msg->angle_min + max_index * msg->angle_increment;

        if (min_distance < 0.35f) {
            obstacle_detected_ = true;
        } else {
            obstacle_detected_ = false;
        }        
    }

    void timer_callback()
    {
        auto ros_time_stamp = this->get_clock()->now();
        RCLCPP_INFO(this->get_logger(), "%s is alive...Time(nanoseconds=%ld, clock_type=ROS_TIME)",
                    node_name_.c_str(),
                    ros_time_stamp.nanoseconds());
    }

    std::string node_name_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    float angle;
    float direction_;
    bool obstacle_detected_;
    //float[] arr;


};

int main(int argc, char** argv)
{

    rclcpp::init(argc, argv);
    auto node = std::make_shared<PatrolNode>("PatrolNode", 1.0);
    rclcpp::spin(node);
    rclcpp::shutdown();


    return 0;
}