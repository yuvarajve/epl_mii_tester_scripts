import xml.etree.ElementTree as ET
import sys
tree = ET.parse('401.xml')

#TODO
#pay back a lot of technical debt!
#lots of this is very hacky and lots of data sanitising and verification needs to happen

string_len_lut = {
'PDO_ErrMapVers_OSTR':32,
'PDO_ErrShort_RX_OSTR':32,
'InterfacePhysAddress_OSTR':6,
'NMT_HostName_VSTR':32,
}
#NMT_ManufactDevName_VS
#NMT_ManufactHwVers_VS
#NMT_ManufactSwVers_VS
#InterfaceDescription_VSTR
#InterfaceName_VSTR


data_size_lu = {
'0001': 1, 
'0002': 1, 
'0005': 1, 
'0003': 2, 
'0006': 2, 
'0010': 3, 
'0016': 3,
'0007': 4, 
'0402': 4, 
'0008': 4, 
'0004': 4, 
'0012': 5,
'0018': 5, 
'0013': 6, 
'000C': 6, 
'000D': 6,  
'0401': 6, 
'0019': 6, 
'001A': 7, 
'0014': 7, 
'0015': 8,  
'001B': 8, 
'0011': 8, 
'0403': 8
}

object_table = []
data_type_table = []

class Var:
  def __init__(self, data_type):
    self.data_type = data_type
    self.name = None
    self.low_limit = None
    self.high_limit = None
    self.access_type = None
    self.default_value = None
    self.actual_value = None
    self.PDO_mapping = None


def process_object(obj, object_table):
  object_index = obj.get("index")
  object_name = obj.get("name")
  object_type = int(obj.get("objectType"))

  if object_type == 0: #null
    return
  if object_type == 5: #deftype
    #TODO add this type to the data_type_table
    print "ERROR: unhandled DEFTYPE"
  if object_type == 6: #defstruct
    print "ERROR: unhandled DEFSTRUCT"
  if object_type == 7: #var
    var_data_type = obj.get("dataType")
    var = Var(var_data_type)
    var.name = None
    var.low_limit = obj.get("lowLimit")
    var.high_limit = obj.get("highLimit")
    var.access_type = obj.get("accessType")
    var.default_value = obj.get("defaultValue")
    var.actual_value = obj.get("actualValue")
    var.PDO_mapping = obj.get("PDOmapping")
    # TODO denotation
    # TODO objFlags
    # TODO uniqueIDRef
    object_table.append([object_index, object_name, object_type, var])

  if object_type == 8: #array
    array_data_type = None
    array_access = None
    array_pdo_mappable = None
    array_default_value = None
    array_hi_limit = None
    array_lo_limit = None
    si_table = {}
    for si in obj:  
      if int(si.get("subIndex"), 16) != 0:
        
        if array_data_type == None:
          array_data_type = si.get("dataType")
          array_access = si.get("accessType")
          array_default_value = si.get("defaultValue")
          array_pdo_mappable = si.get("PDOmapping")
          array_lo_limit = si.get("lowLimit")
          array_hi_limit = si.get("highLimit")
        else:
          if array_data_type != si.get("dataType"):
            print "ERROR: inconsistant data in array"
          if array_access != si.get("accessType"):
            print "ERROR: inconsistant data in array"
          if array_default_value != si.get("defaultValue"):
            print "ERROR: inconsistant data in array"
          if array_pdo_mappable != si.get("PDOmapping"):
            print "ERROR: inconsistant data in array"
          if array_lo_limit != si.get("lowLimit"):
            print "ERROR: inconsistant data in array"
          if array_hi_limit != si.get("highLimit"):
            print "ERROR: inconsistant data in array"

      si_var = Var(si.get("dataType"))

      si_var.name = si.get("name")
      si_var.access_type = si.get("accessType")
      si_var.PDO_mapping = si.get("PDOmapping")
      si_var.default_value = si.get("defaultValue")
      si_var.actual_value = si.get("actualValue")
      si_var.low_limit = si.get("lowLimit")
      si_var.high_limit = si.get("highLimit")
      si_table[int(si.get("subIndex"), 16)] = si_var 

    object_table.append([object_index, object_name, object_type, array_data_type, array_access, array_default_value, array_pdo_mappable, array_lo_limit, array_hi_limit, si_table])
  if object_type == 9: #array
    si_table = {}
    for si in obj:  
      si_data_type = si.get("dataType")
      sub_index = si.get("subIndex")
      sub_index = int(sub_index, 16)
      si_var = Var(si_data_type)

      si_var.name = si.get("name")
      si_var.access_type = si.get("accessType")
      si_var.PDO_mapping = si.get("PDOmapping")
      si_var.default_value = si.get("defaultValue")
      si_var.actual_value = si.get("actualValue")
      si_var.low_limit = si.get("lowLimit")
      si_var.high_limit = si.get("highLimit")
      si_table[sub_index] = si_var 

    object_table.append([object_index, object_name, object_type,  si_table])
  return

