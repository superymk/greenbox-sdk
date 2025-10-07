// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <asm/tlbflush.h>

#include <sa_headers.h>

static struct class *driver_class;
static dev_t tz_client_device_number;
static struct cdev tz_client_cdev;

static long tz_client_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
    OSSTATUS status = EINVAL;
	int copy_retval = 1;
    void *argp = (void __user *) arg;
    struct tz_cmd_packet *cmd_packet = NULL;

    // Check: <argp> must be a valid pointer
    if(!argp)
        return EINVAL;

    cmd_packet = (struct tz_cmd_packet*) os->mm_malloc_zeroed(sizeof(struct tz_cmd_packet));
    if(!cmd_packet){
    	DRV_ERR("<cmd_packet> allocation error!\n");
        status = E_NO_MEM;
        goto finish;
    }

    status = os->copy_from_user(cmd_packet, (void*) argp, sizeof(struct tz_cmd_packet)); 
	if (SA_FAIL(status)) {
    	DRV_ERR("copy_from_user error!\n");
		status = E_INTERNAL_ERROR;
		goto finish;
	}
    
    // [TODO] Remove <copy_retval> and we will force copy results back to user, because they contain ops' result status.
	status = ioctl_request_handler(cmd_packet, &copy_retval);
    cmd_packet->status = status;

    // Copying the result back to the User Space
	status = os->copy_to_user((void*) argp, cmd_packet, sizeof(struct tz_cmd_packet)); 
    if (SA_FAIL(status)) {
        DRV_ERR("copy_to_user error!\n");
        status = E_INTERNAL_ERROR;
        goto finish;
    }

finish:
	if(cmd_packet)
    	os->mm_free(cmd_packet);
    return status;
}

static int tz_client_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int tz_client_release(struct inode *inode, struct file *file)
{
    return 0;
}

 static const struct file_operations tz_client_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = tz_client_ioctl,
    .open = tz_client_open,
    .release = tz_client_release
 };

static int tz_client_init(void)
{
    int ret_code = 0;
    struct device *class_dev;
	OSSTATUS status = OS_SUCCESS;
    
    ret_code = alloc_chrdev_region(&tz_client_device_number, 0, 1,
                                                    TZ_CLIENT_DEV);
    if (ret_code < 0) {
        goto alloc_chrdev_region_err;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    driver_class = class_create(TZ_CLIENT_DEV);
#else
    driver_class = class_create(THIS_MODULE, TZ_CLIENT_DEV);
#endif
    if (IS_ERR(driver_class)) {
        ret_code = ENOMEM;
        goto unregister_chrdev_region;
    }

    class_dev = device_create(driver_class, NULL, tz_client_device_number, NULL,
            TZ_CLIENT_DEV);
    if (!class_dev) {
        ret_code = ENOMEM;
        goto class_destroy;
    }

    cdev_init(&tz_client_cdev, &tz_client_fops);
    tz_client_cdev.owner = THIS_MODULE;

    ret_code = cdev_add(&tz_client_cdev,
                        MKDEV(MAJOR(tz_client_device_number), 0), 1);
    if (ret_code < 0) {
        goto class_device_destroy;
    }

	// Do not use any <os->*> functions in <tz_client_init>, because these function pointers are un-initialized yet.

	status = secagent_init();
	if(SA_FAIL(status))
	{
		ret_code = -1;
		goto class_device_destroy;
	}
	
	return 0;

class_device_destroy:
    device_destroy(driver_class, tz_client_device_number);
class_destroy:
    class_destroy(driver_class);
unregister_chrdev_region:
    unregister_chrdev_region(tz_client_device_number, 1);
alloc_chrdev_region_err:
    secagent_mark_not_running();
    return ret_code;
}

static void tz_client_exit(void)
{
	secagent_fini();
	device_destroy(driver_class, tz_client_device_number); 
	class_destroy(driver_class);
	unregister_chrdev_region(tz_client_device_number, 1);
}

module_init(tz_client_init);
module_exit(tz_client_exit);

MODULE_DESCRIPTION("SecAgent Kernel Module");
MODULE_AUTHOR("Miao Yu <superymk@cmu.edu>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("secagent");