#include "object_dictionary.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct {
  char * name;
  unsigned short data_type;
}lut ;

static const lut t[28] = {
  {"Boolean",0x0001},
  {"Integer8", 0x0002},
  {"Integer16", 0x0003},
  {"Integer32", 0x0004},
  {"Unsigned8", 0x0005},
  {"Unsigned16", 0x0006},
  {"Unsigned32 ", 0x0007},
  {"Real32", 0x0008},
  {"Visible_String", 0x0009},
  {"Octet_String", 0x000A},
  {"Unicode_String", 0x000B},
  {"Time_of_Day", 0x000C},
  {"Time_Diff", 0x000D},
  {"Domain", 0x000F},
  {"Integer24", 0x0010},
  {"Real64", 0x0011},
  {"Integer40", 0x0012},
  {"Integer48", 0x0013},
  {"Integer56", 0x0014},
  {"Integer64", 0x0015},
  {"Unsigned24", 0x0016},
  {"Unsigned40", 0x0018},
  {"Unsigned48", 0x0019},
  {"Unsigned56", 0x001A},
  {"Unsigned64", 0x001B},
  {"MAC_ADDRESS", 0x0401},
  {"IP_ADDRESS", 0x0402},
  {"NETTIME", 0x0403},
};
char * get_name(unsigned short dt){
  for(unsigned i=0;i<28;i++)
    if(t[i].data_type==dt) return t[i].name;
  return 0;
}

void indent(unsigned x){
  for(unsigned i=0;i<x;i++) printf("\t");
}

void print_value(unsigned short data_type, void * data){
  if(data == 0) return;

  switch(data_type){
  case 0x0001:{ printf("Boolean: %d", *(char*)data); break; }
  case 0x0002:{ printf("Integer8: %d", *(char*)data); break; }
  case 0x0003:{ printf("Integer16: %d", *(short*)data); break; }
  case 0x0004:{ printf("Integer32: %d(%08x)", *(int*)data, *(int*)data); break; }
  case 0x0005:{ printf("Unsigned8: %d", *(unsigned char*)data); break; }
  case 0x0006:{ printf("Unsigned16: %d", *(unsigned short*)data); break; }
  case 0x0007:{ printf("Unsigned32: %d(%08x)", *(unsigned*)data,*(unsigned*)data); break; }
  case 0x0008:{ printf("Real32: %f", *(float*)data); break; }
  case 0x0009:{ printf("Visible_String: %s", (char*)data); break; }
//case 0x0010:{ printf("Integer24: %d", *(short*)data); break; }
  case 0x0011:{ printf("Real64: %d", *(short*)data); break; }
  //case 0x0012:{ printf("Integer40: %d", *(short*)data); break; }
  //case 0x0013:{printf("Integer48: %d", *(short*)data); break; }
  //case 0x0014:{ printf("Integer56: %d", *(short*)data); break; }
  case 0x0015:{ printf("Integer64: %d", *(short*)data); break; }
  case 0x000a:{ printf("Octet_String: %d", *(short*)data); break; }
  case 0x000b:{ printf("Unicode_String: %d", *(short*)data); break; }
  case 0x000c:{ printf("Time_of_Day: %d", *(short*)data); break; }
  case 0x000d:{ printf("Time_Diff: %d", *(short*)data); break; }
  case 0x000f:{ printf("Domain: %d", *(short*)data); break; }
  //case 0x0016:{ printf("Unsigned24: %d", *(short*)data); break; }
  //case 0x0018:{ printf("Unsigned40: %d", *(short*)data); break; }
  //case 0x0019:{ printf("Unsigned48: %d", *(short*)data); break; }
  //case 0x001a:{ printf("Unsigned56: %d", *(short*)data); break; }
  case 0x001b:{ printf("Unsigned64: %llu", *(unsigned long long*)data); break; }
  case 0x0401:{ printf("MAC_ADDRESS: %d", *(short*)data); break; }
  case 0x0402:{ printf("IP_ADDRESS: %d", *(short*)data); break; }
  case 0x0403:{ printf("NETTIME: %d", *(short*)data); break; }
  default:{ printf("????? "); break; }
  }
}

void print_var(var_t * v, unsigned i){
  indent(i);
  printf("Data type: %d : %s\n",v->data_type, get_name(v->data_type));


/*
  if(v->low_value){
    indent(i);
    printf("Low Value Range: ");
    print_value(v->data_type, v->low_value);
    printf("\n");
  }
  if(v->high_value){
    indent(i);
    printf("High Value Range: ");
    print_value(v->data_type, v->high_value);
    printf("\n");
  }
  if(v->default_value){
    indent(i);
    printf("Default Value:");
    print_value(v->data_type, v->default_value);
    printf("\n");
  }
  */
  if(v->value){
    indent(i);
    printf("Value: ");
    print_value(v->data_type, v->value);
    printf("\n");
  }

  indent(i);
  switch(v->access){
  case access_rw: printf("Access: RW\n"); break;
  case access_ro: printf("Access: RO\n"); break;
  case access_wo: printf("Access: WO\n"); break;
  case access_const: printf("Access: CONST\n"); break;
  default:
    printf("ERROR in OD\n");
    exit(1);
  }

  indent(i);
  switch(v->pdo_mapable){
  case pdo_no:{
    printf("PDO mappable: No\n");break;
  }
  case pdo_default:{
    printf("PDO mappable: Default\n");break;
  }
  case pdo_optional:{
    printf("PDO mappable: Optional\n");break;
  }
  case pdo_tpdo:{
    printf("PDO mappable: TPDO\n");break;
  }
  case pdo_rpdo:{
    printf("PDO mappable: RPDO\n");break;
  }
  default:{
    printf("ERROR in OD\n");
   exit(1);
  }
  }
}




int main(){
  for(unsigned object = 0; object < NUM_OBJECTS; object++){
    object_t * o = &(objects[object]);
    printf("Object code: %08x\n", o->object_code);
    printf("Data: %p\n", o->data);
    printf("Name: %s\n", o->name);
    printf("Index: %08x\n", o->index);

    if(o->object_code == 7){
      print_var((var_t*)o->data, 1);
    } else if(o->object_code == 8){
      //print the array
      array_t * a= (array_t*)o->data;
      printf("Array Data type: %d\n",a->data_type);

      printf("Array Data num bytes: %d\n",a->data_num_bytes);

      printf("Subindex max_number_of_entries: %d\n", a->si_max_number_of_entries);

      for(unsigned i=0;i<a->si_max_number_of_entries; i++){
        indent(1);
        printf("subindex %d\n", i);
        print_value(a->data_type, a->value + a->data_num_bytes * i);
        printf("\n");
      }

    } else if(o->object_code == 9){
      //print the record
      record_t * r= (record_t*)o->data;


      printf("Subindex max_number_of_entries: %d\n", r->si_max_number_of_entries);

      for(unsigned i=0;i<r->si_max_number_of_entries; i++){
        indent(1);
        printf("subindex %d\n", i);
        if((r->sub_indicies)[i].name){
          indent(1);
          printf("Name: %s\n", (r->sub_indicies)[i].name);
        }
        print_var(&((r->sub_indicies)[i].var), 1);

        printf("\n");
      }
    } else {
      printf("ERROR in OD\n");
      exit(1);
    }
    printf("\n");
    printf("-------------------------------------------------------------\n");
  }
}
