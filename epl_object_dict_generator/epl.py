import xml.etree.ElementTree as ET

#----------------------------------
# Data Types supported in EPL
#----------------------------------
# 0001 - Boolean
# 0002 - Integer8
# 0003 - Integer16
# 0004 - Integer32
# 0005 - Unsigned8
# 0006 - Unsigned16
# 0007 - Unsigned32
# 0008 - Real32
# 0009 - Visible_String
# 0010 - Integer24
# 0011 - Real64
# 0012 - Integer40
# 0013 - Integer48
# 0014 - Integer56
# 0015 - Integer64
# 000A - Octet_String
# 000B - Unicode_String
# 000C - Time_of_Day
# 000D - Time_Diff
# 000F - Domain
# 0016 - Unsigned24
# 0018 - Unsigned40
# 0019 - Unsigned48
# 001A - Unsigned56
# 001B - Unsigned64
# 0401 - MAC_ADDRESS
# 0402 - IP_ADDRESS
# 0403 - NETTIME
#-----------------------------------

tree = ET.parse('epl.xml')
root1 = tree.getroot()

data_types=['0x0001','0x0002','0x0005', '0x0003','0x0006','0x0010','0x0016','0x0004','0x0008','0x0007','0x0012','0x0018','0x0013','0x0019','0x0014','0x001A','0x000F','0x0015','0x0011','0x000D','0x00C','0x001B','0x000A','0x000B','0x0009']

object_details = {}

object_details['name'] = []
object_details['dataType'] = []
object_details['lowLimit'] = []
object_details['highLimit'] = []
object_details['defaultValue'] = []
object_details['index'] = []
object_details['subIndex'] = []
object_details['accessType'] = []
object_details['PDOmapping'] = []
object_details['objectType'] = []
 
co_od_index = []
co_od_si = []
object_type = []

object_var_index = []
object_array_index = []
object_record_index = []
names_list = []
object_names = []
for root in root1:
  for a in root:
    for b in a:
      for l in b:
        for p in l:
          try:
            si=0
            object_type.append('0x'+p.attrib['objectType'])
            names_list.append(p.attrib['name'])
            for ll in p:
              si+=1
              try:
                object_details['objectType'].append('0x'+ll.attrib['objectType'])
              except:
                object_details['objectType'].append('\0')
              object_details['name'].append(ll.attrib['name'])
              object_details['dataType'].append('0x'+ll.attrib['dataType'])
              try:
                object_details['lowLimit'].append(ll.attrib['lowLimit'])
              except:
                object_details['lowLimit'].append('0x00')
              try:
                object_details['highLimit'].append(ll.attrib['highLimit'])
              except:
                object_details['highLimit'].append('0x00')
              try:
                object_details['defaultValue'].append(ll.attrib['defaultValue'])
              except:
                if((ll.attrib['dataType'] == '000A') or (ll.attrib['dataType'] == '000B') or (ll.attrib['dataType'] == '0009')):
                  object_details['defaultValue'].append('\0')
                else:
                  object_details['defaultValue'].append('0x00')
              object_details['index'].append('0x'+p.attrib['index'])
              object_details['subIndex'].append('0x'+ll.attrib['subIndex'])
              object_details['accessType'].append(ll.attrib['accessType'])
              try:
                object_details['PDOmapping'].append(ll.attrib['PDOmapping'])
              except:
                object_details['PDOmapping'].append('no')
              co_od_index.append('0x'+p.attrib['index'])
              co_od_si.append('0x'+ll.attrib['subIndex'])
            if(si == 0):
              try:
                object_details['lowLimit'].append(p.attrib['lowLimit'])
              except:
                object_details['lowLimit'].append('0x00')
              try:
                object_details['highLimit'].append(p.attrib['highLimit'])
              except:
                object_details['highLimit'].append('0x00')
              object_details['name'].append(p.attrib['name'])
              object_details['dataType'].append('0x'+p.attrib['dataType'])
              
              try:
                object_details['defaultValue'].append(p.attrib['defaultValue'])
              except:
                if((p.attrib['dataType'] == '000A') or (p.attrib['dataType'] == '000B') or (p.attrib['dataType'] == '0009')):
                  object_details['defaultValue'].append('\0')
                else:
                  object_details['defaultValue'].append('0x00')
              
              object_details['index'].append('0x'+p.attrib['index'])
              object_details['subIndex'].append('-1')
              object_details['accessType'].append(p.attrib['accessType'])
              try:
                object_details['PDOmapping'].append(ll.attrib['PDOmapping'])
              except:
                object_details['PDOmapping'].append('\0')
              co_od_index.append('0x'+p.attrib['index'])
              co_od_si.append('-1')
          except:
            pass            

