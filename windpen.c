//Wind pendulum
/**
 ******************************************************************************
 * @file       windpen.c
 * @author     Andthen
 * @brief      风力摆题目相关函数,该文件列出了风力摆程序的基本流程，以及相关函数
 * @version    V0.1
 *****************************************************************************/
//main()
//{
//  init();//初始化
 //TODO 根据题目写出各个事件
  
//}
/**********风机控制*******************/
//能直接对各个风机进行控制，f1~f4分别对应着各个风机
//
fan_control(f1, f2, f3, f4)//
{
 //TODO 写出控制各个风机的驱动
  pwm1 = f1;
  pwm2 = f2;
  pwm3 = f3;
  pwm4 = f4;
  
}
/**********单摆角度控制***************/
//使单摆向各个方向摆动
//参数：dip 摆与垂线的夹角
//参数：rotation 相对原点旋转的角度
pendulum_control(dip, rotation)
{
//TODO 找出dip rotation 与 fan_control()各个参数的关系
  f1 = ;
  f2 = ;
  f3 = ;
  f4 = ;
  fan_control(f1, f2, f3, f4);
}
/**************单摆姿态控制**************/
//使单摆维持一个姿态不变
//desired_dip:单摆倾角
//desired_rotation:单摆旋转角
pendulum_attitude_control(*desired_dip, *desired_rotation)
{
  attitude_get(*current_dip, *current_rotation);
  dip_out =  pidupdate(*current_dip, desired_dip);
  rotation_out = pidupdate();
  
  pendulum_control(dip_out, rotation_out);
}
/**************获取姿态*****************/
//获取单摆的当前姿态

attitude_get(*dip, *rotation)
{
  MPU6050_read();//传感器数据读取
  complementary_filter();//互补滤波
  dip = ;
  rotation = ;
}
/**************PID 计算********************/
//current:当前值
//desired:期望值
pidupdate(current, desired)
{
//TODO 根据需求写出PID
  return output;
}