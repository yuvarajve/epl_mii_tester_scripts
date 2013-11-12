import Tkinter as tk
from Tkinter import Label
import tkMessageBox
from Tkinter import Checkbutton
from Tkinter import *
from od_entries import *
from od_data import *
from datetime import datetime
import os

#---------------------------------
#Data Types Suppoted in CANOpen
#---------------------------------
# bool 		- 0x0001
# domain        - 0x000f
# int16 	       - 0x0003
#int24		- 0x0010
#int32		- 0x0004
#int40		- 0x0012
#int48		- 0x0013
#int56		- 0x0014
#int64		- 0x0015
#int8		- 0x0002
#octec string	  - 0x000A
#Real32		- 0x0008
#Real64		- 0x0011
#time_diff	     - 0x000d
#time_of_day	   - 0x000c
#unicode string - 0x000b
#unsigned16	    - 0x0006
#unsigned24	    - 0x0016
#unsigned32	    - 0x0007
#unsigned40	    - 0x0018
#unsigned48	    - 0x0019
#unsigned56	    - 0x001A
#unsigned64	    - 0x001B
#unsigned8	     - 0x0005
#Vis string	    - 0x0009
#----------------------------------

selected_index = []
FILE_info = [' ', ' ', ' ', ' ', ' ']
DEVICE_info = [' ', ' ', ' ', ' ', ' ', ' ']
PDO_info = [' ', ' ']
BAUD_info = ['1','1','1','1','1','1','1','1','1']
MANDITORY_OBJ = ['1000','1001','1018']

