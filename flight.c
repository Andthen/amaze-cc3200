/**
 ******************************************************************************
 * @file       flight.c
 * @author     Andthen
 * @brief      ��������Ŀ��غ���,���ļ��г��˷������������г���Ļ������̣��Լ���غ���
 * @version    V0.1
 *****************************************************************************/
main()
{
    //Ӳ����ʼ������
    Init()��
    //����������
    Unlock();
    //���߷���
    While(1)
      {
        Current_event = Get_event();//��ȡ��ǰ�¼�
        //���е�ָ���߶�
        if(Current_event == HEIGHT_HOLD_FLY)//���߷���
        {
          Path_control(Desired_Height, 0, 0, 0);
        }
//TODO  ��������ģʽ        

      }

}



/*************����·�߿���**************/
//���Ʒ���������ָ��·�߷��У��Ի�ͷΪǰ��
//h:���и߶�
//x:�����������ҷ��еľ���
//y:��������ǰ����еľ���
//
Path_control(h, x, y, yaw)
{
	Current_H = sonar_scan();
	Current_X = camrea_scan();
	Throttle = Pidupdate(Current_H, h);
	Roll          =  Pidupdate(Current_X, x);
	Pitch        = y;//����Ԥ����һ�����е��ٶ�
	Yaw          = yaw;//����Ԥ����Ϊ��
	Dire_control(throttle, roll, pitch, yaw );
	 

}




}

/********���з������*************/
//throttle:���ţ����Ʒ��и߶�
//���Ʒ�����������������
//roll:��������Ӧ������������ҷ���
//pitch:��������Ӧ����ǰ������е��ٶ�
//yaw:��ת�����ƻ�ͷ�ķ���
//���ϲ���ֱ�ӶԷ��������з����ϵĿ��ƣ���������ж�
Dire_control(int throttle, int roll, int pitch, int yaw )
{
	Pwm1 = throttle;
	Pwm2 = roll;
	Pwm3 = pitch;
	Pwm4 = yaw;
//TODO ��������MCU PWM��غ���
}




/*******����·�ߵ�PID����*******/
//��ĳһ�������е���
//current:��ǰֵ
//desired:����ֵ
//return :���ֵ
Pidupdate(int current, int desired)
{
//TODO ������Ҫ��дPID���ƺ���
  Return output;
}




/*******���������***********/
//�ú������ظ߶�ֵ
Sonar_scan()//���������
{
//TODO ��д��������ຯ��
Return height;���ظ߶�
}



/********����ͷɨ����򣬷���·��ƫ��**********/
//�Ժ���������ͷ�м�������Ϊƫ��Ϊ�㣬ƫ��Ϊ��ֵ��ƫ��Ϊ��ֵ
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

/*************��ⶨ����*************************/
height_line_scan()
{
 //TODO
  return Height_line;
}