// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interfaces:srv/GetDirection.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__SRV__DETAIL__GET_DIRECTION__BUILDER_HPP_
#define CUSTOM_INTERFACES__SRV__DETAIL__GET_DIRECTION__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interfaces/srv/detail/get_direction__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetDirection_Request_laser_data
{
public:
  Init_GetDirection_Request_laser_data()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::srv::GetDirection_Request laser_data(::custom_interfaces::srv::GetDirection_Request::_laser_data_type arg)
  {
    msg_.laser_data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::GetDirection_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::GetDirection_Request>()
{
  return custom_interfaces::srv::builder::Init_GetDirection_Request_laser_data();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_GetDirection_Response_compass
{
public:
  explicit Init_GetDirection_Response_compass(::custom_interfaces::srv::GetDirection_Response & msg)
  : msg_(msg)
  {}
  ::custom_interfaces::srv::GetDirection_Response compass(::custom_interfaces::srv::GetDirection_Response::_compass_type arg)
  {
    msg_.compass = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::GetDirection_Response msg_;
};

class Init_GetDirection_Response_info
{
public:
  explicit Init_GetDirection_Response_info(::custom_interfaces::srv::GetDirection_Response & msg)
  : msg_(msg)
  {}
  Init_GetDirection_Response_compass info(::custom_interfaces::srv::GetDirection_Response::_info_type arg)
  {
    msg_.info = std::move(arg);
    return Init_GetDirection_Response_compass(msg_);
  }

private:
  ::custom_interfaces::srv::GetDirection_Response msg_;
};

class Init_GetDirection_Response_direction
{
public:
  Init_GetDirection_Response_direction()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_GetDirection_Response_info direction(::custom_interfaces::srv::GetDirection_Response::_direction_type arg)
  {
    msg_.direction = std::move(arg);
    return Init_GetDirection_Response_info(msg_);
  }

private:
  ::custom_interfaces::srv::GetDirection_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::GetDirection_Response>()
{
  return custom_interfaces::srv::builder::Init_GetDirection_Response_direction();
}

}  // namespace custom_interfaces

#endif  // CUSTOM_INTERFACES__SRV__DETAIL__GET_DIRECTION__BUILDER_HPP_