LUT = {}
LUT['RO'] ='ro'
LUT['RW'] ='rw'
LUT['WO'] ='wo'
LUT['RWR'] ='rwr'
LUT['RWW'] ='rww'
LUT['CONST'] ='const'
LUT[' '] =' '
LUT['BOOLEAN'] = '0x0001'
LUT['DOMAIN'] = '0x000f'
LUT['INTEGER16'] = '0x0003'
LUT['INTEGER32'] = '0x0004'
LUT['INTEGER8'] = '0x0002'
LUT['VARIABLE_STRING'] = '0x0009'
LUT['UNSIGNED8'] = '0x0005'
LUT['UNSIGNED16'] = '0x0006'
LUT['UNSIGNED32'] = '0x0007'
LUT['REAL32'] = '0x0008'
LUT['OCTET_STRING'] = '0x000A'
LUT['DOMAIN'] = '0x2'
LUT['VAR'] = '0x7'
LUT['ARRAY'] = '0x8'
LUT['RECORD'] = '0x9'

   
def GenerateFiles():
   temp_list = []
   for i in selected_index:
      temp_list.append(int(i,16))
   sorted_list = sorted(temp_list, key=int)
   for i in range(0,len(sorted_list)):
       sorted_list[i] = str((hex(sorted_list[i]).split('x')[1]).upper())
   FILE_NAME = FILE_info[0]
   dtn = datetime.now()
   fp = open(FILE_NAME,'w')
   fp.writelines('[FileInfo]\n')
   fp.writelines('CreatedBy='+FILE_info[4]+'\n')
   fp.writelines('ModifieddBy='+FILE_info[4]+'\n')
   fp.writelines('Description='+FILE_info[3]+'\n')
   fp.writelines('CreationTime='+datetime.strftime(dtn,'%I:%M%p')+'\n')
   fp.writelines('CreationDate='+datetime.strftime(dtn,'%m-%d-%Y') +'\n')
   fp.writelines('ModificationTime='+datetime.strftime(dtn,'%I:%M%p')+'\n')
   fp.writelines('ModificationDate='+datetime.strftime(dtn,'%m-%d-%Y') +'\n')
   fp.writelines('FileName='+FILE_NAME +'\n')
   fp.writelines('FileVersion='+FILE_info[1] +'\n')
   fp.writelines('FileRevision='+FILE_info[2] +'\n')
   fp.writelines('EDSVersion=V1\n\n[DeviceInfo]\n')
   fp.writelines('VendorName='+DEVICE_info[0] +'\n')
   fp.writelines('VendorNumber='+DEVICE_info[1] +'\n')
   fp.writelines('ProductName='+DEVICE_info[2] +'\n')
   fp.writelines('ProductNumber='+DEVICE_info[3] +'\n')
   fp.writelines('RevisionNumber='+DEVICE_info[4] +'\n')
   fp.writelines('OrderCode='+DEVICE_info[5] +'\n')
   fp.writelines('BaudRate_10='+BAUD_info[0] +'\n')
   fp.writelines('BaudRate_20='+BAUD_info[1] +'\n')
   fp.writelines('BaudRate_50='+BAUD_info[2] +'\n')
   fp.writelines('BaudRate_100='+BAUD_info[3] +'\n')
   fp.writelines('BaudRate_125='+BAUD_info[4] +'\n')
   fp.writelines('BaudRate_250='+BAUD_info[5] +'\n')
   fp.writelines('BaudRate_500='+BAUD_info[6] +'\n')
   fp.writelines('BaudRate_800='+BAUD_info[7] +'\n')
   fp.writelines('BaudRate_1000='+BAUD_info[8] +'\n')
   fp.writelines('SimpleBootUpMaster=0\nSimpleBootUpSlave=1\nGranularity=0\nDynamicChannelsSupported=0\nCompactPDO=0\nGroupMessaging=0\n')
   fp.writelines('NrOfRXPDO='+PDO_info[1]+'\nNrOfTXPDO='+PDO_info[0]+'\nLSS_Supported=0\n\n')
   fp.writelines('[DummyUsage]\nDummy0001=1\nDummy0002=1\nDummy0003=1\nDummy0004=1\nDummy0005=1\nDummy0006=1\nDummy0007=1\n\n[MandatoryObjects]\n')
   no_of_man_obj = 0
   for obj in MANDITORY_OBJ:
      try:
         sorted_list.index(obj)
         no_of_man_obj+=1
      except:
         no_of_man_obj = no_of_man_obj
   fp.writelines('SupportedObjects='+str(no_of_man_obj)+'\n')
   no_of_man_obj = 0
   for obj in MANDITORY_OBJ:
      try:
         sorted_list.index(obj)
         no_of_man_obj+=1
         fp.writelines(str(no_of_man_obj)+'=0x'+obj+'\n')
      except:
         no_of_man_obj = no_of_man_obj
   for obj in MANDITORY_OBJ:
      try:
         sorted_list.index(obj)
         fp.writelines('\n['+obj+']\n')
         fp.writelines('ParameterName='+index_list[obj][7]+'\n')
         fp.writelines('ObjectType='+LUT[index_list[obj][0]]+'\n')
         if( not((index_list[obj][6] != ' ') and (index_list[obj][6] != '0'))):
            fp.writelines('DataType='+LUT[index_list[obj][1]]+'\n')
         if( not((index_list[obj][6] != ' ') and (index_list[obj][6] != '0'))):
            fp.writelines('AccessType='+LUT[index_list[obj][2]]+'\n')
         if(index_list[obj][3] != ' '):
            fp.writelines('DefaultValue='+index_list[obj][3]+'\n')
         if(index_list[obj][4] != ' '):
            fp.writelines('LowLimit='+index_list[obj][4]+'\n')
         if(index_list[obj][5] != ' '):
            fp.writelines('HighLimit='+index_list[obj][5]+'\n')
         fp.writelines('PDOMapping=0\n')
         if( (index_list[obj][6] != ' ') and (index_list[obj][6] != '0')):
            fp.writelines('SubNumber='+index_list[obj][6]+'\n')
            max_value = int(index_list[obj][6])
            i_value=0
            while(i_value != max_value):
               ind =  obj+'sub'+str(i_value)
               fp.writelines('\n['+ind+']\n')
               fp.writelines('ParameterName='+sub_list[ind][6]+'\n')
               fp.writelines('ObjectType='+LUT[sub_list[ind][0]]+'\n')
               fp.writelines('DataType='+LUT[sub_list[ind][1]]+'\n')
               fp.writelines('AccessType='+LUT[sub_list[ind][2]]+'\n') 
               if(sub_list[ind][3] != ' '):
                  fp.writelines('DefaultValue='+sub_list[ind][3]+'\n')
               if(sub_list[ind][4] != ' '):
                  fp.writelines('LowLimit='+sub_list[ind][4]+'\n')
               if(sub_list[ind][5] != ' '):
                  fp.writelines('HighLimit='+sub_list[ind][5]+'\n')
               fp.writelines('PDOMapping=0\n')
               i_value+=1
         sorted_list.remove(obj)
      except:
         no_of_man_obj = no_of_man_obj
         
   fp.writelines('\n\n[OptionalObjects]\n')
   fp.writelines('SupportedObjects='+str(len(sorted_list))+'\n')
   for i in range(0,len(sorted_list)):
      fp.writelines(str(i+1)+'=0x'+str(sorted_list[i])+'\n')
   for obj in sorted_list:
      try:
         sorted_list.index(obj)
         fp.writelines('\n['+obj+']\n')
         fp.writelines('ParameterName='+index_list[obj][7]+'\n')
         fp.writelines('ObjectType='+LUT[index_list[obj][0]]+'\n')
         if( not((index_list[obj][6] != ' ') and (index_list[obj][6] != '0'))):
            fp.writelines('DataType='+LUT[index_list[obj][1]]+'\n')
         if( not((index_list[obj][6] != ' ') and (index_list[obj][6] != '0'))):
            fp.writelines('AccessType='+LUT[index_list[obj][2]]+'\n')
         if(index_list[obj][3] != ' '):
            fp.writelines('DefaultValue='+index_list[obj][3]+'\n')
         if(index_list[obj][4] != ' '):
            fp.writelines('LowLimit='+index_list[obj][4]+'\n')
         if(index_list[obj][5] != ' '):
            fp.writelines('HighLimit='+index_list[obj][5]+'\n')
         fp.writelines('PDOMapping=0\n')
         if( (index_list[obj][6] != ' ') and (index_list[obj][6] != '0')):
            fp.writelines('SubNumber='+index_list[obj][6]+'\n')
            max_value = int(index_list[obj][6])
            i_value=0
            while(i_value != max_value):
               ind =  obj+'sub'+str(hex(i_value).split('x')[1])
               fp.writelines('\n['+ind+']\n')
               fp.writelines('ParameterName='+sub_list[ind][6]+'\n')
               fp.writelines('ObjectType='+LUT[sub_list[ind][0]]+'\n')
               fp.writelines('DataType='+LUT[sub_list[ind][1]]+'\n')
               fp.writelines('AccessType='+LUT[sub_list[ind][2]]+'\n')
               if(sub_list[ind][3] != ' '):
                  fp.writelines('DefaultValue='+sub_list[ind][3]+'\n')
               if(sub_list[ind][4] != ' '):
                  fp.writelines('LowLimit='+sub_list[ind][4]+'\n')
               if(sub_list[ind][5] != ' '):
                  fp.writelines('HighLimit='+sub_list[ind][5]+'\n')
               fp.writelines('PDOMapping=0\n')
               i_value+=1
      except:
         no_of_man_obj = no_of_man_obj
   fp.writelines('\n[ManufacturerObjects]\nSupportedObjects=0\n')
   fp.close()
   tkMessageBox.showinfo("Info", "Generated 'EDS' File")
   exit(0)

