/*
 * filesystem_types.h
 *
 *  Created on: 31 Dec 2020
 *      Author: vitya
 */

#ifndef FILESYSTEM_TYPES_H_
#define FILESYSTEM_TYPES_H_

struct TMbrPtEntry
{
	uint8_t      status;
	uint8_t      first_h;
	uint8_t      first_s;
	uint8_t      first_c;
	uint8_t      ptype;
	uint8_t      last_h;
	uint8_t      last_s;
	uint8_t      last_c;

	uint32_t     first_lba;     // warning, misaligned for 32 bit read (exception on cortex-m0)
	uint32_t     sector_count;  // warning, misaligned for 32 bit read (exception on cortex-m0)
//
} __attribute__((packed));

struct TMasterBootRecord // aka MBR
{
	uint8_t      bootstrap_code[446];

	TMbrPtEntry  ptentry[4];  // warning: 16 byte / entry but misaligned

	uint16_t     signature;   // should be 0xAA55
//
} __attribute__((packed));


#endif /* FILESYSTEM_TYPES_H_ */
