#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define GPIO18 18

MODULE_LICENSE("GPL");

//PWM �ֱ⸦ �̿��� �������� ���� ��� ���� ������ ��Ȳ���� '����ũ �Ա�'�� ���� ���� �� �ֵ��� ��

void turn_180(void){ // �������� 180���� ���� ���� �Լ�
    
    int i;
    for(i=0; i<50; i++){
			gpio_set_value(GPIO18,1); //����̽��� ����� GPIO�� ���� gpio_set_value�� ���� �� 1�� ����
			mdelay(2);
			udelay(300); // PWM �̷д�θ� 180���� ���� ���ؼ� 2ms ��ŭ High ���� �����ϵ��� delay�� ��� �ϴµ� 300us��ŭ �� ���� �� 180���� �Ǵ� ���� Ȯ���Ͽ���
			gpio_set_value(GPIO18,0);
			mdelay(17); // ������ 17ms + 700 us ��ŭ low ���� ������Ŵ
			udelay(700);
		}
		
}

void turn_0(void){ // �������� 0���� ���� ���� �Լ�
  
    int j;
    for(j=0; j<50; j++){
			gpio_set_value(GPIO18,1);
			udelay(500); // PWM �̷д�θ� 0���� ���� ���ؼ� high ���� 1ms ��ŭ High ���� �����ϵ��� delay�� ��� �ϴµ� �� ���������� ��� 500us��ŭ�� delay�� ������ �� 0���� �Ǵ� ���� Ȯ���Ͽ���
			gpio_set_value(GPIO18,0);
			mdelay(19); // ������ 19ms + 500 us ��ŭ low ���� ������Ŵ
			udelay(500);
		
		}
			
}

int servo1_open(struct inode *inode, struct file *filp){ //OPEN, GPIO�� ����������� ����, �ʱⰪ�� 0���� ����
	printk("servo1 motor device open function called\n");
	gpio_direction_output(GPIO18,0);
	return 0;
}

int servo1_release(struct inode *inode, struct file *filp){ //CLOSE, GPIO ��� ����
	printk("servo1 motor device release function called\n");
	gpio_free(GPIO18);
	return 0;
}

ssize_t servo1_write(struct file *filp, const char *buf, size_t length, loff_t *offset){               
  char magic_number; //magic_number��� ������ ���� app�����κ��� ���� �����ͼ� ������

  if(copy_from_user(&magic_number,buf,length)<0){ 
    printk("write error\n");
    return -1;
  }

    
  if(magic_number=='0')	turn_180(); // ���� app�ܿ��� ������ ���� ���� '0'�� ��� ���Ͱ� 180���� ���� �Լ��� ȣ����
  
  else if(magic_number=='1')	turn_0();	// ���� app�ܿ��� ������ ���� ���� '1'�� ��� ���Ͱ� 180���� ���� �Լ��� ȣ����
  
  return 0;
}

struct file_operations fops = { //GPIO�� ���� WRITE�ϴ� �͸� �ʿ�, READ�� ��� �ʿ���� ������ ������� ����
  .open = servo1_open,
  .write = servo1_write,
  .release = servo1_release,
};

int __init servo1_init(void){ // ����̽� ���
  int rc = register_chrdev(0, "servo1_dev", &fops);	// ����� �� major number�� 0���� �ؼ� ����ϰ� �ִ� ����̽��� ��ġ�� �ʵ��� ��
  if(rc < 0)
    printk("driver init failed\n");
  else{
    printk("driver init successful\n");
    printk("the major number is %d\n",rc);// �� ������ mknod�� �� �� 0���� �ϸ� ����� ����� ���� �ʾ� major number�� dmesg�� ���� Ȯ���� �� �� ��ȣ�� ����ϵ��� Ŀ�ο� ����ϰ� �Ͽ���
  }
  return 0;
}

void __exit servo1_exit(void){	// ����� ����̽� ����
  unregister_chrdev(0, "servo1_dev");
  printk("driver cleanup successful\n");
}

module_init(servo1_init);
module_exit(servo1_exit);
