#pragma once

#include <stdatomic.h>
#include <stdbool.h>

// leaving this from the old version lmao
typedef enum {
	LASR_VOID = 0,
	LASR_BOOL = 'b',
	LASR_INT = 'i',
	LASR_DOUBLE = 'd',
	LASR_STRING = 's', // Harder, save for last lmao
} lasr_type;

/**
 * Borrowed data base type. Owned by READing thread.
 *
 * (todo docucomment)
 */
typedef struct _owned_data owned_data;
struct _owned_data {
    char * key; /* consider `name` ?? not sure which is more intuitive */
	owned_data * next; /* linked list next */
	atomic_bool borrowed; /* writing thread is using this */
	// some enum: data type
	size_t len; /* length of data buffer (struct extends) */
	void data[];
};
