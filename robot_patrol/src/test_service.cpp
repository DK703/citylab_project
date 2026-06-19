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

    void send_request()
    {
        auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
        request->laser_data = sensor_msgs::msg::LaserScan();
        //apparently this one isnt asyncrhonous????

        /*

        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) ==
    rclcpp::FutureReturnCode::SUCCESS)

        */
        future_ = client_->async_send_request(request).future.share();
    }

    bool future_ready() const
    {
        return future_.valid() &&
               future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    custom_interfaces::srv::GetDirection::Response::SharedPtr get_future_result()
    {
        return future_.get();
    }

    void send_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        // hook up later
    }

private:
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr client_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedFuture future_;
};
//std::shared_ptr<DirectionClient> DC;


//send_request() apparently calls DC already, meaining if DC is in main we have it twice


int main(int argc, char** argv)
{
    // Initialize the ROS communication
    rclcpp::init(argc, argv);
    
    // Declare the node constructor
    auto client = std::make_shared<DirectionClient>("direction_client_node");
    
    client->send_request();

    while (rclcpp::ok()) {
    // Pause the program execution, waits for a request to kill the node (ctrl+c)
        rclcpp::spin_some(client);
    
        if (client->future_ready()) {
            try {
            // Checks the future for a response from the Service
            // while the system is running. 
            // If the Service has sent a response, the result will be written
            // to a log message.
            auto response = client->get_future_result();
            RCLCPP_INFO(client->get_logger(), "Pretty message");
        } catch (const std::exception& e) {
            // Display the message on the console
            RCLCPP_INFO(client->get_logger(), "Service call failed: %s", e.what());
        }
        break;
    }
}
            
// Shutdown the ROS communication
rclcpp::shutdown();

return 0;
}