#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>

asmlinkage int printklog_0(int pid, int a, int b, int c, int d)
{
	printk("[Project1] %d %10ld.%09ld %10ld.%09ld\n", pid, a, b, c, d);
    return 1;
}

