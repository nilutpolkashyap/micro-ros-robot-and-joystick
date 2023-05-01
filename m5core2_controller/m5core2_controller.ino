#include <micro_ros_arduino.h>
#include <M5Core2.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

//#include <std_msgs/msg/int32.h>
#include <geometry_msgs/msg/twist.h>

#if !defined(ESP32) && !defined(TARGET_PORTENTA_H7_M7) && !defined(ARDUINO_NANO_RP2040_CONNECT)
#error This example is only avaible for Arduino Portenta, Arduino Nano RP2040 Connect and ESP32 Dev module
#endif

rcl_publisher_t publisher;
//std_msgs__msg__Int32 msg;
geometry_msgs__msg__Twist msg;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;
rclc_executor_t executor;

#define LED_PIN 13

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

Button front(110, 0, 100, 75, false,  "FRONT",{GREEN, WHITE, WHITE});
Button stop(110, 80, 100, 75 , false,  "STOP",{RED, WHITE, WHITE});
Button back(110, 165, 100, 75, false,  "BACK",{GREEN, WHITE, WHITE});
Button left(5, 80, 95, 75, false,  "LEFT",{BLUE, WHITE, WHITE});
Button right(215, 80, 95, 75, false,  "RIGHT",{BLUE, WHITE, WHITE});

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    M5.update();
    
    int status = 0;
    msg.linear.x = 0.0;
    msg.angular.z = 0.0;
    if (front.isPressed()){
      msg.linear.x = 0.50;
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
      status = 1;
    }
    if (back.isPressed()){
      msg.linear.x = -0.50;
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
      status = 1;
    }
    if (stop.isPressed()){
      msg.linear.x = 0.0;
      msg.angular.z = 0.0;
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
      status = 1;
    }
    if (left.isPressed()){
      msg.angular.z = 1.0;
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
      status = 1;
    }
    if (right.isPressed()){
      msg.angular.z = -1.0;
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
      status = 1;
    }
    if (status == 0){
      RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    }
  }
}

void setup() {
  M5.begin();
  
  set_microros_wifi_transports("YOUR_SSID", "YOUR_PASSWORD", "192.168.29.80", 8889);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "m5stack_controller_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel"));

   // create timer,
  const unsigned int timer_timeout = 500;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

   // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

//  msg.data = 0;
}

void loop() {
//    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
//    msg.data++;
  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
