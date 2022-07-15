#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
	
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/gpio.h>

#define IMX_GPIO_NR(bank, nr)       (((bank) - 1) * 32 + (nr))
struct task_struct * kthread;
int d[6];

int fnd_sel[] = {
	IMX_GPIO_NR(4, 27),
	IMX_GPIO_NR(5, 13),
	IMX_GPIO_NR(5, 14),
	IMX_GPIO_NR(5, 15),
	IMX_GPIO_NR(5, 16),
	IMX_GPIO_NR(5, 17),
};

int fnd_data[] = {
	IMX_GPIO_NR(2, 0),
	IMX_GPIO_NR(2, 1),
	IMX_GPIO_NR(2, 2),
	IMX_GPIO_NR(2, 3),
	IMX_GPIO_NR(4, 28),
	IMX_GPIO_NR(4, 29),
	IMX_GPIO_NR(4, 30),
	IMX_GPIO_NR(4, 31),
};

char num[10] = {
	0x3f, 	//0
	0x06, 	//1
	0x5b, 	//2
	0x4f, 	//3
	0x66, 	//4
	0x6d, 	//5
	0x7d, 	//6
	0x27, 	//7
	0x7f, 	//8
	0x6f 	//9
};

static inline void fnd_pos(int pos, int enable){
	gpio_set_value(fnd_sel[pos], enable);
}

static inline void fnd_num(int n) {
	int i;

	for(i = 0; i < ARRAY_SIZE(fnd_data); ++i){
		gpio_set_value(fnd_data[i], (num[n] & (0x01 << i)) != 0);
	}
}

static int fnd_thread(void * p) {
	int i;

	do{
		for(i = 0; i < ARRAY_SIZE(fnd_sel); ++i){
			fnd_pos((i == 0) ? (ARRAY_SIZE(fnd_sel) - 1) : i - 1, 1);
			fnd_pos(i, 0);
			fnd_num(d[(ARRAY_SIZE(fnd_sel) - 1) - i]);
			udelay(2000); //0.002sec
		}
	}while(!kthread_should_stop());

	return 0;
}

static ssize_t drv_hw_fnd_write(struct file * file, const char * buf, size_t length, loff_t * ofs)
{
	int i;
	int ret;
	int data;
	char tmp[6 + 1];

	ret = copy_from_user(&data, buf, length);
	snprintf(tmp, sizeof(tmp), "%06d", data);

	for(i = 0; i < ARRAY_SIZE(fnd_sel); ++i){
		d[i] = tmp[(ARRAY_SIZE(fnd_sel) - 1) -i] - '0';
	}

	return 0;
}

static struct file_operations drv_hw_fnd_fops = 
{
	.owner = THIS_MODULE,
	.write = drv_hw_fnd_write,
};

static struct miscdevice drv_hw_fnd_driver = 
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "drv_hw_fnd",
	.fops = &drv_hw_fnd_fops,
};

static int drv_hw_fnd_init(void)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(fnd_sel); ++i) {
		ret = gpio_request(fnd_sel[i], "gpio fnd select");
		if(ret) {
			printk("#### FAILED Request gpio %d. error : %d \n", fnd_sel[i], ret);
			printk("Please execute the udoo_unexport_gpio module \n");
		} else {
			gpio_direction_output(fnd_sel[i], 1);
		}
	}

	for (i = 0; i < ARRAY_SIZE(fnd_data); ++i) {
		ret = gpio_request(fnd_data[i], "gpio fnd data");
		if(ret) {
			printk("#### FAILED Request gpio %d. error : %d \n", fnd_data[i], ret);
		} else {
			gpio_direction_output(fnd_data[i], 0);
		}
	}

	if(ret){
		return ret;
	}

	kthread = kthread_run(fnd_thread, 0, "fnd_thread");
	if (IS_ERR(kthread)) {
		printk("#### FAILED kthread_run\n");
		return PTR_ERR(kthread);
	}

	return misc_register(&drv_hw_fnd_driver);
}

static void drv_hw_fnd_exit(void)
{
	int i;

	kthread_stop(kthread);

	for (i = 0; i < ARRAY_SIZE(fnd_sel); ++i) {
		gpio_set_value(fnd_sel[i], 1);
		gpio_free(fnd_sel[i]);
	}

	for (i = 0; i < ARRAY_SIZE(fnd_data); ++i) {
		gpio_free(fnd_data[i]);
	}

	misc_deregister(&drv_hw_fnd_driver);
	
}

module_init(drv_hw_fnd_init);
module_exit(drv_hw_fnd_exit);

MODULE_AUTHOR("PlanX Studio");
MODULE_DESCRIPTION("drv_hw_fnd");
MODULE_LICENSE("Dual BSD/GPL");
