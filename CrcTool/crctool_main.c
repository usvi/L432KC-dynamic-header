#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define HEADER_LENGTH 64

#define DYNAMIC_PREAMBLE1_OFFSET 0
#define DYNAMIC_PREAMBLE2_OFFSET 4

#define HEADER_MAGIC1 ((uint32_t)0x461C0000)
#define HEADER_MAGIC2 ((uint32_t)0x12345678)
#define HEADER_MAGIC1_OFFSET 8
#define HEADER_MAGIC2_OFFSET 12

#define DEVICE_NAME_OFFSET 16
#define DEVICE_NAME_LENGTH 12

#define IMAGE_VERSION_OFFSET 28
#define IMAGE_VERSION_LENGTH 8

#define IMAGE_DATE_OFFSET 36
#define IMAGE_DATE_LENGTH 8

#define AFTER_HEADER_DATA_LENGTH_OFFSET 44
#define AFTER_HEADER_DATA_LENGTH_LENGTH 4

#define AFTER_HEADER_DATA_CRC_VALID_OFFSET 48
#define AFTER_HEADER_DATA_CRC_VALID_LENGTH 4

#define AFTER_HEADER_DATA_CRC32_OFFSET 52
#define AFTER_HEADER_DATA_CRC32_LENGTH 4

#define HEADER_DATA_CRC_VALID_OFFSET 56
#define HEADER_DATA_CRC_VALID_LENGTH 4

#define HEADER_DATA_CRC32_OFFSET 60
#define HEADER_DATA_CRC32_LENGTH 4

#define TEMP_BUFFER_LENGTH 1024
#define FILENAME_OPERABLE_LENGTH 950

// Public domain crc32 implementation from
// http://home.thep.lu.se/~bjorn/crc/
uint32_t crc32_for_byte(uint32_t r)
{
  for(int j = 0; j < 8; ++j)
  {
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  }
  
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc)
{
  static uint32_t table[0x100];
  
  if(!*table)
  {
    for(size_t i = 0; i < 0x100; ++i)
    {
      table[i] = crc32_for_byte(i);
    }
  }
  for(size_t i = 0; i < n_bytes; ++i)
  {
    *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
  }
}





