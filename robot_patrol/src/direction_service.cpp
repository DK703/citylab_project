#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
#include <custom_interfaces/srv/get_direction.hpp>
class DirectionService : public rclcpp :: Node
{

public:

    DirectionService(const std::string& node_name) : Node(node_name)
    {
    
    
        std::string name_service = "/direction_service";
        
        service_ = this->create_service<custom_interfaces::srv::GetDirection>(
        name_service,std::bind(&DirectionService::get_direction_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

    

    }


private:
    
   
    void get_direction_callback(
  const std::shared_ptr<custom_interfaces::srv::GetDirection::Request> request,
  std::shared_ptr<custom_interfaces::srv::GetDirection::Response> response)
    {
    
    

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "TEST ";
      
    response->direction = ss.str();
    
    
    }
    
    std::string node_name_;
    rclcpp::Service<custom_interfaces::srv::GetDirection>::SharedPtr service_;
    //rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;

    
};


int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DirectionService>("direction_service_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}