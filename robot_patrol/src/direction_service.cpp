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
    
    
    float test = request->laser_data.scan_time;

    float ci = (0.0 - request->laser_data.angle_min) / request->laser_data.angle_increment;
    float start = ci - (M_PI/2) / request->laser_data.angle_increment;       //49.75
    float end   = ci + (M_PI/2) / request->laser_data.angle_increment;       //149.25

    int step = end - start;
    step = step/3;

    std::string direction;



    //hardcoded values:right is 49.75, 99.5 is center, 149.25 is left, 33 should be the steps to create the 60 degrees
  
    //iv decided on whether i want to encode values or not to either be a calculation or hardcode values
    //ranges[i]
    float total_dist_sec_right = 0;
    float total_dist_sec_front = 0;
    float total_dist_sec_left = 0;

    int rightend = start + step;
    int frontend = rightend + step;
    int leftend =  frontend + step;


    // RCLCPP_INFO(this->get_logger(), "front=%f, right=%f left=%f back=%f",request->laser_data.ranges[99.5], request->laser_data.ranges[49.75], request->laser_data.ranges[149.25], request->laser_data.ranges[199]);
    //83 and 116???
    


   
    //start, rightend
    for(int i = start; i < rightend; i++)
    {
        if(std::isfinite(request->laser_data.ranges[i]))
        {
            total_dist_sec_right += request->laser_data.ranges[i];
        }
    }

    //rightend, frontend
    for(int i = rightend; i < frontend; i++)
    {

        if(std::isfinite(request->laser_data.ranges[i]))
        {
            total_dist_sec_front += request->laser_data.ranges[i];
        }
    }

    //frontend, leftend
    for(int i = frontend; i < leftend; i++)
    {
    
        if(std::isfinite(request->laser_data.ranges[i]))
        {
            total_dist_sec_left += request->laser_data.ranges[i];
        }    
    }
    float FRONT_THRESHOLD = 1;
    if(total_dist_sec_right > total_dist_sec_front && total_dist_sec_right >  total_dist_sec_left)
    {
        if(FRONT_THRESHOLD < total_dist_sec_right)
        {
            direction = "right";
        }
        //direction = "right";
    }
    if(total_dist_sec_front > total_dist_sec_right && total_dist_sec_front >  total_dist_sec_left)
    {

        if(FRONT_THRESHOLD < total_dist_sec_front)
        {
            direction = "forward";
        }
        
    }
    if(total_dist_sec_left > total_dist_sec_front && total_dist_sec_left >  total_dist_sec_right)
    {

        if(FRONT_THRESHOLD < total_dist_sec_left)
        {
            direction = "left";
        }
        
    }

    //std::cout << "right is " << total_dist_sec_right;
    //std::cout << "front is "<< total_dist_sec_front;
    //std::cout << "left is " << total_dist_sec_left;
    RCLCPP_INFO(this->get_logger(), "start=%f, step=%d rightend=%d frontend=%d leftend=%d", start, step, rightend, frontend, leftend);

    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "rightend is " << rightend << "frontend is " << frontend << "leftend is " << leftend << 
    "right is " << total_dist_sec_right << "front is" << total_dist_sec_front << 
    "left is" << total_dist_sec_left << "direction is " << direction;


      
    response->info = ss.str();
    response->direction = direction;
    
    
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