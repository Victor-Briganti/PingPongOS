/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos_bkl.c
 * Description: The big kernel lock.
 * This file is used to lock critical sections of the code.
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#include "ppos.h"

static mutex_t bigKernelLock;

void bkl_init() { mutex_init(&bigKernelLock); }

int bkl_lock() { return mutex_lock(&bigKernelLock); }

int bkl_unlock() { return mutex_unlock(&bigKernelLock); }