#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>
#include <functional>
//#include "sensor_msgs/msg/laser_scan.hpp"
#include <cmath>
#include <geometry_msgs/msg/twist.hpp>
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "nav_msgs/msg/odometry.hpp"  
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav_msgs/msg/odometry.hpp"
//#include <custom_interfaces/action/go_to_pose.hpp>
#include "robot_patrol/action/go_to_pose.hpp"

#include <geometry_msgs/msg/twist.hpp>
class GoToPose : public rclcpp :: Node
{


public:
    using GoalHandleGoToPose = rclcpp_action::ServerGoalHandle<robot_patrol::action::GoToPose>;
    

     //nav_msgs/msg/Odometry
     //ros2 interface proto nav_msgs/msg/Odometry
     

    

      explicit GoToPose(const rclcpp::NodeOptions & options = rclcpp::NodeOptions()): Node("go_to_pose", options)
        {
            using namespace std::placeholders;

            this->action_server_ = rclcpp_action::create_server<robot_patrol::action::GoToPose>(this, "go_to_pose",
                std::bind(&GoToPose::handle_goal, this, _1, _2),
                std::bind(&GoToPose::handle_cancel, this, _1),
                std::bind(&GoToPose::handle_accepted, this, _1));
            auto qos = rclcpp::QoS(10).reliability(rclcpp::ReliabilityPolicy::Reliable);  
            odom_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/fastbot_1/odom", 
            qos,
            std::bind(&GoToPose::scan_callback, this, std::placeholders::_1)
            
        );

        RCLCPP_INFO(this->get_logger(), "Action Server Ready");
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/fastbot_1/cmd_vel", 10);
        }
    

