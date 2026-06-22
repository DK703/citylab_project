#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
//#include "sensor_msgs/msg/laser_scan.hpp"
#include <sensor_msgs/msg/laser_scan.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <custom_interfaces/srv/get_direction.hpp>



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

        std::string name_service = "/direction_service";
        
        service_ = this->create_service<custom_interfaces::srv::GetDirection>(
        name_service,std::bind(&PatrolNode::get_direction_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/fastbot_1/cmd_vel", 10);
        linearx = 0.1;
        obstacle_detected_ = false;
        angularz = 0.0;
        turning_ = false;
        

    }

    void stop_rover()
    {
        auto stop_msg = geometry_msgs::msg::Twist();  // All fields default to zero, which represents stopping the rover
        publisher_->publish(stop_msg);
        RCLCPP_INFO(this->get_logger(), "Publishing stop message before shutdown");
    }

    
private:

    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {

 
    }

    void get_direction_callback( const std::shared_ptr<custom_interfaces::srv::GetDirection::Request> request,
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
        int total_dist_sec_right = 0;
        int total_dist_sec_front = 0;
        int total_dist_sec_left = 0;

        int rightend = start + step;
        int frontend = rightend + step;
        int leftend =  frontend + step;
    
        for(int i = start; i < rightend; i++)
        {
    
            total_dist_sec_right += request->laser_data.ranges[i];
    
        }
        for(int i = rightend; i < frontend; i++)
        {
    
            total_dist_sec_front += request->laser_data.ranges[i];
    
        }
        for(int i = frontend; i < leftend; i++)
        {
    
            total_dist_sec_left += request->laser_data.ranges[i];
    
        }

        if(total_dist_sec_right > total_dist_sec_front && total_dist_sec_right >  total_dist_sec_left)
        {
            direction = "right";
        }
        if(total_dist_sec_front > total_dist_sec_right && total_dist_sec_front >  total_dist_sec_left)
        {
            direction = "front";
        }
        if(total_dist_sec_left > total_dist_sec_front && total_dist_sec_left >  total_dist_sec_front)
        {
            direction = "left";
        }

        std::cout << "right is " << total_dist_sec_right;
        std::cout << "front is "<< total_dist_sec_front;
        std::cout << "left is " << total_dist_sec_left;

    
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "rightend is " << rightend << "frontend is " << frontend << "leftend is " << leftend << 
        "right is " << total_dist_sec_right << "front is" << total_dist_sec_front << 
        "left is" << total_dist_sec_left << "direction is " << direction;
      
        
        if(direction == "forward")
        {
            linearx = 0.1;
            angularz = 0;
        
        }
        else if(direction == "left")
        {
        
        
            linearx = 0.1;
            angularz = 0.5;
        }
        else if(direction == "right")
        {
            linearx = 0.1;
            angularz = -0.5;
        
        }
        else
        {
        
        }
        response->direction = ss.str();


    
    
    
    }

    void timer_callback()
    {
        auto ros_time_stamp = this->get_clock()->now();
        //RCLCPP_INFO(this->get_logger(), "%s is alive...Time(nanoseconds=%ld, clock_type=ROS_TIME)",
           //         node_name_.c_str(),
            //        ros_time_stamp.nanoseconds());
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = linearx;
        msg.angular.z = angularz;
        //how we directly publish the values!
        publisher_->publish(msg);
        //RCLCPP_INFO(this->get_logger(), "%f is linearx", linearx);
    }




    std::string node_name_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Service<custom_interfaces::srv::GetDirection>::SharedPtr service_;
    float angle;
    float direction_;
    float commited_direction_;
    bool obstacle_detected_;
    float linearx;
    float lineary;
    float angularz;
    bool turning_;
    
    //float[] arr;


};

std::shared_ptr<PatrolNode> simple_publisher;


void signal_handler(int signum)
{
    simple_publisher->stop_rover();
    rclcpp::shutdown();
}


int main(int argc, char** argv)
{

    rclcpp::init(argc, argv);
    //auto node = std::make_shared<PatrolNode>("PatrolNode", 1.0);
    simple_publisher = std::make_shared<PatrolNode>("PatrolNode", 1.0);
    signal(SIGINT, signal_handler);
    rclcpp::spin(simple_publisher);
    //rclcpp::shutdown();


    return 0;
}