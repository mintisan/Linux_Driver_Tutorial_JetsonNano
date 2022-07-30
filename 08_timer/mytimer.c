#include <linux/module.h>
#include <linux/init.h>
// #include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/timekeeping.h>

// https://blog.csdn.net/qq_26558047/article/details/115249587
// https://gist.github.com/itrobotics/596443150c01ff54658e
// https://www.kernel.org/doc/html/latest/core-api/timekeeping.html

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple test for LKM's timer");

/** variable for timer */
static struct timer_list my_timer;

static int times = 0;
static u64 last_ns = 0;
static u64 current_ns = 0;

static void timer_callback(unsigned long data) {
	times ++;
	printk("timer callback : %d\n", times);
	last_ns = current_ns;
	current_ns = ktime_get_ns();
	printk("current ns : %lld\n", current_ns);
	printk("current ns diff : %lld\n", current_ns - last_ns);
	// gpio_set_value(4, 0); /* Turn LED off */

	// modify the timer for next time
	mod_timer(&my_timer, jiffies + HZ / 2);
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
	printk("Hello, Kernel!\n");
	// /* GPIO 4 init */
	// if(gpio_request(4, "rpi-gpio-4")) {
	// 	printk("Can not allocate GPIO 4\n");
	// 	return -1;
	// }

	// /* Set GPIO 4 direction */
	// if(gpio_direction_output(4, 0)) {
	// 	printk("Can not set GPIO 4 to output!\n");
	// 	gpio_free(4);
	// 	return -1;
	// }

	/* Turn LED on */
	// gpio_set_value(4, 1);

	/* Initialize timer */
	// setup_timer(&my_timer, timer_callback, 0);

	init_timer(&my_timer);
	my_timer.expires = jiffies + HZ ;
    my_timer.data = (unsigned long) NULL;
    my_timer.function = timer_callback;
    /* ... */
    add_timer(&my_timer);

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
	printk("Goodbye, Kernel\n");
	// gpio_free(4);
	del_timer(&my_timer);
}

module_init(ModuleInit);
module_exit(ModuleExit);


