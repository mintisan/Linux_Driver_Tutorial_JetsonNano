#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/ioctl.h>

#include <linux/hrtimer.h>
#include <linux/jiffies.h>


#define MYMAJOR 65

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("Kernel Module which sync kernel hrtimer to userspace using signal");


// https://blog.csdn.net/qq_21059825/article/details/118570280
// https://docs.kernel.org/timers/hrtimers.html


/* 1. insmod module*/
// sudo insmod userspace_hr_timer.ko
// dmesg | tail -n 10

/* 2. make character mod */
// rm /dev/userspace_hr_timer					# delete if had
// sudo mknod /dev/userspace_hr_timer c 65 0   	# check MYMAJOR firstly
// ls /dev/userspace_hr_timer -al				# check it
// sudo chmod 666 /dev/userspace_hr_timer  # change permission
// [ 4655.363860] start_t - now_t = 144932
// [ 4655.383884] start_t - now_t = 144952
// [ 4655.403908] start_t - now_t = 144972
// [ 4655.423930] start_t - now_t = 144992
// [ 4655.443956] start_t - now_t = 145012
// [ 4655.463984] start_t - now_t = 145032
// [ 4655.484006] start_t - now_t = 145052
// [ 4655.504052] start_t - now_t = 145072
// [ 4655.524086] start_t - now_t = 145092
// [ 4655.544108] start_t - now_t = 145112


/** variable contains pin number o interrupt controller to which GPIO 17 is mapped to */
unsigned int irq_number;

/** Global variables and defines for userspace app registration */
#define REGISTER_UAPP _IO('R', 'g')
static struct task_struct *task = NULL;

/* define for Signal sending */
#define SIGNR 44

/* hr timer */
static struct hrtimer  my_hrtimer;
u64 start_t;

static enum hrtimer_restart test_hrtimer_handler(struct hrtimer *timer) {
	/* Get current pid */
	struct siginfo info;
		if(task != NULL) {
		memset(&info, 0, sizeof(info));
		info.si_signo = SIGNR;
		info.si_code = SI_QUEUE;

		/* Send the signal */
		if(send_sig_info(SIGNR, (struct siginfo *) &info, task) < 0) {
			printk("hr_timer_callback: Error sending signal\n");
		}else{
			printk("hr_timer_callback: Success sync time\n");
		}
	}

	/* Get current time */
	u64 now_t = jiffies;
	printk("start_t - now_t = %u\n", jiffies_to_msecs(now_t - start_t));
	hrtimer_start(&my_hrtimer, ms_to_ktime(20), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int my_close(struct inode *device_file, struct file *instance) {
	if(task != NULL)
		task = NULL;
	return 0;
}

/**
 * @brief IOCTL for registering the Userspace app to the kernel module
 */
static long int my_ioctl(struct file *file, unsigned cmd, unsigned long arg) {
	if(cmd == REGISTER_UAPP) {
		task = get_current();
		printk("gpio_irq_signal: Userspace app with PID %d is registered\n", task->pid);
	}
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.release = my_close,
	.unlocked_ioctl = my_ioctl,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
	printk("Hello, Kernel!\n");

	/* Init of hrtimer */
	hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	my_hrtimer.function = &test_hrtimer_handler;
	start_t = jiffies;
	hrtimer_start(&my_hrtimer, ms_to_ktime(1000), HRTIMER_MODE_REL);

	if(register_chrdev(MYMAJOR, "userspace_hr_timer", &fops) < 0) {
		printk("Error!\n Can't register device Number!\n");
	}

	printk("Done!\n");
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
	hrtimer_cancel(&my_hrtimer);
	
	unregister_chrdev(MYMAJOR, "userspace_hr_timer");

	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);
