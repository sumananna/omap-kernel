/*
 * OMAP hardware spinlock test driver
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Contact: Suman Anna <s-anna@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <linux/hwspinlock.h>

/* load-time options */
static int count = 2;
module_param(count, int, 0);

static int hwspin_lock_test(struct hwspinlock *hwlock)
{
	int i;
	int ret;

	pr_err("\nTesting lock %d\n", hwspin_lock_get_id(hwlock));
	for (i = 0; i < count; i++) {
		ret = hwspin_trylock(hwlock);
		if (ret) {
			pr_err("%s: Initial lock failed\n", __func__);
			return -EFAULT;
		}
		pr_err("trylock #1 status[%d] = %d\n", i, ret);

		/* Verify lock actually works - re-acquiring it should fail */
		ret = hwspin_trylock(hwlock);
		pr_err("trylock #2 status[%d] = %d\n", i, ret);
		if (!ret) {
			/* Keep locks balanced even in failure cases */
			hwspin_unlock(hwlock);
			hwspin_unlock(hwlock);
			pr_err("%s: Recursive lock succeeded unexpectedly\n",
			       __func__);
			return -EFAULT;
		}

		/* Verify unlock by re-acquiring the lock after releasing it */
		hwspin_unlock(hwlock);
		ret = hwspin_trylock(hwlock);
		pr_err("trylock after unlock status[%d] = %d\n", i, ret);
		if (ret) {
			pr_err("%s: Unlock failed\n", __func__);
			return -EINVAL;
		}

		hwspin_unlock(hwlock);
	}

	return 0;
}

/* forward declaration */
static const struct of_device_id omap_hwspinlock_test_of_match[];

static int hwspin_lock_test_all_locks(struct platform_device *pdev)
{
	int i;
	int ret = 0, ret1 = 0;
	int max_locks;
	struct hwspinlock *hwlock = NULL;

	max_locks = (u32)of_match_device(omap_hwspinlock_test_of_match,
					 &pdev->dev)->data;
	for (i = 0; i < max_locks; i++) {
		hwlock = hwspin_lock_request_specific(i);
		if (!hwlock) {
			pr_err("request lock %d failed\n", i);
			ret = -EIO;
			continue;
		}

		ret1 = hwspin_lock_test(hwlock);
		if (ret1) {
			pr_err("hwspinlock tests failed on lock %d\n", i);
			ret = ret1;
			goto free_lock;
		}

free_lock:
		ret1 = hwspin_lock_free(hwlock);
		if (ret1) {
			pr_err("hwspin_lock_free failed on lock %d\n", i);
			ret = ret1;
		}
	}

	return ret;
}

static int hwspin_lock_test_all_phandle_locks(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct hwspinlock *hwlock = NULL;
	int ret = 0, ret1 = 0;
	int max_locks;
	int num_locks;
	int i, hwlock_id;

	max_locks = (u32)of_match_device(omap_hwspinlock_test_of_match,
					 &pdev->dev)->data;
	num_locks = of_count_phandle_with_args(np, "hwlocks", "#hwlock-cells");
	pr_err("Number of phandles = %d\n", num_locks);

	for (i = 0; i < num_locks; i++) {
		hwlock_id = of_hwspin_lock_get_id(np, i);
		if (hwlock_id < 0) {
			pr_err("unable to get hwlock_id : %d\n", hwlock_id);
			ret = -EINVAL;
			continue;
		};

		hwlock = hwspin_lock_request_specific(hwlock_id);
		if (!hwlock) {
			pr_err("unable to get hwlock\n");
			ret = -EINVAL;
			continue;
		}

		ret1 = hwspin_lock_test(hwlock);
		if (ret1) {
			pr_err("hwspinlock test failed on DT lock %d, ret = %d\n",
				hwspin_lock_get_id(hwlock), ret1);
			ret = ret1;
		}

		ret1 = hwspin_lock_free(hwlock);
		if (ret1) {
			pr_err("hwspin_lock_free failed on lock %d\n",
				hwspin_lock_get_id(hwlock));
			ret = ret1;
		}
	}

	return ret;
}

static int omap_hwspinlock_test_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	int ret = 0;

	if (!np) {
		pr_err("invalid node pointer\n");
		return -EINVAL;
	}

	pr_err("\n***** Begin - Test All pHandle Locks ****\n");
	ret = hwspin_lock_test_all_phandle_locks(pdev);
	if (ret)
		pr_err("hwspin_lock_test_all_locks failed, ret = %d\n", ret);
	pr_err("\n***** End - Test All pHandle Locks ****\n");

	pr_err("\n***** Begin - Test All Locks ****\n");
	ret = hwspin_lock_test_all_locks(pdev);
	if (ret)
		pr_err("hwspin_lock_test_all_locks failed, ret = %d\n", ret);
	pr_err("\n***** End - Test All Locks ****\n");

	return 0;
}

static int omap_hwspinlock_test_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id omap_hwspinlock_test_of_match[] = {
	{ .compatible = "ti,omap4-hwspinlock-test",  .data = (void *) 32, },
	{ .compatible = "ti,omap5-hwspinlock-test",  .data = (void *) 32, },
	{ .compatible = "ti,dra7-hwspinlock-test",   .data = (void *) 256, },
	{ .compatible = "ti,am33xx-hwspinlock-test", .data = (void *) 128, },
	{ .compatible = "ti,am43xx-hwspinlock-test", .data = (void *) 128, },
	{ /* end */ },
};
MODULE_DEVICE_TABLE(of, omap_hwspinlock_test_of_match);

static struct platform_driver omap_hwspinlock_test_driver = {
	.probe		= omap_hwspinlock_test_probe,
	.remove		= omap_hwspinlock_test_remove,
	.driver		= {
		.name	= "omap_hwspinlock_test",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(omap_hwspinlock_test_of_match),
	},
};

static int __init omap_hwspinlock_test_init(void)
{
	return platform_driver_register(&omap_hwspinlock_test_driver);
}
module_init(omap_hwspinlock_test_init);

static void __exit omap_hwspinlock_test_exit(void)
{
	platform_driver_unregister(&omap_hwspinlock_test_driver);
}
module_exit(omap_hwspinlock_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware spinlock Test driver for OMAP");
MODULE_AUTHOR("Suman Anna <s-anna@ti.com>");