ns='http://www.ethernet-powerlink.org'

for root in tree.findall("{http://www.ethernet-powerlink.org}ISO15745Profile"):
  header = root.find("{http://www.ethernet-powerlink.org}ProfileHeader")
  body = root.find("{http://www.ethernet-powerlink.org}ProfileBody")  
  class_id = header.find("{http://www.ethernet-powerlink.org}ProfileClassID")
  if class_id.text == "CommunicationNetwork":
    profile_name = header.find("{http://www.ethernet-powerlink.org}ProfileName")
    app_layers = body.find("{http://www.ethernet-powerlink.org}ApplicationLayers");
    if app_layers == None:
      print "ERROR: app layers missing"
      continue

    identity = app_layers.find("{http://www.ethernet-powerlink.org}identity");
    data_type_list = app_layers.find("{http://www.ethernet-powerlink.org}DataTypeList");
    object_list = app_layers.find("{http://www.ethernet-powerlink.org}ObjectList");

    #identity is allowed to not exist
    #if identity != None:
      #print identity
      #vendorID
      #deviceFamily
      #productID
      #version
      #buildDate
      #specificationRevision

    if data_type_list == None:
      print "ERROR: missing DataTypeList"
    if object_list == None:
      print "ERROR: missing ObjectList"
    
    for b in data_type_list:
      data_type_index = b.get("dataType")
      data_type_name = b[0].tag.replace("{http://www.ethernet-powerlink.org}","")
      data_type_table.append([data_type_index, data_type_name])
      #TODO check we know about this entry in the lut

    #if object_list.size > 65535:
    #  print "ERROR: too many objects in ObjectList"
    #  sys.exit(1)

    #TODO look at mandatoryObjects
    #TODO look at optionalObjects
    #TODO look at manufacturerObjects

    for obj in object_list:
      process_object(obj, object_table)
      
# generation time!
# Notes:
# - Only 4 and 8 bytes data types are word aligned
# - Two byte data types are 2 bytes aligned
# - Everything else is byte aligned
# - If not PDO mapping is specified then the object is set to not mappable
# - If no access type is specified then the object is const

#TODO implement the space saving 

word_string = ""
short_string = ""
byte_string = ""
word_string_count = 0
short_string_count = 0
byte_string_count = 0

object_defines = ""

def clean_data(s):
  if s.find("0x") != -1:
    return int(s.replace("0x", ""), 16)
  return int(s)

name_table_string = "char name_table[NAME_TABLE_SIZE] = {\n" 

var_string = "var_t vars[NUM_VARS]={\n";
array_string = "array_t arrays[NUM_ARRAYS]={\n";
record_string = "record_t records[NUM_RECORDS]={\n";
record_si_string = "record_subindex_t record_si[NUM_RECORD_SIS]={\n";

object_string = "object_t objects[NUM_OBJECTS] = {\n";

name_table_char_count = 0
var_count = 0
array_count = 0
record_count = 0
object_count = 0
record_si_count = 0

const_name_table_lu = {}
const_word_table_lu = {}
const_short_table_lu = {}
const_byte_table_lu = {}

