
#ifndef __GDT_IDT_H__
#define __GDT_IDT_H__ 1

#include <stdint.h>

/*
 * -- The following structure is in the format that the processor will recognize as a GDT Entry
 *    -----------------------------------------------------------------------------------------
 */
typedef struct GDTEntry {
	uint16_t geLimitLow;
	uint16_t geBaseLow;
	uint8_t geBaseMiddle;
	uint8_t geAccess;
	uint8_t geGranularity;
	uint8_t geBaseHigh;
} __attribute__((packed)) GDTEntry;


/*
 * -- The following structure contains the locations and size of the GDT Table
 *    ------------------------------------------------------------------------
 */
typedef struct GDTPtr {
	uint16_t gpLimit;
	uint32_t gpBase;
} __attribute__((packed)) GDTPtr;

void InitGDT(void);
void FlushGDT(uint32 ptr);

#endif