NUMBER_OF_OBJECTS=0

obj_index=0
for index in range(0,len(co_od_index)):
  try:
    if((int(co_od_si[index]) == 0 ) or (int(co_od_si[index]) == -1)):
      if(object_type[obj_index] == '0x7'): 
        object_var_index.append(index)
      elif(object_type[obj_index] == '0x8'):
        object_array_index.append(index)
      elif(object_type[obj_index] == '0x9'):
        object_record_index.append(index)
      obj_index+=1
  except:
    if((int(co_od_si[index],16) == 0 ) or (int(co_od_si[index],16) == -1)):
      if(object_type[obj_index] == '0x7'): 
        object_var_index.append(index)
      elif(object_type[obj_index] == '0x8'):
        object_array_index.append(index)
      elif(object_type[obj_index] == '0x9'):
        object_record_index.append(index)
      obj_index+=1
  
file_ptr = open("object_dictionary.h",'w')
file_ptr.write('#ifndef OBJECT_DICTIONARY_H_\n')
file_ptr.write('#define OBJECT_DICTIONARY_H_\n\n')

NUMBER_OF_OBJECTS=0
for index in range(0,len(co_od_index)):
  try:
    if((int(co_od_si[index]) == 0 ) or (int(co_od_si[index]) == -1)):
      NUMBER_OF_OBJECTS+=1
  except:
    if((int(co_od_si[index],16) == 0 ) or (int(co_od_si[index],16) == -1)):
      NUMBER_OF_OBJECTS+=1
file_ptr.write('#define NUMBER_OF_OBJECTS '+str(NUMBER_OF_OBJECTS)+'\n')
file_ptr.write('#define NUMBER_OF_VAR_OBJECTS '+str(len(object_var_index))+'\n')
file_ptr.write('#define NUMBER_OF_ARRAY_OBJECTS '+str(len(object_array_index))+'\n')
file_ptr.write('#define NUMBER_OF_RECORD_OBJECTS '+str(len(object_record_index))+'\n\n')
file_ptr.write('typedef enum {\n  RW,\n  RO,\n  CO,\n  WO\n} access_t;\n\n')
file_ptr.write('typedef enum {\n  NO,\n  OPTIO,\n  TPDO,\n  RPDO\n  YES\n  DEFA\n} pdo_t;\n\n')
file_ptr.write('typedef enum {\n  true,\n  false,\n} options_t;\n\n')
NUMBER_OF_OBJECTS=0        
for names in names_list:
  for character in names:
    object_names.append(character)
  object_names.append('\0')

file_ptr.write('unsigned object_name['+str(len(object_names))+'] = {')
for index in range(0,len(object_names)):
  if(index%15 == 0):
    file_ptr.write('\n  ')
  if(object_names[index] == '\0'):
    file_ptr.write("%4s, "%object_names[index])
  else:
    file_ptr.write("'"+object_names[index]+"', ")
file_ptr.write('\n}\n\n')
  
file_ptr.write('unsigned data_type['+str(len(object_details['dataType']))+'] = {')
for index in range(0,len(object_details['dataType'])):
  if(index%5 == 0):
    file_ptr.write('\n  ')
  file_ptr.write(object_details['dataType'][index]+', ')
file_ptr.write('\n}\n\n')

file_ptr.write('unsigned char access_type['+str(len(object_details['accessType']))+'] = {')
for index in range(0,len(object_details['accessType'])):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  if(object_details['accessType'][index] == 'ro'):
    file_ptr.write('RO, ')
  elif(object_details['accessType'][index] == 'wo'):  
    file_ptr.write('WO, ')
  elif(object_details['accessType'][index] == 'rw'):  
    file_ptr.write('RW, ')
  elif(object_details['accessType'][index] == 'const'):  
    file_ptr.write('CO, ')
file_ptr.write('\n}\n\n')

file_ptr.write('unsigned char pdo_mapping['+str(len(object_details['PDOmapping']))+'] = {')
for index in range(0,len(object_details['PDOmapping'])):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  if(object_details['PDOmapping'][index] == 'default'):
    file_ptr.write('DEFA'+', ')
  elif(object_details['PDOmapping'][index] == 'no'):
    file_ptr.write('NO, ')
  elif(object_details['PDOmapping'][index] == 'yes'):
    file_ptr.write('YES, ')
  elif(object_details['PDOmapping'][index] == 'optional'):
    file_ptr.write('OPTIO, ')
  else:
    file_ptr.write(object_details['PDOmapping'][index]+', ')
      
