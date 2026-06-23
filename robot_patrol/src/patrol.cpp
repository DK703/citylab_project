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
            "/scan", //even tho we are using the sensor_msgs/msg/LaserScan type, the topic we subscribe is /fastbot_1/scan 
            qos,
            std::bind(&PatrolNode::scan_callback, this, std::placeholders::_1)
        );

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
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

        float ci = (0.0 - msg->angle_min) / msg->angle_increment; //rounds to 99.5 (center, aka the front?)
        float start = ci - (M_PI/2) / msg->angle_increment;       //49.75
        float end   = ci + (M_PI/2) / msg->angle_increment;       //149.25
        
        
        //ranges go from about 0 to 200;
        float startindex = 0;
        //float middleindex = len(msg->ranges) / 2;
        //float endindex = len(msg->ranges)- 1;

        //start = 33;
        //end = 166;

        //note, info conflicts with a previous graph given on the previous class. These values are placeholders as a result

        RCLCPP_INFO(this->get_logger(), "start=%f, end=%f, angle_min=%f, increment=%f centerindex=%f max_angle = %f",
    start, end, msg->angle_min, msg->angle_increment, ci, msg->angle_max);

        //RCLCPP_INFO(this->get_logger(), "start index is %f, middle is %f, end is %f", startindex, middleindex, endindex);

        
        //hardcode with big number
        float min_distance = 999999.0;
        float min_index = 999999.0; //min_index is meant to show what angle we got the minimum distance;
        
        float max_distance = 0;
        float max_index = 0;

        std::vector<float>::iterator it;
        float howmany = 0;

        for (it = msg->ranges.begin(); it != msg->ranges.end(); ++it) {
  //std::cout << *it << "\n";
            howmany++;
        }
 RCLCPP_INFO(this->get_logger(), "how many=%f",howmany);

 //center to left scan. Not only is size of range 450, but its starts from the center here, and start index is a negative.

        //49.75 is rounted to 49 instead of 50
        for(int i = 1; i < 112; i++)
        {
         //std::isfinite(msg->ranges[i]) is used to check for an infinite value, which can mess things up;

            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] < min_distance) {
                min_distance = msg->ranges[i];
                min_index = i;
                }

            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] > max_distance){
            max_distance = msg->ranges[i];
            max_index = i;

            }
           
        }
        //right to center
        for(int i = 338; i < 448; i++)
        {
         //std::isfinite(msg->ranges[i]) is used to check for an infinite value, which can mess things up;

            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] < min_distance) {
                min_distance = msg->ranges[i];
                min_index = i;
                }

            if (std::isfinite(msg->ranges[i]) && msg->ranges[i] > max_distance){
            max_distance = msg->ranges[i];
            max_index = i;

            }
           
        }
//good for debugging
    RCLCPP_INFO(this->get_logger(), "front=%f, right=%f left=%f back=%f",msg->ranges[1], msg->ranges[340], msg->ranges[112], msg->ranges[244]);

        //RCLCPP_INFO(this->get_logger(), "%f is min distance and %f is min index", min_distance, min_index);

        //step 5: find safest area!

        


        //RCLCPP_INFO(this->get_logger(), "%f is max distance and %f is max index", max_distance, max_index);
        //direction_ = max_index * msg->angle_increment * 3.141590;
        direction_ = msg->angle_min + max_index * msg->angle_increment;

        if (direction_ > M_PI) {
            direction_ -= 2.0 * M_PI;
        }
        
        //obstacle detected at right, move left
        if(max_index > 338 && max_index < 448)
        {
        direction_ = direction_ * 1.0;
        
        }
        //obstacle detected at left, move right
        else if(max_index > 0 && max_index < 112)
        {
        
        direction_ = direction_ * -1.0;
        
        }
        else
        {
        //direction_ = 0;
        }
        
        //angularz = direction_/2;

        if(min_distance < 0.35f)
        {
            obstacle_detected_ = true;
        }
        else
        {
            obstacle_detected_ = false;
        }

        if(obstacle_detected_ && !turning_)
        {
          //RCLCPP_INFO(this->get_logger(), "obstacle detected at index %f", max_index);
          //RCLCPP_INFO(this->get_logger(), "%f is direction", direction_);
          //angularz = direction_/2;
          commited_direction_ = direction_; 
          turning_ = true;
        }

        if (turning_) {
            //angularz = 0.1;
            angularz = commited_direction_;
          RCLCPP_INFO(this->get_logger(), "obstacle detected at index %f", max_index);
          RCLCPP_INFO(this->get_logger(), "%f is direction", direction_);
          RCLCPP_INFO(this->get_logger(), "%f is commited direction", commited_direction_);
          float front_distance = msg->ranges[0];
          float front_distance2 = msg->ranges[1];
            if (front_distance > 0.5f && front_distance2 > 0.5f) {
                turning_ = false;
                angularz = 0.0;
                RCLCPP_INFO(this->get_logger(), "turning set to false");
            }
        }


/*
        if (min_distance < 0.35f && !turning_) {
            obstacle_detected_ = true;
            RCLCPP_INFO(this->get_logger(), "obstacle detected at index %f", max_index);
            RCLCPP_INFO(this->get_logger(), "%f is direction", direction_);
            angularz = direction_/2;
            RCLCPP_INFO(this->get_logger(), "%f is angularz", angularz);
            //linearx = 0;
            
            //RCLCPP_INFO(this->get_logger(), "%f is direction", direction_);
        } else {
            obstacle_detected_ = false;
            angularz = 0;
        }  
*/      
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