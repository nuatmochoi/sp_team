#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define GPIO18 18

MODULE_LICENSE("GPL");

//PWM 주기를 이용한 서보모터 각도 제어를 통해 적절한 상황에서 '물탱크 입구'를 열고 닫을 수 있도록 함

void turn_180(void){ // 서보모터 180도를 돌기 위한 함수
    
    int i;
    for(i=0; i<50; i++){
			gpio_set_value(GPIO18,1); //디바이스와 연결된 GPIO의 값을 gpio_set_value를 통해 값 1로 지정
			mdelay(2);
			udelay(300); // PWM 이론대로면 180도를 돌기 위해선 2ms 만큼 High 값을 유지하도록 delay를 줘야 하는데 300us만큼 더 줬을 때 180도가 되는 것을 확인하였음
			gpio_set_value(GPIO18,0);
			mdelay(17); // 나머지 17ms + 700 us 만큼 low 값을 유지시킴
			udelay(700);
		}
		
}

void turn_0(void){ // 서보모터 0도를 돌기 위한 함수
  
    int j;
    for(j=0; j<50; j++){
			gpio_set_value(GPIO18,1);
			udelay(500); // PWM 이론대로면 0도를 돌기 위해선 high 값이 1ms 만큼 High 값을 유지하도록 delay를 줘야 하는데 이 서보모터의 경우 500us만큼만 delay를 시켰을 때 0도가 되는 것을 확인하였음
			gpio_set_value(GPIO18,0);
			mdelay(19); // 나머지 19ms + 500 us 만큼 low 값을 유지시킴
			udelay(500);
		
		}
			
}

int servo1_open(struct inode *inode, struct file *filp){ //OPEN, GPIO를 출력전용으로 설정, 초기값을 0으로 지정
	printk("servo1 motor device open function called\n");
	gpio_direction_output(GPIO18,0);
	return 0;
}

int servo1_release(struct inode *inode, struct file *filp){ //CLOSE, GPIO 사용 해제
	printk("servo1 motor device release function called\n");
	gpio_free(GPIO18);
	return 0;
}

ssize_t servo1_write(struct file *filp, const char *buf, size_t length, loff_t *offset){               
  char magic_number; //magic_number라는 변수를 통해 app단으로부터 값을 가져와서 저장함

  if(copy_from_user(&magic_number,buf,length)<0){ 
    printk("write error\n");
    return -1;
  }

    
  if(magic_number=='0')	turn_180(); // 만약 app단에서 가져온 문자 값이 '0'일 경우 모터가 180도를 도는 함수를 호출함
  
  else if(magic_number=='1')	turn_0();	// 만약 app단에서 가져온 문자 값이 '1'일 경우 모터가 180도를 도는 함수를 호출함
  
  return 0;
}

struct file_operations fops = { //GPIO에 값을 WRITE하는 것만 필요, READ의 경우 필요없기 때문에 사용하지 않음
  .open = servo1_open,
  .write = servo1_write,
  .release = servo1_release,
};

int __init servo1_init(void){ // 디바이스 등록
  int rc = register_chrdev(0, "servo1_dev", &fops);	// 등록할 때 major number를 0으로 해서 사용하고 있는 디바이스가 겹치지 않도록 함
  if(rc < 0)
    printk("driver init failed\n");
  else{
    printk("driver init successful\n");
    printk("the major number is %d\n",rc);// 이 과정서 mknod를 할 때 0으로 하면 등록이 제대로 되지 않아 major number을 dmesg를 통해 확인한 뒤 그 번호를 등록하도록 커널에 출력하게 하였음
  }
  return 0;
}

void __exit servo1_exit(void){	// 등록한 디바이스 해제
  unregister_chrdev(0, "servo1_dev");
  printk("driver cleanup successful\n");
}

module_init(servo1_init);
module_exit(servo1_exit);
