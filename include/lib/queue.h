// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Version 1.4 -- Jan. 2022

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
 * @param queue
 * @param elem
 * @return 0 if it was successfuly inserted, <0 if something went wrong
 */
int queue_append(queue_t **queue, queue_t *elem);

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
