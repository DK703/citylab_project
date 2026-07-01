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
            std::chrono::milliseconds(static_cast<int>(timer_period * 1000)),
            std::bind(&PatrolNode::timer_callback, this));
            //std::chrono::milliseconds(100),  // 100ms = 10 Hz
            //std::bind(&PatrolNode::timer_callback, this));

            auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);  

            // Topic name is fastbot_1/scan
        laser_subscriber_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "fastbot_1/scan", //even tho we are using the sensor_msgs/msg/LaserScan type, the topic we subscribe is /fastbot_1/scan 
            qos,
            std::bind(&PatrolNode::scan_callback, this, std::placeholders::_1)
        );

        
        
      

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("fastbot_1/cmd_vel", 10);
        linearx = 0.0;
        obstacle_detected_ = false;
        angularz = 0.0;
        turning_ = false;
        request_pending_ = false;


        client_ = this->create_client<custom_interfaces::srv::GetDirection>("/direction_service");

        while (!client_->wait_for_service(std::chrono::seconds(1))) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Service not available, waiting again...");
        }
        

    }

    void stop_rover()
    {
        auto stop_msg = geometry_msgs::msg::Twist();  // All fields default to zero, which represents stopping the rover
        publisher_->publish(stop_msg);
        RCLCPP_INFO(this->get_logger(), "Publishing stop message before shutdown");
    }

    
private:
    
    void send_request(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        auto request = std::make_shared<custom_interfaces::srv::GetDirection::Request>();
        request->laser_data = *msg;
        RCLCPP_INFO(this->get_logger(), "front=%f, right=%f left=%f back=%f",msg->ranges[1], msg->ranges[338], msg->ranges[112], msg->ranges[225]);
        client_->async_send_request(
            request,
            std::bind(&PatrolNode::response_callback, this, std::placeholders::_1));
    }

    void response_callback(
        rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedFuture future)
    {   
        //request_pending_ = false;

        //auto now = this->get_clock()->now();

        //if (direction_initialized_) {
        //    double elapsed_ms = (now - last_direction_change_).nanoseconds() / 1e6;
        //    if (elapsed_ms < DIRECTION_HOLD_MS) return;
        //}

       // direction_initialized_ = true;
       // last_direction_change_ = now;


 

        auto response = future.get();
        
        RCLCPP_INFO(this->get_logger(), "Status Report: %s", response->direction.c_str());

      

        
            if(response->direction == "forward")
            {
                linearx = 0.1;
                angularz = 0;
                turning_ = false;
            }
            else if(response->direction == "right")
            {
                linearx = 0.1;
                angularz = -0.5;
                turning_ = false;
            }
            else if(response->direction == "left")
            {
                linearx = 0.1;
                angularz = 0.5;
                turning_ = false;
            }
            else
            {
        
            }
        
    }



    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        //if (!request_pending_) {  
         //   request_pending_ = true;
            send_request(msg);
        //}
 
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
        //RCLCPP_INFO(this->get_logger(), "%f is angularz", angularz);
    }




    std::string node_name_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Service<custom_interfaces::srv::GetDirection>::SharedPtr service_;
    rclcpp::Client<custom_interfaces::srv::GetDirection>::SharedPtr client_;
    float angle;
    float direction_;
    float commited_direction_;
    bool obstacle_detected_;
    float linearx;
    float lineary;
    float angularz;
    bool turning_;
    bool request_pending_ = false;
    rclcpp::Time last_direction_change_;
    bool direction_initialized_ = false;
    const double DIRECTION_HOLD_MS = 500.0;
    
    //float[] arr;


};

std::shared_ptr<PatrolNode> simple_publisher;



void signal_handler(int signum)
{
    (void)signum;
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