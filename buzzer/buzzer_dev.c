/******************************************
		201420871 소프트웨어학과
		김창희 buzzer_dev
		작성일: 2019.5.24
******************************************/

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#define GPIO24 24

#define DEV_NAME "buzzer_dev"
#define DEV_NUM 240

MODULE_LICENSE("GPL");


//buzzer_dev open funcion
int buzzer_open(struct inode *pinode, struct file *pfile){
	printk("[BUZZER] Open buzzer_dev\n");
	if(gpio_request(GPIO24,"GPIO24")!=0){		//Gpio 24번 핀 사용유무 판단
		printk("[BUZZER] Already being used");
		return -1;
	}
	gpio_direction_output(GPIO24,0);			//gpio 24번 핀에 신호를 주기위해 gpio_direction_output으로 설정
	return 0;
}

//buzzer_dev using gpio_pin free
int buzzer_release(struct inode *pinode, struct file *pfile){
	printk("[BUZZER] Close buzzer_dev\n");
	gpio_free(GPIO24);
	return 0;
}

//buzzer_dev에 신호를 주어 울리는 구간
void speak_buzzer(int mod){
	int i;

	if(mod==1){		//mod값이 1일 때 buzzer_dev단에 신호를 주는 곳
		for(i=0;i<10000;i++){	//10000번의 반	복문동안 신호를 주게 되어 부저 단의 신호 시간을 정할 수 있다
			gpio_set_value(GPIO24,1);	//gpio 24번 핀에 신호 1을 udelay 150만큼 준다.
			udelay(150);
			gpio_set_value(GPIO24,0);	//gpio 24번 핀에 신호 0을 udelay 150만큼 준다.
			udelay(150);				//1번의 반복문에서 1과 0 을 총 300u만큼 주고 이에 해당하는 진동수의 음이 나오게 된다.
		}
	}
}

//어플단에서 값을 읽어오는 함수
ssize_t buzzer_write(struct file *filp,const char * buf, size_t count, loff_t *f_pos){
	char get_msg;

	if(copy_from_user(&get_msg,buf,count)<0){	//어플단에서 값을 입려ㅏ면 받아오는 함수
		printk("[BUZZER] Write error\n");
		return -1;
	}

	printk("%c\n",get_msg);

	if(get_msg=='1'){							//'1'이 입려되었을 시에 부저를 울리도록 한다.
		speak_buzzer(1);
		printk("[BUZZER] Buzzer Sound\n");
	}
	return 0;
}

//fops 구조에 사용되는 함수들 
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = buzzer_open,
	.write = buzzer_write,
	.release = buzzer_release,
};

//buzzer_dev Major num값을 DEV_NUM값으로 지정하며 시작한다
int __init buzzer_init(void){
	printk("[BUZZER] Initialize buzzer_dev\n");
	printk("MAJOR: %d\n",register_chrdev(DEV_NUM,DEV_NAME,&fops));
	return 0;
}

//buzzer_dev Major num값 등록 해지하는 구간
void __exit buzzer_exit(void){
	printk("[BUZZER] Exit buzzer_dev\n");
	unregister_chrdev(DEV_NUM,DEV_NAME);
}

module_init(buzzer_init);	//모듈이 시작되는 곳
module_exit(buzzer_exit);	//모듈이 해제되는 곳
