/*
 * PingPongOS - PingPong Operating System
 * Filename: ppos_bkl.h
 * Description: The big kernel lock.
 * This file is used to lock critical sections of the code.
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

/**
 * @brief Initializes the Big Kernel Lock
 */
void bkl_init();

/**
 * @brief Locks the Big Kernel Lock
 *
 * @return 1 if could be locked, and 0 otherwise.
 */
int bkl_lock();

/**
 * @brief Unlock the Big Kernel Lock
 *
 * @return 0 if could be unlocked, and 1 otherwise.
 */
int bkl_unlock();