file_ptr.write('\n}\n\n')

default_values_list = []
low_limit_list = []
high_limit_list = []

for index in range(0,len(object_details['defaultValue'])):  
  if((object_details['dataType'][index] == '0x0001') or (object_details['dataType'][index] == '0x0002') or (object_details['dataType'][index] == '0x0005')):
    default_values_list.append(object_details['defaultValue'][index])
    low_limit_list.append(object_details['lowLimit'][index])
    high_limit_list.append(object_details['highLimit'][index])
  elif((object_details['dataType'][index] == '0x0003') or (object_details['dataType'][index] == '0x0006')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
  elif((object_details['dataType'][index] == '0x0010') or (object_details['dataType'][index] == '0x0016')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
  elif((object_details['dataType'][index] == '0x0004') or (object_details['dataType'][index] == '0x0008') or (object_details['dataType'][index] == '0x0007')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
  elif((object_details['dataType'][index] == '0x0012') or (object_details['dataType'][index] == '0x0018')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000)>>32))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000)>>32))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000)>>32))
  elif((object_details['dataType'][index] == '0x0013') or (object_details['dataType'][index] == '0x0019')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000)>>32))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000000000)>>40))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000)>>32))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000000000)>>40))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000)>>32))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000000000)>>40))
  elif((object_details['dataType'][index] == '0x0014') or (object_details['dataType'][index] == '0x001A')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000)>>32))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000000000)>>40))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000000000)>>48))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000)>>32))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000000000)>>40))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000000000)>>48))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000)>>32))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000000000)>>40))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000000000)>>48))
  elif((object_details['dataType'][index] == '0x000F') or (object_details['dataType'][index] == '0x0015') or (object_details['dataType'][index] == '0x0011')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000)>>32))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000000000)>>40))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000000000)>>48))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000000000)>>52))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000)>>32))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000000000)>>40))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000000000)>>48))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000000000)>>52))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000)>>32))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000000000)>>40))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000000000)>>48))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000000000)>>52))
  elif((object_details['dataType'][index] == '0x000C') or (object_details['dataType'][index] == '0x000D') or (object_details['dataType'][index] == '0x001B')):
        try:
          object_details['defaultValue'][index] = hex(int(object_details['defaultValue'][index]))
        except:
          pass
        default_values_list.append(str(long(object_details['defaultValue'][index],16)&0xFF))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00)>>8))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000)>>16))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000)>>24))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000)>>32))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF0000000000)>>40))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF000000000000)>>48))
        default_values_list.append(str((long(object_details['defaultValue'][index],16)&0xFF00000000000000)>>52))
        try:
          object_details['lowLimit'][index] = hex(int(object_details['lowLimit'][index]))
        except:
          pass
        low_limit_list.append(str(long(object_details['lowLimit'][index],16)&0xFF))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00)>>8))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000)>>16))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000)>>24))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000)>>32))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF0000000000)>>40))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF000000000000)>>48))
        low_limit_list.append(str((long(object_details['lowLimit'][index],16)&0xFF00000000000000)>>52))
        try:
          object_details['highLimit'][index] = hex(int(object_details['highLimit'][index]))
        except:
          pass
        high_limit_list.append(str(long(object_details['highLimit'][index],16)&0xFF))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00)>>8))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000)>>16))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000)>>24))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000)>>32))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF0000000000)>>40))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF000000000000)>>48))
        high_limit_list.append(str((long(object_details['highLimit'][index],16)&0xFF00000000000000)>>52))
  elif((object_details['dataType'][index] == '0x000A') or (object_details['dataType'][index] == '0x000B') or (object_details['dataType'][index] == '0x0009')):
    for i in object_details['defaultValue'][index]:
      if(i == '\0'):
        default_values_list.append(i)
      else:
        default_values_list.append("'"+i+"'")
    default_values_list.append('\0')
    for i in object_details['lowLimit'][index]:
      if(i == '\0'):
        low_limit_list.append(i)
      elif(object_details['lowLimit'][index] == '0x00'):
        low_limit_list.append('\0')
      else:
        low_limit_list.append("'"+i+"'")
    low_limit_list.append('\0')
    for i in object_details['highLimit'][index]:
      if(i == '\0'):
        high_limit_list.append(i)
      elif(object_details['highLimit'][index] == '0x00'):
        high_limit_list.append('\0')
      else:
        high_limit_list.append("'"+i+"'")
    high_limit_list.append('\0')

