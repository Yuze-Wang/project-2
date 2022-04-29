#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "CacheCore.h"

CacheCore::CacheCore(uint32_t s, uint32_t a, uint32_t b, const char *pStr)
  : size(s)
  ,lineSize(b)
  ,assoc(a)
  ,numLines(s/b)
  ,numRows(s/b/a)
{
  if (strcasecmp(pStr, "RANDOM") == 0)
    policy = RANDOM;
  else if (strcasecmp(pStr, "LRU") == 0)
    policy = LRU;
  else {
    assert(0);
  }

  content = new CacheLine[numLines + 1];

  for(uint32_t i = 0; i < numLines; i++) {
    content[i].initialize();
  }
}

CacheCore::~CacheCore() {
  delete [] content;
}

CacheLine *CacheCore::accessLine(uint32_t addr)
{
  uint32_t offset_bit_len;
  uint32_t row_bit_len;
  uint32_t tag_bit_len;
  //uint32_t offset_bit;
  uint32_t row_bit;
  uint32_t tag_bit;
  offset_bit_len = log2i(lineSize);
  row_bit_len = log2i(numRows);
  tag_bit_len = 32 - offset_bit_len - row_bit_len;
  //offset_bit = addr << (32 - offset_bit_len) >> (32 - offset_bit_len);
  row_bit = addr << tag_bit_len >>(32 - row_bit_len);
  tag_bit = addr >> (32 - tag_bit_len);
  for(uint32_t i = 0; i < assoc; i++){
    if(content[numRows * i + row_bit].getTag() == tag_bit && content[numRows * i + row_bit].isValid()){
      for(uint32_t j = 0; j < assoc; j++){
        if(content[numRows * i + row_bit].isValid() && i != j) content[numRows * i + row_bit].incAge();
      }
      content[numRows + i * numLines / assoc].resetAge();
      return &content[numRows * i + row_bit];
    }
  }
  //printf("content size: %d", content.size());
  
  return NULL;
}

CacheLine *CacheCore::allocateLine(uint32_t addr, uint32_t *rplcAddr) {
  // TODO: Implement
  uint32_t offset_bit_len;
  uint32_t row_bit_len;
  uint32_t tag_bit_len;
  //uint32_t offset_bit;
  uint32_t row_bit;
  uint32_t tag_bit;
  int max_age;
  uint32_t max_age_index;
  offset_bit_len = log2i(lineSize);
  row_bit_len = log2i(numRows);
  tag_bit_len = 32 - offset_bit_len - row_bit_len;
  //offset_bit = addr << (32 - offset_bit_len) >> (32 - offset_bit_len);
  row_bit = (addr << tag_bit_len) >>(32 - row_bit_len);
  tag_bit = addr >> (32 - tag_bit_len);
  max_age = -1;
  //if(size == 16384){
    /*
    printf("size: %d\n", (int)size);
    printf("num of rows: %d\n", (int)numRows);
    printf("tag_bit_len: %d\n", (int)tag_bit_len);
    printf("row_bit_len: %d\n", (int)row_bit_len);
    printf("tag_bit: %x\n", (int)tag_bit);
    printf("row_bit: %x\n", (int)row_bit);
    */
  //}
  for(uint32_t i = 0; i <= assoc; i++){
    if(!content[numRows * i + row_bit].isValid()){
      //printf("index: %d\n", numRows * i + row_bit);
      content[numRows * i + row_bit].validate();
      content[numRows * i + row_bit].setTag(tag_bit);
      //printf("**************\n");
      return &content[numRows * i + row_bit];
    }
    if((int) content[numRows * i + row_bit].getAge() > max_age){
      max_age = content[numRows * i + row_bit].getAge();
      max_age_index = numRows * i + row_bit;
    }
  }
  
  if(content[max_age_index].isDirty()) rplcAddr = &addr;
  content[max_age_index].initialize();
  content[max_age_index].setTag(tag_bit);
  content[max_age_index].validate();
  return &content[max_age_index];

}
