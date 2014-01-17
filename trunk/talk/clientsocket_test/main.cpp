#include <iostream>
#include "talk/base/basictypes.h"
#include <string>
#include <sstream>
#include <string.h>

const char BASE[] ={'0','1','2','3','4','5','6',
  '7','8','9','a','b','c','d','e','f','g','h',
  'i','j','k','l','m','n','o','p','q','r','s',
  't','u','v','w','x','y','z','A','B','C','D',
  'E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z'};
const int BASE_LENGTH       = 62;
const int BASE_16_LENGTH    = 16;
int HEX_BASE[256]           = {0};
int BASE62[256]             = {0};
const size_t MAX_BASE_EQUIP = 11;
const size_t MAX_BASE_IDENT = 6;
const uint32 SCUR           =  0X12345678;

void InitHEX_BASE(){
  HEX_BASE['0'] = 0;
  HEX_BASE['1'] = 1;
  HEX_BASE['2'] = 2;
  HEX_BASE['3'] = 3;
  HEX_BASE['4'] = 4;
  HEX_BASE['5'] = 5;
  HEX_BASE['6'] = 6;
  HEX_BASE['7'] = 7;
  HEX_BASE['8'] = 8;
  HEX_BASE['9'] = 9;
  HEX_BASE['A'] = 10;
  HEX_BASE['B'] = 11;
  HEX_BASE['C'] = 12;
  HEX_BASE['D'] = 13;
  HEX_BASE['E'] = 14;
  HEX_BASE['F'] = 15;
  HEX_BASE['a'] = 10;
  HEX_BASE['b'] = 11;
  HEX_BASE['c'] = 12;
  HEX_BASE['d'] = 13;
  HEX_BASE['e'] = 14;
  HEX_BASE['f'] = 15;

  for(int i = 0; i <= 62; i++)
    BASE62[BASE[i]] = i;
}

bool ConverIdeWithSginalString(const char *origanl_string,
                               char *new_string)
{
  //1. get the '-'
  int totoal_length = strlen(origanl_string);
  const char *p = strchr(origanl_string,'-');
  int len = p - origanl_string;
  if(p == NULL || totoal_length != 17)
    return false;
  memcpy(new_string,origanl_string,len);
  memcpy(new_string + len, origanl_string + len + 1, totoal_length - len);
  return true;
}

uint64 Base16ToDecimal(const char *hec_string)
{
  uint64 res = 0;
  for(int i = strlen(hec_string) - 1, j = 0;
    i >= 0; i--, j++)
  {
    res += HEX_BASE[hec_string[i]] * (uint64)pow(16,j);
  }
  return res;
}

uint64 Base62ToDecimal(const char * base_62_string){
  uint64 res = 0;
  for(int i = strlen(base_62_string) - 1, j = 0;
    i >= 0; i--, j++)
  {
    uint64 s = base_62_string[i];
    uint64 a = BASE62[base_62_string[i]];
    uint64 t = (uint64)pow(BASE_LENGTH,j);
    res += a * t;
  }
  return res;
}

void DecimalToBase62(uint64 dec_num,
                        char *result){
  int  k = 0;
  while (dec_num)
  {
    uint64 t = dec_num%BASE_LENGTH;
    result[k++] = BASE[t];
    dec_num /= BASE_LENGTH;
  }
  k --;
  for(int i = 0; k > i; k--,i++){
    char t = result[k];
    result[k] = result[i];
    result[i] = t;
  }
}

void DecimalToBase16(uint64 dec_num,char *result){
  int  k = 0;
  while (dec_num)
  {
    uint64 t = dec_num%BASE_16_LENGTH;
    result[k++] = BASE[t];
    dec_num /= BASE_16_LENGTH;
  }
  k --;
  for(int i = 0; k > i; k--,i++){
    char t = result[k];
    result[k] = result[i];
    result[i] = t;
  }
}

uint64 GenaratorIDE(uint64 dec_num, uint32 scur){
  uint32 l = dec_num & scur;
  uint32 r = (dec_num >> 32) & l;
  return r;
}

void MergeTwoString(const char *a, const char *b, char *result){
  memset(result,'0',MAX_BASE_EQUIP + MAX_BASE_IDENT);
  size_t length_a   = strlen(a);
  size_t front_a    = 0;
  size_t length_b   = strlen(b);
  size_t front_b    = 0;

  if(length_a < MAX_BASE_EQUIP)
    front_a = MAX_BASE_EQUIP - length_a;
  memcpy(result + front_a,a,length_a);

  if(length_b < MAX_BASE_IDENT)
    front_b = MAX_BASE_IDENT - length_b;
  memcpy(result + MAX_BASE_EQUIP + front_b, b,length_b);
}