file_ptr.write('unsigned char default_value['+str(len(default_values_list))+'] = {')
for index in range(0,len(default_values_list)):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  file_ptr.write("%4s, "%default_values_list[index])
file_ptr.write('\n}\n\n')

file_ptr.write('unsigned char low_limit['+str(len(low_limit_list))+'] = {')
for index in range(0,len(low_limit_list)):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  file_ptr.write("%4s, "%low_limit_list[index])
file_ptr.write('\n}\n\n')

file_ptr.write('unsigned char high_limit['+str(len(high_limit_list))+'] = {')
for index in range(0,len(high_limit_list)):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  file_ptr.write("%4s, "%high_limit_list[index])
file_ptr.write('\n}\n\n')

file_ptr.write('unsigned char current_value['+str(len(default_values_list))+'] = {')
for index in range(0,len(default_values_list)):
  if(index%10 == 0):
    file_ptr.write('\n  ')
  file_ptr.write("%4s, "%default_values_list[index])
file_ptr.write('\n}\n\n')

file_ptr.write('s_var var_list[NUMBER_OF_VAR_OBJECTS] = {\n')
for  index in range(0,len(object_var_index)):
  def_no_of_bytes=0
  low_no_of_bytes=0
  high_no_of_bytes=0
  for def_index in range(0,object_var_index[index]):
    if((object_details['dataType'][def_index] == '0x0001') or (object_details['dataType'][def_index] == '0x0002') or (object_details['dataType'][def_index] == '0x0005')):
      def_no_of_bytes+=1
      low_no_of_bytes+=1
      high_no_of_bytes+=1
    elif((object_details['dataType'][def_index] == '0x0003') or (object_details['dataType'][def_index] == '0x0006')):
      def_no_of_bytes+=2
      low_no_of_bytes+=2
      high_no_of_bytes+=2
    elif((object_details['dataType'][def_index] == '0x0016') or (object_details['dataType'][def_index] == '0x0010')):
      def_no_of_bytes+=3
      low_no_of_bytes+=3
      high_no_of_bytes+=3
    elif((object_details['dataType'][def_index] == '0x0004') or (object_details['dataType'][def_index] == '0x0008') or (object_details['dataType'][def_index] == '0x0007')):
      def_no_of_bytes+=4
      low_no_of_bytes+=4
      high_no_of_bytes+=4
    elif((object_details['dataType'][def_index] == '0x0012') or (object_details['dataType'][def_index] == '0x0018')):
      def_no_of_bytes+=5
      low_no_of_bytes+=5
      high_no_of_bytes+=5
    elif((object_details['dataType'][def_index] == '0x0013') or (object_details['dataType'][def_index] == '0x0019')):
      def_no_of_bytes+=6
      low_no_of_bytes+=6
      high_no_of_bytes+=6
    elif((object_details['dataType'][def_index] == '0x0014') or (object_details['dataType'][def_index] == '0x001A')):
      def_no_of_bytes+=7
      low_no_of_bytes+=7
      high_no_of_bytes+=7
    elif((object_details['dataType'][def_index] == '0x000F') or (object_details['dataType'][def_index] == '0x0015') or (object_details['dataType'][def_index] == '0x0011')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000D') or (object_details['dataType'][def_index] == '0x000C') or (object_details['dataType'][def_index] == '0x001B')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000A') or (object_details['dataType'][def_index] == '0x000B') or (object_details['dataType'][def_index] == '0x0009')):
      def_no_of_bytes+=len(object_details['defaultValue'][def_index])+1
      low_no_of_bytes+=len(object_details['lowLimit'][def_index])+1
      high_no_of_bytes+=len(object_details['highLimit'][def_index])+1
  file_ptr.write('  {&data_type['+str(object_var_index[index])+'], &access_type['+str(object_var_index[index])+'], &pdo_mappable['+str(object_var_index[index])+'], '+'&default_value['+str(def_no_of_bytes)+'], '+'&low_value['+str(low_no_of_bytes)+'], '+'&high_value['+str(high_no_of_bytes)+'], '+'&current_value['+str(def_no_of_bytes)+'],0},\n')
file_ptr.write('}\n\n')