#add the string to the name table
#return where it is located
def emit_string(string, const, min_bytes):
  global const_name_table_lu
  global name_table_char_count
  global name_table_string

  #if we have already emitted this string as a constant string before
  if string in const_name_table_lu and const == True:
    return const_name_table_lu[string]

  #emit the string

  location = "name_table + " + str(name_table_char_count)

  name_table_string += "\t"
  for char in string:
    name_table_string += "\'" + char + "\',"
  name_table_string += "0,"
  if min_bytes > 0 and len(string) + 1 < min_bytes:
    for i in range(0, min_bytes - (len(string) + 1)):
      name_table_string += "0,"
      name_table_char_count += 1
  name_table_string += "\n"

  name_table_char_count += len(string) + 1

  #if constant then add it to the lut
  if const == True:
    const_name_table_lu[string] = location

  return location

def emit_BOOLEAN(value, constant):
  global byte_string
  global byte_string_count
  pointer = "byte_table + "+ str(byte_string_count)
  if value == "true":
    byte_string += "0x01, "
  elif value == "false":
    byte_string += "0x00, "
  else:
    print "ERROR: unknown BOOLEAN: " + value
  byte_string_count+=1
  if byte_string_count%8 == 7:
    byte_string += "\n\t"
  return pointer

def emit_bytes(value, constant, byte_count):
  global byte_string
  global byte_string_count
  pointer = "byte_table + "+ str(byte_string_count)
  value =  clean_data(value)
  for i in range(0, byte_count):
    byte_string += '0x%02x' % value+ ", "
    if byte_string_count%8 == 7:
      byte_string += "\n\t"
    byte_string_count+=1
    value = value >> 8
  return pointer

def emit_shorts(value, constant, short_count):
  global short_string
  global short_string_count
  pointer = "short_table + "+ str(short_string_count)
  value =  clean_data(value)
  for i in range(0, short_count):
    short_string += '0x%04x' % value+ ", "
    if short_string_count%8 == 7:
      short_string += "\n\t"
    short_string_count+=1
    value = value >> 16
  return pointer

def emit_words(value, constant, word_count):
  global word_string
  global word_string_count
  pointer = "word_table + "+ str(word_string_count)
  value =  clean_data(value)
  for i in range(0, word_count):
    word_string += '0x%08x' % value+ ", "
    if word_string_count%8 == 7:
      word_string += "\n\t"
    word_string_count+=1
    value = value >> 32
  
  return pointer

def emit_data(value, data_type, constant):
  if data_type == '0001':
    return emit_BOOLEAN(value, constant)
  elif data_type == '0002' or data_type == '0005':
    return emit_bytes(value, constant, 1)
  elif data_type == '0003' or data_type == '0006':
    return emit_shorts(value, constant, 1)
  elif data_type == '0004' or data_type == '0007' or data_type == '0008' :
    return emit_words(value, constant, 1)
  elif data_type == '000C' or data_type == '000D' or data_type == '0010' or data_type == '0016':
    return emit_bytes(value, constant, 3)
  elif data_type == '00012' or data_type == '0018':
    return emit_bytes(value, constant, 5)
  elif data_type == '00013' or data_type == '0019':
    return emit_bytes(value, constant, 6)
  elif data_type == '00014' or data_type == '001A':
    return emit_bytes(value, constant, 7)
  elif data_type == '00011' or data_type == '0015'  or data_type == '001B':
    return emit_words(value, constant, 2)
  elif data_type == '0009': # VISIBLE STRING
    return emit_string(value, constant, 32)
  elif data_type == '000A': # OCTET STRING
    return emit_string(value, constant, 6)
  else:
    print "unknown size for data type: " + data_type

def emit_var_data(var):

  string = ""
  data_type = var.data_type

  if var.actual_value == None:
    var.actual_value = var.default_value
  if var.default_value == None:
    var.actual_value = var.default_value
  if var.actual_value == None:
    var.actual_value = get_empty_value(data_type);

 

  num_bytes_required_per_entry = 0
  if data_type in data_size_lu:
    num_bytes_required_per_entry = data_size_lu[data_type]
  else:
    
    num_bytes_required_per_entry = 4

  #this must exist
  pointer = emit_data(var.actual_value, data_type, False)

  mask = 0

  if var.default_value != None:
    emit_data(var.default_value, data_type, True)
    mask += 1

  if var.low_limit != None:
    emit_data(var.low_limit, data_type, True)
    mask += 2

  if var.high_limit != None:
    emit_data(var.high_limit, data_type, True)
    mask += 4

  string += "{"
  string += pointer + ", "
  string += str(num_bytes_required_per_entry) + ", "
  string += str(mask) + ", "
  if var.access_type == None:
    string += "access_const, "
  else:
    string += "access_" + var.access_type + ", "

  if var.PDO_mapping  == None:
    string += "pdo_no, "
  else:
    string += "pdo_"+(var.PDO_mapping).lower() + ", "

  string += "0x" +str(var.data_type)

  string += "}"
  return string

