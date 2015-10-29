/**
 ******************************************************************************
 * @file       flight.c
 * @author     Andthen
 * @brief      飞行器题目相关函数,该文件列出了飞行器自主飞行程序的基本流程，以及相关函数
 * @version    V0.1
 *****************************************************************************/
main()
{
    //硬件初始化操作
    Init()；
    //飞行器解锁
    Unlock();
    //定高飞行
    While(1)
      {
        Current_event = Get_event();//获取当前事件
        //飞行到指定高度
        if(Current_event == HEIGHT_HOLD_FLY)//定高飞行
        {
          Path_control(Desired_Height, 0, 0, 0);
        }
//TODO  其他飞行模式        

      }

}



/*************飞行路线控制**************/
//控制飞行器按照指定路线飞行，以机头为前方
//h:飞行高度
//x:飞行器往左右飞行的距离
//y:飞行器往前后飞行的距离
//
Path_control(h, x, y, yaw)
{
	Current_H = sonar_scan();
	Current_X = camrea_scan();
	Throttle = Pidupdate(Current_H, h);
	Roll          =  Pidupdate(Current_X, x);
	Pitch        = y;//可以预定义一个飞行的速度
	Yaw          = yaw;//可以预定义为零
	Dire_control(throttle, roll, pitch, yaw );
	 

}




}

/********飞行方向控制*************/
//throttle:油门，控制飞行高度
//控制飞行器向各个方向飞行
//roll:翻滚，对应着往左或者往右飞行
//pitch:俯仰，对应着往前后方向飞行的速度
//yaw:旋转，控制机头的方向
//以上参数直接对飞行器进行方向上的控制，不做误差判断
Dire_control(int throttle, int roll, int pitch, int yaw )
{
	Pwm1 = throttle;
	Pwm2 = roll;
	Pwm3 = pitch;
	Pwm4 = yaw;
//TODO 连接瑞萨MCU PWM相关函数
}




/*******飞行路线的PID调节*******/
//对某一参量进行调节
//current:当前值
//desired:期望值
//return :输出值
Pidupdate(int current, int desired)
{
//TODO 根据需要编写PID控制函数
  Return output;
}




/*******超声波测距***********/
//该函数返回高度值
Sonar_scan()//超声波测距
{
//TODO 编写超声波测距函数
Return height;返回高度
}



/********摄像头扫描程序，返回路线偏差**********/
//以黑线在摄像头中间像素认为偏差为零，偏左为负值，偏右为正值
Camera_scan()
{
//TODO 
Return ErrorValue;
}

unlock()
{
  Dire_control(THROTTLE_ZERO, ROLL_RIGHT_MAX, PITCH_ZERO, YAW_ZERO )
  //delay for a while
  delay();
  return ;
}

/*************检测定高线*************************/
height_line_scan()
{
 //TODO
  return Height_line;
}