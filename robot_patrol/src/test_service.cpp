#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
#include <custom_interfaces/srv/get_direction.hpp>
#include <memory>
class DirectionClient : public rclcpp :: Node
{

public:
    DirectionClient(const std::string& node_name) : Node(node_name)
    {
    
        //create client
        std::string name_service = "/direction_service";
        client_ = this->create_client<custom_interfaces::srv::GetDirection>(name_service);

        //wait for service
        while (!client_->wait_for_service(std::chrono::seconds(1))) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Service %s not available, waiting again...", name_service.c_str());
        }
    
    }



    

     void send_request()
        {
        
        auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
        request->laser_data =  sensor_msgs::msg::LaserScan();
        auto result_future = client_->async_send_request(request);

        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) ==
            rclcpp::FutureReturnCode::SUCCESS)
            {
        
            auto response = result_future.get();



            RCLCPP_INFO(this->get_logger(), "Status Report: %s", response->direction.c_str());
            }
        
        else
            {
            RCLCPP_ERROR(this->get_logger(), "Failed to call service");
            }
        
        }
    
    private:
        rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr client_;
};



int main(int argc, char** argv)
{
    // Initialize the ROS communication
    rclcpp::init(argc, argv);
    
    // Declare the node constructor
    auto client = std::make_shared<DirectionClient>("direction_client_node");
    
    // Run the send_request() method
    client->send_request();
    
    // Shutdown the ROS communication
    rclcpp::shutdown();
    return 0;
}