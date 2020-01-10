#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

//빗물감지 센서의 경우 디지털 값으로 1또는 0을 출력하기 때문에 연결된 GPIO로부터 값을 읽어와서 비가 오는 경우와 안오는 경우로 나눠준다
#define A1 17

MODULE_LICENSE("GPL");

static int value = 0;

int raindrop_read(struct file *flip, char *buf, size_t count, loff_t *f_pos) { //GPIO로부터 센서 값을 가져오기 위한 함수
	
	gpio_direction_input(A1); // GPI017을 입력전용으로 설정

	printk(KERN_INFO "rain detect");
	
	value = gpio_get_value(A1); //gpio_get_value()를 통해 GPIO17로부터 값을 가져와 value에 저장
	copy_to_user(buf, &value, sizeof(int)); // copy_to_user()로 커널 단에서 app단으로 value값을 보내줌
	return count;
}

static struct file_operations rain_fops = { // file operation 사용함수, read만 필요함
	.read = raindrop_read
};

int __init raindrop_init(void){ // 디바이스 등록
	
	int a= register_chrdev(0, "rains_dev", &rain_fops); // 등록할 때 major number를 0으로 해서 사용하고 있는 디바이스가 겹치지 않도록 함
	if(a<0){
		printk(KERN_INFO "Raindrop sensor init failed\n");
		}
	else{
		printk(KERN_INFO "Raindrop sensor init successful\n");
		printk(KERN_INFO "major number is %d\n",a); // 이 과정서 mknod를 할 때 0으로 하면 등록이 제대로 되지 않아 major number을 dmesg를 통해 확인한 뒤 그 번호를 등록하도록 커널에 출력하게 하였음
	}
	
	
	
	return 0;
}

void __exit raindrop_exit(void) { // 등록한 디바이스 해제
	gpio_free(A1); // GPIO 사용 해제
	unregister_chrdev(0, "rains_dev");
	printk(KERN_INFO "Raindrop sensor exit\n");
}

module_init(raindrop_init);
module_exit(raindrop_exit);