def FileInfo():
   top = tk.Tk()
   top.title("File and Device Info")
   checkCmd1 = IntVar()
   checkCmd2 = IntVar()
   checkCmd3 = IntVar()
   checkCmd4 = IntVar()
   checkCmd5 = IntVar()
   checkCmd6 = IntVar()
   checkCmd7 = IntVar()
   checkCmd8 = IntVar()
   checkCmd9 = IntVar()
   w = 1050
   h = 320
   x = 10
   y = 50
   
   def cbcommand1():
      checkCmd1.set(not checkCmd1.get())
   def cbcommand2():
      checkCmd2.set(not checkCmd2.get())
   def cbcommand3():
      checkCmd3.set(not checkCmd3.get())
   def cbcommand4():
      checkCmd4.set(not checkCmd4.get())
   def cbcommand5():
      checkCmd5.set(not checkCmd5.get())
   def cbcommand6():
      checkCmd6.set(not checkCmd6.get())
   def cbcommand7():
      checkCmd7.set(not checkCmd7.get())
   def cbcommand8():
      checkCmd8.set(not checkCmd8.get())
   def cbcommand9():
      checkCmd9.set(not checkCmd9.get())

   def ADD_PDOCallBack():
      if((TPDO.get() == '') or (RPDO.get() == '')):
         tkMessageBox.showinfo("Info", "Invalid PDO information !!")
      elif((int(TPDO.get()) < 4) or (int(RPDO.get()) < 4)):
         tkMessageBox.showinfo("Info", "Minimum of 4 PDOs should be added")
      elif( (int(TPDO.get()) > 15) or (int(RPDO.get()) > 15) ):
         tkMessageBox.showinfo("Info", "Maximum of 15 PDOs only supported by this tool")
      else:
         for i in range(0,int(TPDO.get())):
            selected_index.append('180'+str(hex(i).split('x')[1]).upper())
            lb1.insert("end", '180'+str(hex(i).split('x')[1]).upper()+'  Transmit PDO Communication Parameter '+str(i))
         for i in range(0,int(TPDO.get())):
            selected_index.append('1A0'+str(hex(i).split('x')[1]).upper())
            lb1.insert("end", '1A0'+str(hex(i).split('x')[1]).upper()+'  Transmit PDO Mapping Parameter '+str(i))
         for i in range(0,int(RPDO.get())):
            selected_index.append('140'+str(hex(i).split('x')[1]).upper())
            lb1.insert("end", '140'+str(hex(i).split('x')[1]).upper()+'  Receive PDO Communication Parameter '+str(i))
         for i in range(0,int(RPDO.get())):
            selected_index.append('160'+str(hex(i).split('x')[1]).upper())
            lb1.insert("end", '160'+str(hex(i).split('x')[1]).upper()+'  Receive PDO Mapping Parameter '+str(i))
         tkMessageBox.showinfo("Info", "PDOs Added")
      
   top.geometry("%dx%d+%d+%d" % (w, h, x, y))
   group1 = tk.LabelFrame(top, text="File Information").grid(row=0, columnspan=7, sticky='W',padx=5, pady=5, ipadx=140, ipady=120)
   Label(top,text="File Name : ").place(x=20,y=70)
   FileName = tk.Entry(top, width=25)
   FileName.pack()
   FileName.place(x=110,y=70)
   FileName.insert(0,"CANopen.eds")
   Label(top,text="File Version : ").place(x=20,y=100)
   FileVersion = tk.Entry(top, width=25)
   FileVersion.pack()
   FileVersion.place(x=110,y=100)
   Label(top,text="File Revision : ").place(x=20,y=130)
   FileRevision = tk.Entry(top, width=25)
   FileRevision.pack()
   FileRevision.place(x=110,y=130)
   Label(top,text="Description : ").place(x=20,y=160)
   Description = tk.Entry(top, width=25)
   Description.pack()
   Description.place(x=110,y=160)
   Label(top,text="Author : ").place(x=20,y=190)
   Author = tk.Entry(top, width=25)
   Author.pack()
   Author.place(x=110,y=190)

   group2 = tk.LabelFrame(top, text="Device Information").grid(row=0, columnspan=7, sticky='W',padx=305, pady=5, ipadx=150, ipady=120)
   Label(top,text="Vendor Name : ").place(x=320,y=70)
   VendorName = tk.Entry(top, width=25)
   VendorName.pack()
   VendorName.place(x=430,y=70)
   Label(top,text="Vendor Number : ").place(x=320,y=100)
   VendorNumber = tk.Entry(top, width=25)
   VendorNumber.pack()
   VendorNumber.place(x=430,y=100)
   Label(top,text="Product Name : ").place(x=320,y=130)
   ProductName = tk.Entry(top, width=25)
   ProductName.pack()
   ProductName.place(x=430,y=130)
   Label(top,text="Product Number : ").place(x=320,y=160)
   ProductNumber = tk.Entry(top, width=25)
   ProductNumber.pack()
   ProductNumber.place(x=430,y=160)
   Label(top,text="Revision Number : ").place(x=320,y=190)
   RevisionNumber = tk.Entry(top, width=25)
   RevisionNumber.pack()
   RevisionNumber.place(x=430,y=190)
   Label(top,text="Order Code : ").place(x=320,y=220)
   OrderCode = tk.Entry(top, width=25)
   OrderCode.pack()
   OrderCode.place(x=430,y=220)

   group3 = tk.LabelFrame(top, text="Supported Bit Rate").grid(row=0, columnspan=7, sticky='W',padx=650, pady=5, ipadx=80, ipady=150)
   checkBox1 = Checkbutton(top, variable=checkCmd1, text="10k Bit/Sec", command=cbcommand1)
   checkBox1.pack()
   checkBox1.place(x=700,y=20)
   checkBox2 = Checkbutton(top, variable=checkCmd2, text="20k Bit/Sec", command=cbcommand2)
   checkBox2.pack()
   checkBox2.place(x=700,y=50)
   checkBox3 = Checkbutton(top, variable=checkCmd3, text="50k Bit/Sec", command=cbcommand3)
   checkBox3.pack()
   checkBox3.place(x=700,y=80)
   checkBox4 = Checkbutton(top, variable=checkCmd4, text="100k Bit/Sec", command=cbcommand4)
   checkBox4.pack()
   checkBox4.place(x=700,y=110)
   checkBox5 = Checkbutton(top, variable=checkCmd5, text="125k Bit/Sec", command=cbcommand5)
   checkBox5.pack()
   checkBox5.place(x=700,y=140)
   checkBox6 = Checkbutton(top, variable=checkCmd6, text="250k Bit/Sec", command=cbcommand6)
   checkBox6.pack()
   checkBox6.place(x=700,y=170)
   checkBox7 = Checkbutton(top, variable=checkCmd7, text="500k Bit/Sec", command=cbcommand7)
   checkBox7.pack()
   checkBox7.place(x=700,y=200)
   checkBox8 = Checkbutton(top, variable=checkCmd8, text="800k Bit/Sec", command=cbcommand8)
   checkBox8.pack()
   checkBox8.place(x=700,y=230)
   checkBox9 = Checkbutton(top, variable=checkCmd9, text="1000k Bit/Sec", command=cbcommand9)
   checkBox9.pack()
   checkBox9.place(x=700,y=260)

   group4 = tk.LabelFrame(top, text="Number of PDOs").grid(row=0, columnspan=7, sticky='W',padx=850, pady=5, ipadx=80, ipady=80)
   Label(top,text="Transmit PDOs").place(x=890,y=100)
   TPDO = tk.Entry(top, width=10)
   TPDO.pack()
   TPDO.place(x=900,y=120)
   Label(top,text="Receive PDOs").place(x=890,y=170)
   RPDO = tk.Entry(top, width=10)
   RPDO.pack()
   RPDO.place(x=900,y=190)

   ADD = tk.Button(top, text ="Add PDOs", command = ADD_PDOCallBack)
   ADD.pack()
   ADD.place(x=900,y=250)

   def SaveClose():
      FILE_info[0] = (FileName.get())
      FILE_info[1] = (FileVersion.get())
      FILE_info[2] = (FileRevision.get())
      FILE_info[3] = (Description.get())
      FILE_info[4] = (Author.get())
      DEVICE_info[0] = (VendorName.get())
      DEVICE_info[1] = (VendorNumber.get())
      DEVICE_info[2] = (ProductName.get())
      DEVICE_info[3] = (ProductNumber.get())
      DEVICE_info[4] = (RevisionNumber.get())
      DEVICE_info[5] = (OrderCode.get())
      PDO_info[0] = TPDO.get()
      PDO_info[1] = RPDO.get()
      BAUD_info[0] = str(checkCmd1.get())
      BAUD_info[1] = str(checkCmd2.get())
      BAUD_info[2] = str(checkCmd3.get())
      BAUD_info[3] = str(checkCmd4.get())
      BAUD_info[4] = str(checkCmd5.get())
      BAUD_info[5] = str(checkCmd6.get())
      BAUD_info[6] = str(checkCmd7.get())
      BAUD_info[7] = str(checkCmd8.get())
      BAUD_info[8] = str(checkCmd9.get())
      tkMessageBox.showinfo("Info", "Closing File and Device Info Window")
      top.destroy()
   
   SaveButton = tk.Button(top, text="Save & Close", command=SaveClose)
   SaveButton.place(x=330,y=280)
   top.mainloop()      
   