file_ptr.write('s_array array_list[NUMBER_OF_ARRAY_OBJECTS] = {\n')
for  index in range(0,len(object_array_index)):
  def_no_of_bytes=0
  low_no_of_bytes=0
  high_no_of_bytes=0
  for def_index in range(0,object_array_index[index]):
    if((object_details['dataType'][def_index] == '0x0001') or (object_details['dataType'][def_index] == '0x0002') or (object_details['dataType'][def_index] == '0x0005')):
      def_no_of_bytes+=1
      low_no_of_bytes+=1
      high_no_of_bytes+=1
    elif((object_details['dataType'][def_index] == '0x0003') or (object_details['dataType'][def_index] == '0x0006')):
      def_no_of_bytes+=2
      low_no_of_bytes+=2
      high_no_of_bytes+=2
    elif((object_details['dataType'][def_index] == '0x0016') or (object_details['dataType'][def_index] == '0x0010')):
      def_no_of_bytes+=3
      low_no_of_bytes+=3
      high_no_of_bytes+=3
    elif((object_details['dataType'][def_index] == '0x0004') or (object_details['dataType'][def_index] == '0x0008') or (object_details['dataType'][def_index] == '0x0007')):
      def_no_of_bytes+=4
      low_no_of_bytes+=4
      high_no_of_bytes+=4
    elif((object_details['dataType'][def_index] == '0x0012') or (object_details['dataType'][def_index] == '0x0018')):
      def_no_of_bytes+=5
      low_no_of_bytes+=5
      high_no_of_bytes+=5
    elif((object_details['dataType'][def_index] == '0x0013') or (object_details['dataType'][def_index] == '0x0019')):
      def_no_of_bytes+=6
      low_no_of_bytes+=6
      high_no_of_bytes+=6
    elif((object_details['dataType'][def_index] == '0x0014') or (object_details['dataType'][def_index] == '0x001A')):
      def_no_of_bytes+=7
      low_no_of_bytes+=7
      high_no_of_bytes+=7
    elif((object_details['dataType'][def_index] == '0x000F') or (object_details['dataType'][def_index] == '0x0015') or (object_details['dataType'][def_index] == '0x0011')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000D') or (object_details['dataType'][def_index] == '0x000C') or (object_details['dataType'][def_index] == '0x001B')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000A') or (object_details['dataType'][def_index] == '0x000B') or (object_details['dataType'][def_index] == '0x0009')):
      def_no_of_bytes+=len(object_details['defaultValue'][def_index])+1
      low_no_of_bytes+=len(object_details['lowLimit'][def_index])+1
      high_no_of_bytes+=len(object_details['highLimit'][def_index])+1
  if object_details['index'][object_array_index[index]] in co_od_index:
    temp_lisst1 = [x for x in co_od_index if x==object_details['index'][object_array_index[index]]]
  file_ptr.write('  {'+str(len(temp_lisst1))+', &data_type['+str(object_array_index[index])+'], &access_type['+str(object_array_index[index])+'], &pdo_mappable['+str(object_array_index[index])+'], '+'&default_value['+str(def_no_of_bytes)+'], '+'&low_value['+str(low_no_of_bytes)+'], '+'&high_value['+str(high_no_of_bytes)+'], '+'&current_value['+str(def_no_of_bytes)+'], 0},\n')
file_ptr.write('}\n\n')