private:
    rclcpp_action::Server<robot_patrol::action::GoToPose>::SharedPtr action_server_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;


    void scan_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        

        msg->pose.pose.position.x;
        msg->pose.pose.position.y;

        //theta formula

        // quaternion values from odom
        double qx = msg->pose.pose.orientation.x;
        double qy = msg->pose.pose.orientation.y;
        double qz = msg->pose.pose.orientation.z;
        double qw = msg->pose.pose.orientation.w;

        // convert to yaw (theta)
        double theta = atan2(2.0 * (qw * qz + qx * qy), 1.0 - 2.0 * (qy * qy + qz * qz));

        current_pos_.x = msg->pose.pose.position.x;
        current_pos_.y = msg->pose.pose.position.y;
        current_pos_.theta = theta;

        //RCLCPP_INFO(this->get_logger(), "x is %f, y is %f, theta is %f", msg->pose.pose.position.x,  msg->pose.pose.position.y, theta);
    }

    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const robot_patrol::action::GoToPose::Goal> goal)
    {
        //RCLCPP_INFO(this->get_logger(), "Received goal request with x: %.2f, y: %.2f, yaw: %.2f", 
        //        goal->x, goal->y, goal->yaw);
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "Received goal request with values x is %f, y is %f, theta is %f", goal->goal_pos.x,  goal->goal_pos.y, goal->goal_pos.theta);
         RCLCPP_INFO(this->get_logger(), "Action Called");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel( const std::shared_ptr<GoalHandleGoToPose> goal_handle)
    {
    //RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
    (void)goal_handle;
    return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleGoToPose> goal_handle)
    {
        using namespace std::placeholders;
        // This needs to return quickly to avoid blocking the executor, so spin up a new thread
        std::thread{std::bind(&GoToPose::execute, this, _1), goal_handle}.detach();
    }

    void execute(const std::shared_ptr<GoalHandleGoToPose> goal_handle) {
        // control loop goes here

        const auto goal = goal_handle->get_goal();
        //Pose2D 
        auto feedback = std::make_shared<robot_patrol::action::GoToPose::Feedback>();
        //bool
        auto result = std::make_shared<robot_patrol::action::GoToPose::Result>();

        // ADD this before the if check:
  


        feedback->current_pos.x = current_pos_.x;
        feedback->current_pos.y = current_pos_.y;
        feedback->current_pos.theta = current_pos_.theta;
        goal_handle->publish_feedback(feedback);

     
        desired_pos_.x = goal->goal_pos.x;
        desired_pos_.y = goal->goal_pos.y;
        desired_pos_.theta = goal->goal_pos.theta;

        geometry_msgs::msg::Twist cmd_vel;
        
        RCLCPP_INFO(this->get_logger(), "desired_pos_x is %f, desired_pos_u is %f, desired_pos_theta is %f", desired_pos_.x,  desired_pos_.y, desired_pos_.theta);
 
        rclcpp::Rate loop_rate(1); 


        //handle_accepted is called automatically by the action server, handle_goal and handle_cancle serve as gatekeepers or if statements!

        while(rclcpp::ok())
        {
        

            double diffx = desired_pos_.x - current_pos_.x;
            double diffy = desired_pos_.y - current_pos_.y;
            
            double distance = sqrt(diffx * diffx + diffy * diffy);
            double angle = atan2(diffy, diffx);


            //angle may be wrapped but not when i calcuate difftheta, that needs to be wrapped again
            double difftheta = angle - current_pos_.theta;
            double propertheta = atan2(sin(difftheta), cos(difftheta)); //ensure proper radian values
            //double difftheta = desired_pos_.theta - current_pos_.theta;

            if (goal_handle->is_canceling()) {
                cmd_vel.linear.x = 0.0;
                cmd_vel.angular.z = 0.0;
                publisher_->publish(cmd_vel);
                result->status = false;
                goal_handle->canceled(result);
                break;  
            }

            if (distance < 0.4) {

                //stop the turning for a moment
                cmd_vel.linear.x = 0;
                cmd_vel.angular.z = 0;
                publisher_->publish(cmd_vel);
                double final_error = atan2(sin(desired_pos_.theta - current_pos_.theta), cos(desired_pos_.theta - current_pos_.theta));

                while(std::fabs(final_error) > 0.05)
                {
                
//

                        if (goal_handle->is_canceling()) {
                            cmd_vel.linear.x = 0.0;
                            cmd_vel.angular.z = 0.0;
                            publisher_->publish(cmd_vel);
                            result->status = false;
                            goal_handle->canceled(result);
                            break;  
                        }

                        cmd_vel.linear.x = 0.0;
                        cmd_vel.angular.z = 0.4 * final_error; // rotate in place
                        publisher_->publish(cmd_vel);

                        loop_rate.sleep();
                        final_error = atan2(sin(desired_pos_.theta - current_pos_.theta), cos(desired_pos_.theta - current_pos_.theta));

                }
                
                cmd_vel.linear.x = 0;
                cmd_vel.angular.z = 0;
                publisher_->publish(cmd_vel);

                RCLCPP_INFO(this->get_logger(), "Action Complete");
                result->status = true;
                goal_handle->succeed(result);

                break;
            }


            feedback->current_pos = current_pos_;
            goal_handle->publish_feedback(feedback);
            //publisher_->publish(cmd_vel);
            cmd_vel.linear.x = 0.2;

            cmd_vel.angular.z = propertheta;
            //cmd_vel.angular.z = angle - current_pos_.theta;  
            //RCLCPP_INFO(this->get_logger(), "angular z is %f", cmd_vel.angular.z);

            publisher_->publish(cmd_vel);


    
            loop_rate.sleep();
        
        
        }



 


    }
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber_;
    geometry_msgs::msg::Pose2D desired_pos_;   
    geometry_msgs::msg::Pose2D current_pos_;   


};



int main(int argc, char** argv)
{

    rclcpp::init(argc, argv);
    auto node = std::make_shared<GoToPose>();
    //simple_publisher = std::make_shared<PatrolNode>("PatrolNode", 1.0);
    //signal(SIGINT, signal_handler);
    //rclcpp::spin(simple_publisher);
    //rclcpp::shutdown();

    rclcpp::spin(node);
    rclcpp::shutdown();


    return 0;
}