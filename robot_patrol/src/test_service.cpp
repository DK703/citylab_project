#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
#include <custom_interfaces/srv/get_direction.hpp>
#include <memory>
#include <sensor_msgs/msg/laser_scan.hpp>

class DirectionClient : public rclcpp::Node
{
public:
    DirectionClient(const std::string& node_name) : Node(node_name)
    {
        auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);

        laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/fastbot_1/scan", qos,
            std::bind(&DirectionClient::send_callback, this, std::placeholders::_1));

        client_ = this->create_client<custom_interfaces::srv::GetDirection>("/direction_service");

        while (!client_->wait_for_service(std::chrono::seconds(1))) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Service not available, waiting again...");
        }
    }

private:
    void send_request(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
        request->laser_data = *msg;

        client_->async_send_request(
            request,
            std::bind(&DirectionClient::response_callback, this, std::placeholders::_1));
    }

    void response_callback(
        rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedFuture future)
    {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "Status Report: %s", response->direction.c_str());
    }

    void send_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        send_request(msg);
    }

    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr client_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<DirectionClient>("direction_client_node");
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}