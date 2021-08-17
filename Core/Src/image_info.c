/*
 * image_info.c
 *
 *  Created on: Aug 16, 2021
 *      Author: Janne Paalijarvi / Stacklake Ltd / http://stacklake.fi/index.en.html
 */


#include "image_info.h"
#include <stdint.h>

// Header area size = 64

// Offset 0
// Dynamic preamble (stack and reset handler addresses): 2 * 4 bytes

// Offset 0 + 8
const uint32_t gcau32ImageStartMagic[2] __attribute__ ((section (".image_header_body"))) = {0x461C0000, 0x12345678 };

// Offset 8 + 8 = 16
const uint8_t gcau8DeviceName[12] __attribute__ ((section (".image_header_body"))) =
        {'N', 'u', 'c', 'l', 'e', 'o', 'L', '4', '3', '2', 'K', 'C' };
// Offset 16 + 12 = 28
const uint8_t gcau8ImageVersion[8] __attribute__ ((section (".image_header_body"))) =
        {'v', '.', '1', '.', '2', '.', '7', 0};

// Offset 28 + 8 = 36
const uint8_t gcau8ImageDate[8] __attribute__ ((section (".image_header_body"))) =
        {'2', '0', '2', '1', '0', '8', '1', '7'};

// Offset 36 + 8 = 44
const uint32_t gcu32AfterHeaderDataLength __attribute__ ((section (".image_header_body"))) = 0;

// Offset 44 + 4 = 48
const uint32_t gcu32AfterHeaderDataCrcValid __attribute__ ((section (".image_header_body"))) = 0;

// Offset 48 + 4 = 52
const uint32_t gcu32AfterHeaderDataCrc32 __attribute__ ((section (".image_header_body"))) = 0;

// Offset 52 + 4 = 56
const uint32_t gcu32HeaderCrcValid __attribute__ ((section (".image_header_body"))) = 0;

// Offset 56 + 4 = 60
const uint32_t gcu32HeaderCrc32 __attribute__ ((section (".image_header_body"))) = 0;

// Offset 60 + 4 = 64
