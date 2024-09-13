/*
 * PingPongOS - PingPong Operating System
 * Filename: queue.h
 * Description: Generic queue library to be used with the OS
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#ifndef __QUEUE__
#define __QUEUE__

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_ERR_NULL -1
#define Q_ERR_EMPTY -2
#define Q_ERR_ELEM_NULL -3
#define Q_ERR_ELEM_DUP -4
#define Q_ERR_ELEM_NOT_FOUND -5
#define Q_ERR_ELEM_DUP_LIST -6

/**
 * @brief Generic queue structure.
 */
typedef struct queue_t {
  struct queue_t *prev;
  struct queue_t *next;
} queue_t;

/**
 * @brief Count the total number of elements in the queue
 *
 * @param queue Pointer for the queue
 * @return The total number of elements in the queue
 */
int queue_size(queue_t *queue);

/**
 * @brief Walk the queue printing its content.
 *
 * Each element is printed by a function passed by the user.
 * The printing is going to have the following structure:
 * <name>: [print_elem(ptr)]
 *
 * @param name Name of the queue to be printend
 * @param queue Pointer for the queue that is going to be printed
 * @param print_elem Void pointer that defines the print function
 */
void queue_print(char *name, queue_t *queue, void print_elem(void *));

/**
 * @brief Inserts an element in the end of the queue.
 *
 * Before inserting the elements, some condition must be met:
 * - The queue must not be null
 * - The element must not be null
 * - The element must not be in the queue
 *
 * @param queue Pointer for the queue that is going to receive the element
 * @param elem The element that is going to be inserted into the queue
 *
 * @return 0 if it was successfuly inserted, <0 if something went wrong
 */
int queue_append(queue_t **queue, queue_t *elem);

/**
 * @brief Inserts an element in the queue in order.
 *
 * Before inserting the elements, some condition must be met:
 * - The queue must not be null
 * - The element must not be null
 * - The element must not be in the queue
 *
 * @param queue Pointer for the queue that is going to receive the element
 * @param elem The element that is going to be inserted into the queue
 * @param compare Function used to determine the insertion.
 *                 - 0> ptr1 should be placed after ptr2.
 *                 - 0< ptr1 should be placed before ptr2.
 *                 - 0  ptr1 and ptr2 are equal.
 *
 * @return 0 if it was successfuly inserted, <0 if something went wrong
 */
int queue_insert_inorder(queue_t **queue, queue_t *elem,
                         int (*compare)(const void *ptr1, const void *ptr2));

/**
 * @brief Removes the element in the queue.
 *
 * Before removing the elements, some condition must be met:
 * - The queue must not be null
 * - The queue must not be empty
 * - The element must not be null
 * - The element must be on the queue being passed
 *
 * @param queue
 * @param elem
 * @return 0 if it was successfuly removed, <0 if something went wrong
 */
int queue_remove(queue_t **queue, queue_t *elem);

#endif
