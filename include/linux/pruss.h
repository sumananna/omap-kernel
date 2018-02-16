/* SPDX-License-Identifier: GPL-2.0 */
/**
 * PRU-ICSS Subsystem user interfaces
 *
 * Copyright (C) 2015-2018 Texas Instruments Incorporated - http://www.ti.com
 *	Suman Anna <s-anna@ti.com>
 */

#ifndef __LINUX_PRUSS_H
#define __LINUX_PRUSS_H

#include <linux/remoteproc.h>

/**
 * enum pruss_pru_id - PRU core identifiers
 */
enum pruss_pru_id {
	PRUSS_PRU0 = 0,
	PRUSS_PRU1,
	PRUSS_NUM_PRUS,
};

/**
 * enum pru_ctable_idx - Configurable Constant table index identifiers
 */
enum pru_ctable_idx {
	PRU_C24 = 0,
	PRU_C25,
	PRU_C26,
	PRU_C27,
	PRU_C28,
	PRU_C29,
	PRU_C30,
	PRU_C31,
};

#if IS_ENABLED(CONFIG_PRUSS_REMOTEPROC)

int pruss_intc_trigger(unsigned int irq);

enum pruss_pru_id pru_rproc_get_id(struct rproc *rproc);
int pru_rproc_set_ctable(struct rproc *rproc, enum pru_ctable_idx c, u32 addr);

#else

static inline int pruss_intc_trigger(unsigned int irq)
{
	return -ENOTSUPP;
}

static inline enum pruss_pru_id pru_rproc_get_id(struct rproc *rproc)
{
	return -ENOTSUPP;
}

static inline int pru_rproc_set_ctable(struct rproc *rproc,
				       enum pru_ctable_idx c, u32 addr)
{
	return -ENOTSUPP;
}

#endif /* CONFIG_PRUSS_REMOTEPROC */

#endif /* __LINUX_PRUSS_H */