def ADDCallBack():
   name = lb.get("active")
   try:
     selected_index.index(name[0:4])
     tkMessageBox.showinfo("Error", "Index already added !!")
   except:
      selected_index.append(name[0:4])
      lb1.insert("end", name)

def EXITCallBack():
   exit(0)

def HelpCallback():
   os.startfile('Release_Notes.txt')

def AboutCallback():
   tkMessageBox.showinfo("Info", "EDS Genarator tool for XMOS Semiconductor. \n\n\tVersion 1.0")

def on_listbox_select(event):
    lb2.delete(0,"end")
   
def DeleteObject():
   name = lb1.get("active")
   try:
      lb1.delete(selected_index.index(name[0:4]),selected_index.index(name[0:4]))
      selected_index.remove(name[0:4])
   except:
      tkMessageBox.showinfo("Info", "No Objects to Delete")
   
def callback():
   try:
     index_list[Index.get()][0] = Otype.get()
     index_list[Index.get()][1] = Dtype.get()
     index_list[Index.get()][2] = Atype.get()
     index_list[Index.get()][3] = Default_value.get()
     index_list[Index.get()][4] = Low_value.get()
     index_list[Index.get()][5] = High_value.get()
     index_list[Index.get()][6] = SUB_value.get()
     lb2.delete(0,"end")
   except:
     sub_list[Index.get()][0] = Otype.get()
     sub_list[Index.get()][1] = Dtype.get()
     sub_list[Index.get()][2] = Atype.get()
     sub_list[Index.get()][3] = Default_value.get()
     sub_list[Index.get()][4] = Low_value.get()
     sub_list[Index.get()][5] = High_value.get()
    
