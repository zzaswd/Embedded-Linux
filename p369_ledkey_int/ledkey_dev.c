#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>

#include <linux/interrupt.h>
#include <linux/irq.h>


#define CALL_DEV_NAME "ledkey_int"
#define CALL_DEV_MAJOR 240


#define DEBUG 0
#define IMX_GPIO_NR(bank, nr)       (((bank) - 1) * 32 + (nr))


static int sw_irq[8]={0};

static long sw_no = 0;

static int led[] = {
    IMX_GPIO_NR(1, 16),   //16
    IMX_GPIO_NR(1, 17),   //17
    IMX_GPIO_NR(1, 18),   //18
    IMX_GPIO_NR(1, 19),   //19
};

static int key[] = {
	IMX_GPIO_NR(1, 20),
	IMX_GPIO_NR(1, 21),
	IMX_GPIO_NR(4, 8),
	IMX_GPIO_NR(4, 9),
  	IMX_GPIO_NR(4, 5),
  	IMX_GPIO_NR(7, 13),
  	IMX_GPIO_NR(1, 7),
 	IMX_GPIO_NR(1, 8),
};

static int led_init(void)
{
    int ret = 0;
    int i;

    for (i = 0; i < ARRAY_SIZE(led); i++) {
        ret = gpio_request(led[i], "gpio led");
        if(ret<0){
            printk("#### FAILED Request gpio %d. error : %d \n", led[i], ret);
        }
    }
    return ret;
}

static void led_exit(void)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(led); i++){
        gpio_free(led[i]);
    }
}


static int key_init(void)
{
    int ret = 0;
    int i;

    for (i = 0; i < ARRAY_SIZE(key); i++) {
        ret = gpio_request(key[i], "gpio led");
        if(ret<0){
            printk("#### FAILED Request gpio %d. error : %d \n", key[i], ret);
        }
    }
    return ret;
}

irqreturn_t sw_isr(int irq,void* unuse){
	int i;
	for(i = 0; i < ARRAY_SIZE(key); i ++){
		if(irq == sw_irq[i]){
			sw_no = i + 1;
			break;
		}

	}
	printk("IRQ : %d, %ld\n",irq,sw_no);
	return IRQ_HANDLED;

}

static int key_irq_init(void){
	int ret;
	int i;
	char * irq_name[8] = {"irq sw1","irq sw2","irq sw3","irq sw4","irq sw5","irq sw6","irq sw7","irq sw8"};
	/*
	sw_irq[0] = gpio_to_irq(IMX_GPIO_NR(1,20));
	sw_irq[1] = gpio_to_irq(IMX_GPIO_NR(1,21));
	sw_irq[2] = gpio_to_irq(IMX_GPIO_NR(4,8));
	sw_irq[3] = gpio_to_irq(IMX_GPIO_NR(4,9));
	sw_irq[4] = gpio_to_irq(IMX_GPIO_NR(4,5));
	sw_irq[5] = gpio_to_irq(IMX_GPIO_NR(7,13));
	sw_irq[6] = gpio_to_irq(IMX_GPIO_NR(1,7));
	sw_irq[7] = gpio_to_irq(IMX_GPIO_NR(1,8));
	*/

	for(i=0; i<ARRAY_SIZE(key);i++){
		sw_irq[i] = gpio_to_irq(key[i]);
	}
	for(i=0; i<ARRAY_SIZE(key);i++){
		ret = request_irq(sw_irq[i],sw_isr,IRQF_TRIGGER_RISING,irq_name[i],NULL);
		if(ret <0) {
			printk("### FAILED Request irq %d. error : %d\n",sw_irq[i],ret);
		}
	}

	return ret;
}


static void key_exit(void)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(key); i++){
        gpio_free(key[i]);
    }
}

static void key_irq_exit(void){
	int i;
    for (i = 0; i < ARRAY_SIZE(key); i++){
		free_irq(sw_irq[i], NULL);
    }

}


void led_write(char data)
{
    int i;
    for(i = 0; i < ARRAY_SIZE(led); i++){
        gpio_direction_output(led[i], (data >> i ) & 0x01);
//      gpio_set_value(led[i], (data >> i ) & 0x01);
    }
#if DEBUG
//    printk("#### %s, data = %ld\n", __FUNCTION__, data);
#endif
}