def get_empty_value(data_type):  
  if data_type == '0001': # boolean
    return "false"
  elif data_type == '0009' or data_type == '000A' or data_type == '000B': #strings
    return ""
  else:
    return "0"


#emit a var and return where it was put
def emit_var_into_var_table(var):
  global var_string
  global var_count
  var_string += "\t"
  var_string += emit_var_data(var)
  var_string += ",\n"
  var_count+=1
  return "vars + " + str(var_count-1)


for obj in object_table:
  object_code = obj[2]
  object_defines += "#define " + obj[1] + " 0x" + obj[0] +"\n"

  if object_code == 7:
    object_index = obj[0]
    var_name = obj[1]
    object_code = obj[2]

    var_loc = emit_var_into_var_table(obj[3])
    name_loc = emit_string(var_name, True, 0)

    object_string += "\t{ " + var_loc +", " + name_loc + ", " + str(object_code) + ", 0x" + object_index + "},\n"
    object_count += 1
  
  if object_code == 8 :
    object_index = obj[0]
    array_name = obj[1]
    #object_code = obj[2]
    array_data_type = obj[3]
    array_access = obj[4]
    array_default_value = obj[5]
    array_pdo_mappable = obj[6]
    array_low_limit = obj[7]
    array_high_limit = obj[8]
    array_sub_indicies = obj[9]

    array_name_location = emit_string(array_name, True, 0)

    object_string += "\t{ arrays + " + str(array_count) +", " + array_name_location + ", " + str(object_code) + ", 0x" + object_index + "},\n"
    object_count += 1

    if len(array_sub_indicies) == 0:
      print "ERROR: array with no subindicies"

    if array_data_type not in data_size_lu:
      print "ERROR: array with varaible size elements"

    number_of_entries = emit_var_into_var_table(array_sub_indicies[0])

    if array_sub_indicies[1].actual_value == None:
      array_sub_indicies[1].actual_value = get_empty_value(array_data_type);

    data_loc = emit_data(array_sub_indicies[1].actual_value, array_data_type, False)

    for asi in range(2, len(array_sub_indicies)):
      if array_sub_indicies[asi].actual_value == None:
        array_sub_indicies[asi].actual_value =  get_empty_value(array_data_type);
      emit_data(array_sub_indicies[asi].actual_value, array_data_type, False)

    #assume all have the same property for:
    #  pdo mappable
    #  limits
    #  default value
    #  access

    num_bytes_required_per_entry = 0

    if array_data_type in data_size_lu:
      num_bytes_required_per_entry = data_size_lu[array_data_type]
    else:
      num_bytes_required_per_entry = 4
    mask = 0

    if array_default_value != None:
      emit_data(array_default_value, array_data_type, True)
      mask += 1

    if array_low_limit != None:
      emit_data(array_low_limit, array_data_type, True)
      mask += 2

    if array_high_limit != None:
      emit_data(array_high_limit, array_data_type, True)
      mask += 4

    if array_access == None:
      array_access = "const"
    if array_pdo_mappable  == None:
      array_pdo_mappable = "no"


    array_string += "\t{ " + number_of_entries + ", " + data_loc + ", "
    array_string += str(len(array_sub_indicies)) + ", "
    array_string += str(mask) + ", "
    array_string += "access_" + array_access + ", "
    array_string += "pdo_"+ (array_pdo_mappable).lower() + ", "
    array_string += " 0x" +str(array_data_type) + ", "
    array_string += str(num_bytes_required_per_entry) + ", " 
    array_string += "},\n"

    array_count+=1

  if object_code == 9:
    object_index = obj[0]
    record_name = obj[1]
    #object_code = obj[2]
    record_subindexs = obj[3]

    record_name_location = emit_string(record_name, True, 0)

    object_count+=1
    object_string += "\t{ records + " + str(record_count) +", " + record_name_location +  ", " + str(object_code) + ", 0x" + object_index + "},\n"

    record_string += "\t{ record_si + " + str(record_si_count) + ", "+ str(len(record_subindexs))+" },\n"

   # print array_sis
    index = 0
    if len(record_subindexs) == 0:
      print "ERROR: array with no subindicies"


    for si in record_subindexs:
      object_defines += "#define " + obj[1] +"_" + record_subindexs[si].name + " " + str(si) +"\n"
      record_si_string += "\t{" + emit_var_data(record_subindexs[si]) + ", " + emit_string(record_subindexs[si].name, True, 0) +"},\n"
      record_si_count += 1


    record_count+=1

