//Wind pendulum
/**
 ******************************************************************************
 * @file       windpen.c
 * @author     Andthen
 * @brief      ��������Ŀ��غ���,���ļ��г��˷����ڳ���Ļ������̣��Լ���غ���
 * @version    V0.1
 *****************************************************************************/
//main()
//{
//  init();//��ʼ��
 //TODO ������Ŀд�������¼�
  
//}
/**********�������*******************/
//��ֱ�ӶԸ���������п��ƣ�f1~f4�ֱ��Ӧ�Ÿ������
//
fan_control(f1, f2, f3, f4)//
{
 //TODO д�����Ƹ������������
  pwm1 = f1;
  pwm2 = f2;
  pwm3 = f3;
  pwm4 = f4;
  
}
/**********���ڽǶȿ���***************/
//ʹ�������������ڶ�
//������dip ���봹�ߵļн�
//������rotation ���ԭ����ת�ĽǶ�
pendulum_control(dip, rotation)
{
//TODO �ҳ�dip rotation �� fan_control()���������Ĺ�ϵ
  f1 = ;
  f2 = ;
  f3 = ;
  f4 = ;
  fan_control(f1, f2, f3, f4);
}
/**************������̬����**************/
//ʹ����ά��һ����̬����
//desired_dip:�������
//desired_rotation:������ת��
pendulum_attitude_control(*desired_dip, *desired_rotation)
{
  attitude_get(*current_dip, *current_rotation);
  dip_out =  pidupdate(*current_dip, desired_dip);
  rotation_out = pidupdate();
  
  pendulum_control(dip_out, rotation_out);
}
/**************��ȡ��̬*****************/
//��ȡ���ڵĵ�ǰ��̬

attitude_get(*dip, *rotation)
{
  MPU6050_read();//���������ݶ�ȡ
  complementary_filter();//�����˲�
  dip = ;
  rotation = ;
}
/**************PID ����********************/
//current:��ǰֵ
//desired:����ֵ
pidupdate(current, desired)
{
//TODO ��������д��PID
  return output;
}