void led_read(char * led_data)
{
    int i;
    unsigned long data=0;
    unsigned long temp;
    for(i=0;i<4;i++)
    {
        gpio_direction_input(led[i]); //error led all turn off
        temp = gpio_get_value(led[i]) << i;
        data |= temp;
    }
/*
    for(i=3;i>=0;i--)
    {
        gpio_direction_input(led[i]); //error led all turn off
        temp = gpio_get_value(led[i]);
        data |= temp;
        if(i==0)
            break;
        data <<= 1;  //data <<= 1;
    }
*/
#if DEBUG
//    printk("#### %s, data = %ld\n", __FUNCTION__, data);
#endif
    *led_data = (char)data;
    led_write(data);
    return;
}


#if 0
void key_read(char * key_data)
{
    int i;
    unsigned long data=0;
    unsigned long temp;
	for(i=0; i<8;i++){
		gpio_direction_input(key[i]);
		temp = gpio_get_value(key[i])<<i;
		data |= temp;	
	}

	for(i= 0 ;i <8 ; i++){
		if((data & (0x01<<i)) == (0x01<<i)){
			data = i+1;

			break;
		}

	}
/*
    for(i=0;i<8;i++)
    {
        gpio_direction_input(key[i]); //error led all turn off
        temp = gpio_get_value(key[i]) << i;
        data |= temp;
    }
	*/
/*
    for(i=3;i>=0;i--)
    {
        gpio_direction_input(led[i]); //error led all turn off
        temp = gpio_get_value(led[i]);
        data |= temp;
        if(i==0)
            break;
        data <<= 1;  //data <<= 1;
    }
*/
#if DEBUG
//    printk("#### %s, data = %ld\n", __FUNCTION__, data);
#endif
    *key_data = (char)data;
//	led_write(data);
    return;
}
#endif


static int call_open(struct inode *inode, struct file *file){
	int num = MINOR(inode->i_rdev);
	printk("call open -> minor : %d\n",num);
	num = MAJOR(inode->i_rdev);
	printk("call open -> major : %d\n",num);
	led_init();
	key_init();
	return 0;
}

static loff_t call_llseek(struct file *filp, loff_t off, int whence){
	printk("call llseek -> off : %08x, whence : %08x\n",(unsigned int)off,whence);
	return 0x23;
}

static ssize_t call_read(struct file *filp, char *buf, size_t count, loff_t *f_pos){
	//printk("call read -> buf : %08x, count : %08x \n", (unsigned int)buf,count);
	//led_read(buf);
	char kbuf;
//	key_read(&kbuf);
	kbuf = (char)sw_no;
//	put_user(kbuf,buf);
	copy_to_user(buf,&kbuf,count);
	sw_no = 0;
	/*  연속된 공간을 넘기는 과정
	char kbuf[10] = {0};
	for(int i = 0; i < 10 ; i ++){
		put_user(kbuf[i], buf+=i);
	}

	*/

	return count;
}

static ssize_t call_write(struct file *filp, const char *buf, size_t count,loff_t *f_pos){
	printk("call write -> buf : %08x, count : %08x \n",(unsigned int)buf,count);
	char kbuf;
	//get_user(kbuf,buf);
	copy_from_user(&kbuf,buf,count);
	led_write(kbuf);
	return count;

}

static long call_ioctl(struct file *filp,unsigned int cmd, unsigned long arg){
	printk("call ioctl -> cmd : %08x, arg:%08x\n",cmd,(unsigned int)arg);
	return 0x53;
}

static int call_release(struct inode * inode , struct file * filp){
	printk("call release\n");
    led_exit();
	led_write(0);
	key_exit();
	return 0;
}

struct file_operations call_fops=
{
	.owner = THIS_MODULE,
	.llseek = call_llseek,
	.read = call_read,
	.write = call_write,
	.unlocked_ioctl = call_ioctl,
	.open = call_open,
	.release = call_release,
};

static int call_init(void){
	int result ;
	printk("call call_init \n");
	result = register_chrdev(CALL_DEV_MAJOR, CALL_DEV_NAME, &call_fops);
	if(result <0) return result;

	led_init();
	//key_init();
	result = key_irq_init();

	return 0;
}

static void call_exit(void)
{
	printk("call call_exit \n");
	unregister_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME);
	led_exit();
	//key_exit();
	key_irq_exit();
}

module_init(call_init);
module_exit(call_exit);
MODULE_LICENSE("Dual BSD/GPL");
