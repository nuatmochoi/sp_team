/* **************************************
	작성자: 최성우(201420913)
	작성일자 : 2019.05.31
 *************************************** */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define GPIO_MOTOR 25
#define MAJOR_NUMBER 0

MODULE_LICENSE("GPL");

//창문을 닫기 위한 SERVO MOTOR DEVICE DRIVER 
//구조와 기능은 기타 서보 모터와 동일
int motor_open(struct inode *pinode, struct file *pfile){
	printk("Motor device is opened\n");
	if(gpio_request(GPIO_MOTOR, "motor")!=0){
		printk("motor device is already used by other\n");
		return -1;
	}
	gpio_direction_output(GPIO_MOTOR,0);
	return 0;
}

int motor_close(struct inode *pinode, struct file *pfile){
	printk("Motor device is closed\n");
	gpio_free(GPIO_MOTOR);
	return 0;
}

void operation(int loc){ // 모터의 각도와 속도를 제어하기 위한 함수 
	int t=0;
	switch(loc){
		case 1:
			for(t=0;t<50;t++){
				gpio_set_value(GPIO_MOTOR,1);
				mdelay(2);
				udelay(300);
				gpio_set_value(GPIO_MOTOR,0);
				mdelay(17);
				udelay(700);
			}
			break;
		case 0:
			for(t=0;t<50;t++){
				gpio_set_value(GPIO_MOTOR,1);
				udelay(500);
				gpio_set_value(GPIO_MOTOR,0);
				mdelay(19);
				udelay(500);
			}
			break;
	}
}

ssize_t motor_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset){
	char from_app;
	
	if(copy_from_user(&from_app,buffer,length)<0){
		printk("ERROR for Writing\n");
		return -1;
	}
	
	if(from_app=='1'){
		operation(1);
		printk("turned till 180 degree\n");
	}
	else if(from_app=='0'){
		operation(0);
		printk("returned to initial status\n");
	}
	return 0;
}

struct file_operations fop={
	.owner = THIS_MODULE,
	.open = motor_open,
	.release = motor_close,
	.write = motor_write,
};

int __init motor_init(void){
	printk("motor device is initialized\n");
	printk("%d\n",register_chrdev(MAJOR_NUMBER,"motor_dev",&fop));
	return 0;
}

void __exit motor_exit(void){
	printk("motor device is ended\n");
	unregister_chrdev(MAJOR_NUMBER,"motor_dev");
}

module_init(motor_init);
module_exit(motor_exit);

