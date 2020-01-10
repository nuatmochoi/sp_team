/******************************************
201420927 소프트웨어학과 서지용 작성자
2019.05.31
******************************************/
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define GPIO27 27

MODULE_LICENSE("GPL");

// PWM 주기를 이용한 서보모터 각도 제어를 통해 적절한 상황에서 '유수관 입구'를 열고 닫을 수 있도록 함
// 구조와 기능은 servo1_dev와 동일함
void turn_180(void){
    
    int i;
    for(i=0; i<50; i++){
			gpio_set_value(GPIO27,1);
			mdelay(2);
			udelay(300);
			gpio_set_value(GPIO27,0);
			mdelay(17);
			udelay(700);
		}
		
}

void turn_0(void){
  
    int j;
    for(j=0; j<50; j++){
			gpio_set_value(GPIO27,1);
			udelay(500);
			gpio_set_value(GPIO27,0);
			mdelay(19);
			udelay(500);
		
		}
			
}

int servo3_open(struct inode *inode, struct file *filp){
	printk("servo3 motor device open function called\n");
	gpio_direction_output(GPIO27,0);
	return 0;
}

int servo3_release(struct inode *inode, struct file *filp){
	printk("servo3 motor device release function called\n");
	gpio_free(GPIO27);
	return 0;
}

ssize_t servo3_write(struct file *filp, const char *buf, size_t length, loff_t *offset){               
  char magic_number;

  if(copy_from_user(&magic_number,buf,length)<0){
    printk("write error\n");
    return -1;
  }

    
  if(magic_number=='0')	turn_180();
  
  else if(magic_number=='1')	turn_0();
  
  return 0;
}

struct file_operations fops = {
  .open = servo3_open,
  .write = servo3_write,
  .release = servo3_release,
};

int __init servo3_init(void){
  int rc = register_chrdev(0, "servo3_dev", &fops);
  if(rc < 0)
    printk("driver init failed\n");
  else{
    printk("driver init successful\n");
    printk("the major number is %d\n",rc);
  }
  return 0;
}

void __exit servo3_exit(void){
  unregister_chrdev(0, "servo3_dev");
  printk("driver cleanup successful\n");
}

module_init(servo3_init);
module_exit(servo3_exit);