const char *GenaratorSecrecy(const char * equip_ide,
                             char *final_result){
  
  InitHEX_BASE();
  //std::string equip_ide     = "FFFFFFFF-FFFFFFFF";
  char equip_hex_string[64] = {0};

  uint64 equip_decimal      =  0;
  char equip_62_base[64]    = {0};

  uint64 equip_identifer    =  0;
  char quip_ide_62_base[64] = {0};


  std::cout << "the equipment identifier is " << equip_ide << std::endl;
  std::cout << "1. const equipment identifier to pure hex string"
   << std::endl;
  ConverIdeWithSginalString(equip_ide,equip_hex_string);
  std::cout << "\t " << equip_hex_string << std::endl;
  
  std::cout << "2. convert the hex string to Decimal"
    << std::endl;
  equip_decimal = Base16ToDecimal(equip_hex_string);
  std::cout << "\t " << equip_decimal << std::endl;
  
  std::cout << "3. decimal number to 62 Base number" << std::endl;
  DecimalToBase62(equip_decimal,equip_62_base);
   std::cout << "\t " << equip_62_base << std::endl;

   std::cout << "4. generator a identifier by this base 62 number"
     << std::endl;
  equip_identifer = GenaratorIDE(equip_decimal,SCUR);
   std::cout << "\t " << equip_identifer << std::endl;

   std::cout << "5. convert the identifier number to 62 base number"
     << std::endl;
  DecimalToBase62(equip_identifer,quip_ide_62_base);
  std::cout << "\t " << quip_ide_62_base << std::endl;

   std::cout << "6. merge equipment identifier string"
    << std::endl;
  MergeTwoString(equip_62_base,quip_ide_62_base,final_result);
  std::cout << "\t " << final_result << std::endl;

  return final_result;
}

void SeparateEquipGettingString(const char *getting_string,
                                char *equip_62_base_string,
                                char *equip_62_base_ide)
{
  size_t equip_start = 0;
  size_t ide_start = MAX_BASE_EQUIP;
  
  while(getting_string[equip_start] == '0')
    equip_start ++;
  memcpy(equip_62_base_string,getting_string + equip_start,
    MAX_BASE_EQUIP - equip_start);

  while(getting_string[ide_start] == '0')
    ide_start ++;
  memcpy(equip_62_base_ide,getting_string + ide_start,
    MAX_BASE_EQUIP + MAX_BASE_IDENT - ide_start);
}

uint64 CheckTheStringIsCorrect(const char * getting_string){
  InitHEX_BASE();
  char equip_62_base_string[64] = {0};
  char equip_62_base_ide[64] = {0};
  uint64 equip_dec = 0;
  uint64 equip_dec_ide = 0;
  uint64 genarator_ide = 0;
  std::cout << "\n\n----------------------------" << std::endl;

  std::cout << "1. Separate the giving string" << std::endl;
  SeparateEquipGettingString(getting_string,equip_62_base_string,
    equip_62_base_ide);
  std::cout << "\t" << equip_62_base_string
    << "\t" << equip_62_base_ide << std::endl;

  std::cout << "2. convert the base 62 string to uint64 number" << std::endl;
  equip_dec = Base62ToDecimal(equip_62_base_string);
  equip_dec_ide = Base62ToDecimal(equip_62_base_ide);
  std::cout << "\t" << equip_dec
    << "\t" << equip_dec_ide << std::endl;

  std::cout << "3. checkout the base is correct" << std::endl;
  genarator_ide = GenaratorIDE(equip_dec,SCUR);
  std::cout << "\t" << equip_dec_ide << 
    "\t" << genarator_ide << std::endl;
  if(equip_dec_ide == genarator_ide)
    return equip_dec;
  return 0;
}

bool CoverBase16ToEquipIde(const char *base_16_string, char * result){
  size_t len = strlen(base_16_string);
  if(len != 16)
    return false;
  memcpy(result,base_16_string,8);
  result[8] = '-';
  memcpy(result + 9, base_16_string + 8, 8);
  return true;
}

void CovertEquipStringToEquipIDE(uint64 equip_dec_ide,
                                 char *equip_ide){
  
  char base_16_string[64] = {0};

  std::cout << "1. convert the decimal to Hex string" << std::endl;
  DecimalToBase16(equip_dec_ide,base_16_string);
  std::cout << "\t" << base_16_string << std::endl;

  std::cout << "2. convert the base 16 string to equipment identifier"
    << std::endl;
  CoverBase16ToEquipIde(base_16_string,equip_ide);
  std::cout << "\t" << equip_ide << std::endl;
}

int main(void){
  char final_string[64]   = {0};
  char equip_identifier[64] = {0};
  char equip_ide[] = "8771edae-2f73a3ff";
  std::cout << "the equipment identifier is " << equip_ide << std::endl;
  GenaratorSecrecy(equip_ide,final_string);
  std::cout << final_string << std::endl;

  uint64 equip_number = CheckTheStringIsCorrect(final_string);
  if(!equip_number){
    std::cout << "the equipment string is in-value" << std::endl;
  }
  CovertEquipStringToEquipIDE(equip_number,equip_identifier);
  std::cout << equip_identifier << std::endl;

  return 0;
}

