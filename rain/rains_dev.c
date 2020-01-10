#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

//�������� ������ ��� ������ ������ 1�Ǵ� 0�� ����ϱ� ������ ����� GPIO�κ��� ���� �о�ͼ� �� ���� ���� �ȿ��� ���� �����ش�
#define A1 17

MODULE_LICENSE("GPL");

static int value = 0;

int raindrop_read(struct file *flip, char *buf, size_t count, loff_t *f_pos) { //GPIO�κ��� ���� ���� �������� ���� �Լ�
	
	gpio_direction_input(A1); // GPI017�� �Է��������� ����

	printk(KERN_INFO "rain detect");
	
	value = gpio_get_value(A1); //gpio_get_value()�� ���� GPIO17�κ��� ���� ������ value�� ����
	copy_to_user(buf, &value, sizeof(int)); // copy_to_user()�� Ŀ�� �ܿ��� app������ value���� ������
	return count;
}

static struct file_operations rain_fops = { // file operation ����Լ�, read�� �ʿ���
	.read = raindrop_read
};

int __init raindrop_init(void){ // ����̽� ���
	
	int a= register_chrdev(0, "rains_dev", &rain_fops); // ����� �� major number�� 0���� �ؼ� ����ϰ� �ִ� ����̽��� ��ġ�� �ʵ��� ��
	if(a<0){
		printk(KERN_INFO "Raindrop sensor init failed\n");
		}
	else{
		printk(KERN_INFO "Raindrop sensor init successful\n");
		printk(KERN_INFO "major number is %d\n",a); // �� ������ mknod�� �� �� 0���� �ϸ� ����� ����� ���� �ʾ� major number�� dmesg�� ���� Ȯ���� �� �� ��ȣ�� ����ϵ��� Ŀ�ο� ����ϰ� �Ͽ���
	}
	
	
	
	return 0;
}

void __exit raindrop_exit(void) { // ����� ����̽� ����
	gpio_free(A1); // GPIO ��� ����
	unregister_chrdev(0, "rains_dev");
	printk(KERN_INFO "Raindrop sensor exit\n");
}

module_init(raindrop_init);
module_exit(raindrop_exit);
