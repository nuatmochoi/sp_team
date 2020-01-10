/******************************************
		201420871 ����Ʈ�����а�
		��â�� buzzer_dev
		�ۼ���: 2019.5.24
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
	if(gpio_request(GPIO24,"GPIO24")!=0){		//Gpio 24�� �� ������� �Ǵ�
		printk("[BUZZER] Already being used");
		return -1;
	}
	gpio_direction_output(GPIO24,0);			//gpio 24�� �ɿ� ��ȣ�� �ֱ����� gpio_direction_output���� ����
	return 0;
}

//buzzer_dev using gpio_pin free
int buzzer_release(struct inode *pinode, struct file *pfile){
	printk("[BUZZER] Close buzzer_dev\n");
	gpio_free(GPIO24);
	return 0;
}

//buzzer_dev�� ��ȣ�� �־� �︮�� ����
void speak_buzzer(int mod){
	int i;

	if(mod==1){		//mod���� 1�� �� buzzer_dev�ܿ� ��ȣ�� �ִ� ��
		for(i=0;i<10000;i++){	//10000���� ��	�������� ��ȣ�� �ְ� �Ǿ� ���� ���� ��ȣ �ð��� ���� �� �ִ�
			gpio_set_value(GPIO24,1);	//gpio 24�� �ɿ� ��ȣ 1�� udelay 150��ŭ �ش�.
			udelay(150);
			gpio_set_value(GPIO24,0);	//gpio 24�� �ɿ� ��ȣ 0�� udelay 150��ŭ �ش�.
			udelay(150);				//1���� �ݺ������� 1�� 0 �� �� 300u��ŭ �ְ� �̿� �ش��ϴ� �������� ���� ������ �ȴ�.
		}
	}
}

//���ôܿ��� ���� �о���� �Լ�
ssize_t buzzer_write(struct file *filp,const char * buf, size_t count, loff_t *f_pos){
	char get_msg;

	if(copy_from_user(&get_msg,buf,count)<0){	//���ôܿ��� ���� �Է����� �޾ƿ��� �Լ�
		printk("[BUZZER] Write error\n");
		return -1;
	}

	printk("%c\n",get_msg);

	if(get_msg=='1'){							//'1'�� �Է��Ǿ��� �ÿ� ������ �︮���� �Ѵ�.
		speak_buzzer(1);
		printk("[BUZZER] Buzzer Sound\n");
	}
	return 0;
}

//fops ������ ���Ǵ� �Լ��� 
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = buzzer_open,
	.write = buzzer_write,
	.release = buzzer_release,
};

//buzzer_dev Major num���� DEV_NUM������ �����ϸ� �����Ѵ�
int __init buzzer_init(void){
	printk("[BUZZER] Initialize buzzer_dev\n");
	printk("MAJOR: %d\n",register_chrdev(DEV_NUM,DEV_NAME,&fops));
	return 0;
}

//buzzer_dev Major num�� ��� �����ϴ� ����
void __exit buzzer_exit(void){
	printk("[BUZZER] Exit buzzer_dev\n");
	unregister_chrdev(DEV_NUM,DEV_NAME);
}

module_init(buzzer_init);	//����� ���۵Ǵ� ��
module_exit(buzzer_exit);	//����� �����Ǵ� ��
