## --------- XMOS Ethernet PowerLink Automated Test Mode ---------- ##
import Tkinter as tk
from Tkinter import *
from Tkinter import Label
import tkMessageBox
from Tkinter import Checkbutton
import os
from mii_frame_gen import *
import time

#------------------------------------- Label Frame --------------------------------------------------------------------------------------
def automated_test():
    automated = tk.Tk()
    automated.title('Automated Testing Mode')
    automated.geometry("300x215")
    automated_labelframe = tk.LabelFrame(automated, text="Choose the Test Pattern(s)")
    automated_labelframe.pack(fill="both", expand="yes")    

    #------------------------------------- StartButton CallBack -----------------------------------------------------------------------------
    def startbutton_callback():
        
        if((pattern1.get() or pattern2.get() or pattern3.get()) == 0):
            tkMessageBox.showerror("Automated Testing Mode", "ERROR: No Pattern Chosen to Start")
            automated.destroy()

        if(int(timeentry.get()) > 5000):
            tkMessageBox.showerror("Automated Testing Mode", "ERROR: Frame Send Time - Out of Range \n Range: 0 to 5000 mSec")
            automated.destroy()

        timedelay = int(timeentry.get())
        checksum = 0
        byte_cnt = 64
        if(pattern1.get() == 1):
            while(byte_cnt <= 1522):
                checksum = send_frame_to_host(1,1,byte_cnt,0,timedelay)
                byte_cnt += 1

        byte_cnt = 64
        if(pattern2.get() == 1):
            while(byte_cnt <= 1522):
                send_frame_to_host(1,2,byte_cnt,0,timedelay)
                byte_cnt += 1

        byte_cnt = 64
        if(pattern3.get() == 1):
            while(byte_cnt <= 1522):
                send_frame_to_host(1,4,byte_cnt,0,timedelay)
                byte_cnt += 1
                
    #------------------------------------- CancelButton CallBack ----------------------------------------------------------------------------
    def cancelbutton_callback():
        automated.destroy()

    #------------------------------------- CheckButtons Callback ----------------------------------------------------------------------------        
    def pattern1_callback():
        pattern1.set(not pattern1.get())
        selectall_pattern.deselect()    #Deselect SelectAll Checkbox when any 1 pattern is selected or deselected
        all_pattern.set(0)         #Set all_pattern value as 0
        clearall_pattern.deselect()

    def pattern2_callback():
        pattern2.set(not pattern2.get())
        selectall_pattern.deselect()    #Deselect SelectAll Checkbox when any 1 pattern is selected or deselected
        all_pattern.set(0)         #Set all_pattern value as 0
        clearall_pattern.deselect()
        
    def pattern3_callback():
        pattern3.set(not pattern3.get())
        selectall_pattern.deselect()    #Deselect SelectAll Checkbox when any 1 pattern is selected or deselected
        all_pattern.set(0)         #Set all_pattern value as 0
        clearall_pattern.deselect()
        
    def selectall_pattern_callback():
        all_pattern.set(not all_pattern.get())
        clearall_pattern.deselect()
        if (all_pattern.get() != 0):
            patterntype_1.select()
            pattern1.set(1)
            patterntype_2.select()
            pattern2.set(1)
            patterntype_3.select()
            pattern3.set(1)
        else:
            patterntype_1.deselect()
            pattern1.set(0)
            patterntype_2.deselect()
            pattern2.set(0)
            patterntype_3.deselect()
            pattern3.set(0)

    def clearall_pattern_callback():
        all_pattern.set(0)
        patterntype_1.deselect()
        pattern1.set(0)
        patterntype_2.deselect()
        pattern2.set(0)
        patterntype_3.deselect()
        pattern3.set(0)
        selectall_pattern.deselect()

    #------------------------------------- Check Buttons -------------------------------------------------------------------------------------   
    pattern1 = IntVar()
    pattern2 = IntVar()
    pattern3 = IntVar()
    all_pattern = IntVar()
    clear_all = IntVar()
    time_delay = IntVar()

    patterntype_1 = tk.Checkbutton(automated,text="0xAA", variable=pattern1, onvalue = 1, offvalue = 0,command=pattern1_callback)
    patterntype_1.pack()
    patterntype_1.place(x=10,y=30)
    patterntype_2 = tk.Checkbutton(automated,text="0x55", variable=pattern2,command=pattern2_callback)
    patterntype_2.pack()
    patterntype_2.place(x=10,y=65)
    patterntype_3 = tk.Checkbutton(automated,text="0x01, 0x02, 0x03,...0xFF", variable=pattern3, command=pattern3_callback)
    patterntype_3.pack()
    patterntype_3.place(x=10,y=100)
    selectall_pattern = tk.Checkbutton(automated,text="Select All",variable=all_pattern, command=selectall_pattern_callback)
    selectall_pattern.pack()
    selectall_pattern.place(x=10,y=135) 
    clearall_pattern = tk.Checkbutton(automated,text="Clear All",variable=clear_all, command=clearall_pattern_callback)
    clearall_pattern.pack()
    clearall_pattern.place(x=95,y=135)

    #------------------------------------- Entry ----------------------------------------------------------------------------------------
    label_entry = tk.Label(automated,text="Frame Send Time:  in mS").place(x=10, y=175)
    timeentry = tk.Entry(automated,bd=4,textvariable=time_delay,justify=RIGHT, width=6)
    timeentry.pack()
    timeentry.place(x=160, y=175)
    timeentry.insert(0,"0") #to show the default value

    #------------------------------------- Buttons --------------------------------------------------------------------------------------
    automated_start = tk.Button(automated, text = " Start ", height=2,width=7,bd=4, command=startbutton_callback)
    automated_start.pack()
    automated_start.place(x=215,y=32)

    automated_cancel = tk.Button(automated, text = " Cancel ", height=2,width=7,bd=4, command=cancelbutton_callback)
    automated_cancel.pack()
    automated_cancel.place(x=215,y=110)

#----------------------------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    automated_test()