name_table_string += "};\n"
var_string += "};\n"
array_string += "};\n"
record_string += "};\n"
object_string += "};\n"
record_si_string += "};\n"

file_object_defines = open("object_dictionary_defines.h",'w')
file_object_defines.write("#ifndef OBJECT_DICTIONARY_DEFINES_H_\n")
file_object_defines.write("#define OBJECT_DICTIONARY_DEFINES_H_\n\n")
file_object_defines.write("// THIS FILE IS GENERATED - DO NOT EDIT\n\n")
file_object_defines.write(object_defines)
file_object_defines.write("#endif\n")
file_object_defines.close()


file_header = open("object_dictionary.h",'w')
file_header.write("#ifndef OBJECT_DICTIONARY_H_\n")
file_header.write("#define OBJECT_DICTIONARY_H_\n\n")
file_header.write("#include \"can_open_od_structs.h\"\n\n")
file_header.write("// THIS FILE IS GENERATED - DO NOT EDIT\n\n")

file_header.write("#define NAME_TABLE_SIZE " + str(name_table_char_count) + "\n");
file_header.write("#define NUM_VARS " + str(var_count) + "\n");
file_header.write("#define NUM_ARRAYS " + str(array_count) + "\n");
file_header.write("#define NUM_RECORDS " + str(record_count) + "\n");
file_header.write("#define NUM_RECORD_SIS " + str(record_si_count) + "\n");
file_header.write("#define NUM_OBJECTS " + str(object_count) + "\n");

file_header.write("\n");

file_header.write( "extern char byte_table[];\n");
file_header.write( "extern short short_table[];\n");
file_header.write( "extern unsigned word_table[];\n");

bytes = (byte_string_count + short_string_count*2 + word_string_count*4)
bytes += var_count*8 + array_count*16 + record_count*8 + record_si_count*12 + object_count*16

file_header.write("//Data bytes: " + str(bytes) + "\n");

file_header.write("\n");

file_header.write("extern char name_table[NAME_TABLE_SIZE];\n");
file_header.write("extern var_t vars[NUM_VARS];\n");
file_header.write("extern array_t arrays[NUM_ARRAYS];\n");
file_header.write("extern record_subindex_t record_si[NUM_RECORD_SIS];\n");
file_header.write("extern record_t records[NUM_RECORDS];\n");
file_header.write("extern object_t objects[NUM_OBJECTS];\n");

file_header.write("#endif\n")
file_header.close()


file_source = open("object_dictionary.c",'w')
file_source.write("#include \"object_dictionary.h\"\n\n")
file_source.write("// THIS FILE IS GENERATED - DO NOT EDIT\n\n")

file_source.write( "char byte_table[] = {\n\t" + byte_string + "};\n\n");
file_source.write( "short short_table[] = {\n\t" + short_string + "};\n\n");
file_source.write( "unsigned word_table[] = {\n\t" + word_string + "};\n\n");

file_source.write(name_table_string + "\n\n");
file_source.write(var_string + "\n");
file_source.write(array_string + "\n");
file_source.write(record_si_string + "\n");
file_source.write(record_string + "\n");
file_source.write(object_string + "\n");

file_source.close()