def on_listbox1_select(event):
    lb2.delete(0,"end")
    name = lb1.get("active")
    w.delete(0,'end')
    Index.delete(0,'end')
    Otype.delete(0,'end')
    Dtype.delete(0,'end')
    Atype.delete(0,'end')
    Default_value.delete(0,'end')
    Low_value.delete(0,'end')
    High_value.delete(0,'end')
    SUB_value.delete(0,'end') 
    w.insert(0,name[5:])
    Index.insert(0,name[:4])
    Otype.insert(0,index_list[name[:4]][0])
    Dtype.insert(0,index_list[name[:4]][1])
    Atype.insert(0,index_list[name[:4]][2])
    Default_value.insert(0,index_list[name[:4]][3])
    Low_value.insert(0,index_list[name[:4]][4])
    High_value.insert(0,index_list[name[:4]][5])
    SUB_value.insert(0,index_list[name[:4]][6])
    if(index_list[name[:4]][6] != 0 ):
       for i in range(0,int(index_list[name[:4]][6])):
          lb2.insert("end",name[:4]+"sub"+str(hex(i).split('x')[1]))
    else:
       lb2.delete(0,"end")

def on_listbox2_select(event):
    name = lb2.get("active")
    w.delete(0,'end')
    Index.delete(0,'end')
    Otype.delete(0,'end')
    Dtype.delete(0,'end')
    Atype.delete(0,'end')
    Default_value.delete(0,'end')
    Low_value.delete(0,'end')
    High_value.delete(0,'end')
    SUB_value.delete(0,'end')
    w.delete(0,'end')
    w.insert(0,sub_list[name][6])
    Index.insert(0,name)
    Otype.insert(0,sub_list[name][0])
    Dtype.insert(0,sub_list[name][1])
    Atype.insert(0,sub_list[name][2])
    Default_value.insert(0,sub_list[name][3])
    Low_value.insert(0,sub_list[name][4])
    High_value.insert(0,sub_list[name][5])
    