file_ptr.write('s_record record_list[NUMBER_OF_RECORD_OBJECTS] = {\n')
for  index in range(0,len(object_record_index)):
  def_no_of_bytes=0
  low_no_of_bytes=0
  high_no_of_bytes=0
  for def_index in range(0,object_record_index[index]):
    if((object_details['dataType'][def_index] == '0x0001') or (object_details['dataType'][def_index] == '0x0002') or (object_details['dataType'][def_index] == '0x0005')):
      def_no_of_bytes+=1
      low_no_of_bytes+=1
      high_no_of_bytes+=1
    elif((object_details['dataType'][def_index] == '0x0003') or (object_details['dataType'][def_index] == '0x0006')):
      def_no_of_bytes+=2
      low_no_of_bytes+=2
      high_no_of_bytes+=2
    elif((object_details['dataType'][def_index] == '0x0016') or (object_details['dataType'][def_index] == '0x0010')):
      def_no_of_bytes+=3
      low_no_of_bytes+=3
      high_no_of_bytes+=3
    elif((object_details['dataType'][def_index] == '0x0004') or (object_details['dataType'][def_index] == '0x0008') or (object_details['dataType'][def_index] == '0x0007')):
      def_no_of_bytes+=4
      low_no_of_bytes+=4
      high_no_of_bytes+=4
    elif((object_details['dataType'][def_index] == '0x0012') or (object_details['dataType'][def_index] == '0x0018')):
      def_no_of_bytes+=5
      low_no_of_bytes+=5
      high_no_of_bytes+=5
    elif((object_details['dataType'][def_index] == '0x0013') or (object_details['dataType'][def_index] == '0x0019')):
      def_no_of_bytes+=6
      low_no_of_bytes+=6
      high_no_of_bytes+=6
    elif((object_details['dataType'][def_index] == '0x0014') or (object_details['dataType'][def_index] == '0x001A')):
      def_no_of_bytes+=7
      low_no_of_bytes+=7
      high_no_of_bytes+=7
    elif((object_details['dataType'][def_index] == '0x000F') or (object_details['dataType'][def_index] == '0x0015') or (object_details['dataType'][def_index] == '0x0011')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000D') or (object_details['dataType'][def_index] == '0x000C') or (object_details['dataType'][def_index] == '0x001B')):
      def_no_of_bytes+=8
      low_no_of_bytes+=8
      high_no_of_bytes+=8
    elif((object_details['dataType'][def_index] == '0x000A') or (object_details['dataType'][def_index] == '0x000B') or (object_details['dataType'][def_index] == '0x0009')):
      def_no_of_bytes+=len(object_details['defaultValue'][def_index])+1
      low_no_of_bytes+=len(object_details['lowLimit'][def_index])+1
      high_no_of_bytes+=len(object_details['highLimit'][def_index])+1
  if object_details['index'][object_record_index[index]] in co_od_index:
    temp_lisst1 = [x for x in co_od_index if x==object_details['index'][object_record_index[index]]]
  file_ptr.write('  {'+str(len(temp_lisst1))+', &data_type['+str(object_record_index[index])+'], &access_type['+str(object_record_index[index])+'], &pdo_mappable['+str(object_record_index[index])+'], '+'&default_value['+str(def_no_of_bytes)+'], '+'&low_value['+str(low_no_of_bytes)+'], '+'&high_value['+str(high_no_of_bytes)+'], '+'&current_value['+str(def_no_of_bytes)+'], 0},\n')
file_ptr.write('}\n\n')
   
var_index=0
array_index=0
record_index=0
length=0
file_ptr.write('object_t objects[NUMBER_OF_OBJECTS]={\n')
for index in range(0,len(co_od_index)):
  try:
    if((int(co_od_si[index]) == 0 ) or (int(co_od_si[index]) == -1)):
      if co_od_index[index] in co_od_index:
        temp_lisst = [x for x in co_od_index if x==co_od_index[index]]
        NUMBER_OF_OBJECTS +=1
        length=0
        for names in range(0,NUMBER_OF_OBJECTS-1):
          length+=len(names_list[names])
        if(NUMBER_OF_OBJECTS == 1):
          length=-1
        if(object_type[NUMBER_OF_OBJECTS-1] == '0x7'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &var_list['+str(var_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          var_index+=1
        elif(object_type[NUMBER_OF_OBJECTS-1] == '0x8'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &array_list['+str(array_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          array_index+=1
        elif(object_type[NUMBER_OF_OBJECTS-1] == '0x9'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &record_list['+str(record_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          record_index+=1
  except:
    if((int(co_od_si[index],16) == 0 ) or (int(co_od_si[index],16) == -1)):
      if co_od_index[index] in co_od_index:
        temp_lisst = [x for x in co_od_index if x==co_od_index[index]]
        NUMBER_OF_OBJECTS +=1
        length=0
        for names in range(0,NUMBER_OF_OBJECTS):
          length+=len(names_list[names])
        if(object_type[NUMBER_OF_OBJECTS-1] == '0x7'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &var_list['+str(var_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          var_index+=1
        elif(object_type[NUMBER_OF_OBJECTS-1] == '0x8'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &array_list['+str(array_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          array_index+=1
        elif(object_type[NUMBER_OF_OBJECTS-1] == '0x9'):
          file_ptr.write('  {'+co_od_index[index]+', '+object_type[NUMBER_OF_OBJECTS-1]+', &record_list['+str(record_index)+'], &object_name['+str(length+NUMBER_OF_OBJECTS)+']},\n')
          record_index+=1
file_ptr.write("};\n\n")
file_ptr.write("#end if /* OBJECT_DICTIONARY_H_ */")