int main(int argc, char *argv[])
{
  uint8_t au8TempBuffer[TEMP_BUFFER_LENGTH] = { 0 };
  uint8_t au8HeaderBuffer[HEADER_LENGTH] = { 0 };
  uint8_t au8DeviceName[DEVICE_NAME_LENGTH + 1] = { 0 };
  uint8_t au8ImageVersion[IMAGE_VERSION_LENGTH + 1] = { 0 };
  uint8_t au8ImageDate[IMAGE_DATE_LENGTH + 1] = { 0 };
  char* psTempPointer = NULL;
  uint32_t u32Tmp1 = 0;
  uint32_t u32Tmp2 = 0;
  uint32_t u32AfterHeaderStartPos = 0;
  uint32_t u32AfterHeaderEndPos = 0;
  uint32_t u32AfterHeaderLength = 0;
  uint32_t u32AfterHeaderCrc32 = 0;
  uint32_t u32HeaderCrc32 = 0;
  FILE* pfOrigFile = NULL;
  FILE* pfNewFile = NULL;
  void* pvMalloc = NULL;
  
  
  if (argc == 1)
  {
    printf("No file given!");
    
    return 1;
  }
  printf("Opening file %s\n", argv[1]);
  pfOrigFile = fopen(argv[1], "rb");
  
  if (pfOrigFile == NULL)
  {
    printf("Failed opening %s\n", argv[1]);
    
    return 1;
  }
  if (fread(au8HeaderBuffer, 1, HEADER_LENGTH, pfOrigFile) != HEADER_LENGTH)
  {
    printf("Error reading header from %s\n", argv[1]);
    
    return 1;
  }
  printf("Found dynamic preamble: ");
  u32Tmp1 = *((uint32_t*)(au8HeaderBuffer + DYNAMIC_PREAMBLE1_OFFSET));
  u32Tmp2 = *((uint32_t*)(au8HeaderBuffer + DYNAMIC_PREAMBLE2_OFFSET));
  printf("0x%08" PRIX32, u32Tmp1);
  printf(" ");
  printf("0x%08" PRIX32, u32Tmp2);
  printf("\n");
  
  // Header magic check
  u32Tmp1 = *((uint32_t*)(au8HeaderBuffer + HEADER_MAGIC1_OFFSET));
  u32Tmp2 = *((uint32_t*)(au8HeaderBuffer + HEADER_MAGIC2_OFFSET));
  
  if ((u32Tmp1 == HEADER_MAGIC1) && (u32Tmp2 == HEADER_MAGIC2))
  {
    printf("Found MATCHING header magic values: " "0x%08" PRIX32 " " "0x%08" PRIX32 "\n", u32Tmp1, u32Tmp2);
  }
  else
  {
    printf("Error: Header magic values not mathed!: " "0x%08" PRIX32 " " "0x%08" PRIX32 "\n", u32Tmp1, u32Tmp2);
    fclose(pfOrigFile);
    
    return 1;
  }
  // Device name
  memcpy(au8DeviceName, au8HeaderBuffer + DEVICE_NAME_OFFSET, DEVICE_NAME_LENGTH);
  printf("Device name: %s\n", au8DeviceName);

  // Image version
  memcpy(au8ImageVersion, au8HeaderBuffer + IMAGE_VERSION_OFFSET, IMAGE_VERSION_LENGTH);
  printf("Image version: %s\n", au8ImageVersion);
  
  // Image date
  memcpy(au8ImageDate, au8HeaderBuffer + IMAGE_DATE_OFFSET, IMAGE_DATE_LENGTH);
  printf("Image date: %s\n", au8ImageDate);
  
  // Need to calculate first after header data size.
  u32AfterHeaderStartPos = ftell(pfOrigFile);
  fseek(pfOrigFile, 0, SEEK_END);
  u32AfterHeaderEndPos = ftell(pfOrigFile);
  fseek(pfOrigFile, u32AfterHeaderStartPos, SEEK_SET);
  u32AfterHeaderLength = u32AfterHeaderEndPos - u32AfterHeaderStartPos;
  printf("After header data length: " "%" PRIu32 "\n", u32AfterHeaderLength);
  
  // Store the length
  *((uint32_t*)(au8HeaderBuffer + AFTER_HEADER_DATA_LENGTH_OFFSET)) = u32AfterHeaderLength;
  
  
  // Next after header data crc
  // Actually read the whole file to memory
  
  pvMalloc = malloc(u32AfterHeaderLength);
  
  if (pvMalloc == NULL)
  {
    printf("Error: Malloc failed!\n");
    fclose(pfOrigFile);
    
    return 1;
  }
  u32Tmp1 = fread(pvMalloc, 1, u32AfterHeaderLength, pfOrigFile);
  
  // Stupid hack to force feof
  u32Tmp2 = fread(au8TempBuffer, 1, 1, pfOrigFile);

  
  // Some checks
  if ((u32Tmp1 != u32AfterHeaderLength) || (!feof(pfOrigFile)) || (u32Tmp2 != 0))
  {
    printf("Error: Reading after header data to memory failed!\n");
    printf("temp: " "%" PRIu32 " vs :" "%" PRIu32 "\n", u32Tmp1, u32AfterHeaderLength);
    
    free(pvMalloc);
    fclose(pfOrigFile);
    
    return 1;
  }
  // Calculate the after header data crc.
  u32AfterHeaderCrc32 = 0;
  crc32(pvMalloc, u32AfterHeaderLength, &u32AfterHeaderCrc32);
  
  // Store after header crc valid flag and the crc
  *((uint32_t*)(au8HeaderBuffer + AFTER_HEADER_DATA_CRC_VALID_OFFSET)) = (uint32_t)0x1;
  *((uint32_t*)(au8HeaderBuffer + AFTER_HEADER_DATA_CRC32_OFFSET)) = u32AfterHeaderCrc32;
  printf("Added after header data crc32: " "0x%08" PRIX32 "\n", u32AfterHeaderCrc32);


  // Put in header crc valid flag and crc
  *((uint32_t*)(au8HeaderBuffer + HEADER_DATA_CRC_VALID_OFFSET)) = (uint32_t)0x1;
  // Last thing, calculate header data crc
  // BUT don't include the last u32, it is the header checksum
  u32HeaderCrc32 = 0;
  crc32(au8HeaderBuffer, (HEADER_LENGTH - sizeof(u32HeaderCrc32)), &u32HeaderCrc32);
  // Slice it in
  *((uint32_t*)(au8HeaderBuffer + HEADER_DATA_CRC32_OFFSET)) = u32HeaderCrc32;
  printf("Added header data crc32: " "0x%08" PRIX32 "\n", u32HeaderCrc32);

  // Close reading file but keep stuff in memory for writing to new location
  fclose(pfOrigFile);
  
  // Start assembling new location
  memset(au8TempBuffer, 0, TEMP_BUFFER_LENGTH);
  strncpy((char*)au8TempBuffer, argv[1], TEMP_BUFFER_LENGTH);

  if (au8TempBuffer[TEMP_BUFFER_LENGTH - 1] != 0)
  {
    free(pvMalloc);
    
    au8TempBuffer[TEMP_BUFFER_LENGTH] = 0;
    printf("Error: Input filename %ssomething too long!\n", au8TempBuffer);

    return 1;
  }
  psTempPointer = strrchr((char*)au8TempBuffer, '.');
  
  if (psTempPointer == NULL)
  {
    free(pvMalloc);

    printf("Error: Malformed input file %s!\n", au8TempBuffer);
    
    return 1;
  }
  if ((psTempPointer - (char*)au8TempBuffer) > FILENAME_OPERABLE_LENGTH)
  {
    free(pvMalloc);
    
    printf("Error: Input filename %s too long for modifications!\n", au8TempBuffer);

    return 1;
  }
  // Ok to write new end replacing .bin
  sprintf(psTempPointer, "%s", ".with_crc32.bin");
  printf("Writing to %s\n", au8TempBuffer);
  
  pfNewFile = fopen((char*)au8TempBuffer, "wb");
  
  if (pfNewFile == NULL)
  {
    free(pvMalloc);
    
    printf("Error: Unable to open new file %s for writing!\n", au8TempBuffer);

    return 1;
  }
  u32Tmp1 = 0;
  u32Tmp2 = 0;
  
  u32Tmp1 = fwrite(au8HeaderBuffer, 1, HEADER_LENGTH, pfNewFile);
  u32Tmp2 = fwrite(pvMalloc, 1, u32AfterHeaderLength, pfNewFile);

  fclose(pfNewFile);
  free(pvMalloc);
  
  if ((u32Tmp1 != HEADER_LENGTH) || (u32Tmp2 != u32AfterHeaderLength))
  {
    printf("Error: Error in writing to the new file %s !\n", au8TempBuffer);
    
    return 1;
  }

  printf("Successfully wrote file with correct header to %s\n", au8TempBuffer);

  return 0;
}