root = tk.Tk()
root.title("XMOS Semiconductor : CANopen EDS Generator Tool")
root.resizable(width=TRUE, height=TRUE)
root.geometry("1500x600")

menubar = Menu(root)
filemenu = Menu(menubar, tearoff=0)
menubar.add_cascade(label="Help", menu=filemenu)
filemenu.add_command(label="Release Notes", command=HelpCallback)
filemenu.add_command(label="About", command=AboutCallback)

logo = PhotoImage(file="xmos.gif")
w1 = Label(root, image=logo)
w1.pack()
w1.place(x=1000,y=1)
lb = tk.Listbox(root, width=50, height=20)
lb.bind("<<ListboxSelect>>", on_listbox_select)
lb.pack(side="left",fill="both", expand=False)
for items in od_list:
    lb.insert("end",items)
lb1 = tk.Listbox(root, width=50, height=20)
lb1.pack(side="left",fill="both", expand=False)
lb1.bind("<<ListboxSelect>>", on_listbox1_select)
lb2 = tk.Listbox(root, width=50, height=20)
lb2.pack(side="left",fill="both", expand=False)
lb2.bind("<<ListboxSelect>>", on_listbox2_select)

Label(root,text="Name : ").place(x=930,y=140)
w = tk.Entry(root, width=50)
w.pack()
w.place(x=990,y=140)
Label(root,text="Index : ").place(x=930,y=170)
Index = tk.Entry(root, width=12)
Index.pack()
Index.place(x=1050,y=170)
Label(root,text="Object Type : ").place(x=930,y=200)
Otype = tk.Entry(root, width=10)
Otype.pack()
Otype.place(x=1050,y=200)
Label(root,text="Data Type : ").place(x=930,y=230)
Dtype = tk.Entry(root, width=20)
Dtype.pack()
Dtype.place(x=1050,y=230)
Label(root,text="Access Type : ").place(x=930,y=260)
Atype = tk.Entry(root, width=6)
Atype.pack()
Atype.place(x=1050,y=260)
Label(root,text="Default Value : ").place(x=930,y=290)
Default_value = tk.Entry(root, width=20)
Default_value.pack()
Default_value.place(x=1050,y=290)
Label(root,text="Lower Limit Value : ").place(x=930,y=320)
Low_value = tk.Entry(root, width=10)
Low_value.pack()
Low_value.place(x=1050,y=320)
Label(root,text="Higher Limit Value : ").place(x=930,y=350)
High_value = tk.Entry(root, width=10)
High_value.pack()
High_value.place(x=1050,y=350)
Label(root,text="Sub Index Entries : ").place(x=930,y=380)
SUB_value = tk.Entry(root, width=6)
SUB_value.pack()
SUB_value.place(x=1050,y=380)

A = tk.Button(root, text ="ADD", command = ADDCallBack)
A.pack()
A.place(x=1050,y=500)
E = tk.Button(root, text ="Exit", command = EXITCallBack)
E.pack()
E.place(x=1100,y=500)
S = tk.Button(root, text ="Save", command = callback)
S.pack()
S.place(x=1150,y=500)
F = tk.Button(root, text ="FileInfo", command = FileInfo)
F.pack()
F.place(x=1200,y=500)
G = tk.Button(root, text ="GenerateFiles", command = GenerateFiles)
G.pack()
G.place(x=1130,y=550)
D = tk.Button(root, text ="Delete", command = DeleteObject)
D.pack()
D.place(x=1270,y=500)

root.config(menu=menubar)
root.mainloop()

