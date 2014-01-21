## --------- XMOS Ethernet PowerLink Manual Test Mode ---------- ##
import Tkinter as tk
from Tkinter import *
from Tkinter import Label
import tkMessageBox
from Tkinter import Checkbutton
import os
from mii_frame_gen import *

#------------------------------------- Label Frame --------------------------------------------------------------------------------------
def manual_test():
    manual = tk.Tk()
    manual.title('manual Testing Mode')
    manual.geometry("400x215")
    manual_labelframe = tk.LabelFrame(manual, text="Choose the Test Pattern")
    manual_labelframe.pack(fill="both", expand="yes")

    pattern_selection = IntVar(manual,value=0)
    user_input = IntVar(manual,value=0)
    time_delay = IntVar(value=0)
    byte_count = IntVar(value=64)

    #------------------------------------- StartButton CallBack -----------------------------------------------------------------------------
    def sendframe_callback(time):
        print 'pattern' + str(pattern_selection.get())

        if(int(pattern_selection.get()) == 0):
            tkMessageBox.showerror("Manual Testing Mode", "ERROR: No Pattern Chosen to Start")
            manual.destroy()
            
        if( (int(datalength.get()) < 64) or (int(datalength.get()) > 1522) ):
            tkMessageBox.showerror("Manual Testing Mode", "ERROR: Number of Bytes to Send - Out of Range \n Range: 64 to 1522 bytes")
            manual.destroy()
        
        if((time == 1) and (int(timeentry.get()) > 5000)):
            tkMessageBox.showerror("Manual Testing Mode", "ERROR: Frame Send Time - Out of Range \n Range: 0 to 5000 mSec")
            manual.destroy()

        if( (int(pattern_selection.get()) == 8) and ((int(userinput.get()) <= 0) or (int(userinput.get()) > 255)) ):
            tkMessageBox.showerror("Manual Testing Mode", "ERROR: User Input - Invalid \n Range: > 0 to <= 255 ")
            print 'user_input ' + str(userinput.get())
            manual.destroy()
        else:
            print 'user_input ' + str(userinput.get())

        checksum = 0
        pattern_type = int(pattern_selection.get())
        userdata = int(userinput.get())
        timedelay = int(timeentry.get())
        nofbytes = int(datalength.get())
        checksum = send_frame_to_host(0,pattern_type,nofbytes,userdata,timedelay)
        
    #------------------------------------- CancelButton CallBack ----------------------------------------------------------------------------
    def cancelbutton_callback():
        manual.destroy()

    #------------------------------------- Radio Callback ----------------------------------------------------------------------------
    def patternselect_callback():  ## Disable UserEntry when other radiobuttons are selected
       userinput = tk.Entry(manual,bd=4,textvariable=user_input,justify=RIGHT, width=6,state=DISABLED)
       userinput.place(x=160, y=115)
   
    #------------------------------------- User Entry ----------------------------------------------------------------------------------------
    def userinput_callback():
        print "You selected the option " + str(pattern_selection.get())
        userinput = tk.Entry(manual,bd=4,textvariable=user_input,justify=RIGHT, width=6)
        userinput.pack()
        userinput.place(x=160, y=115)
        print 'user_input ' + str(int(userinput.get()))

    #------------------------------------- Radio Buttons -------------------------------------------------------------------------------------    
    man_rb1 = tk.Radiobutton(manual, text="0xAA", value=1, variable=pattern_selection, command=patternselect_callback)
    man_rb1.pack()
    man_rb1.place(x=10, y=25)
    man_rb2 = tk.Radiobutton(manual, text="0x55", value=2, variable=pattern_selection, command=patternselect_callback)
    man_rb2.pack()
    man_rb2.place(x=10, y=55)
    man_rb3 = tk.Radiobutton(manual, text="0x01, 0x02, 0x03,...0xFF", value=4, variable=pattern_selection, command=patternselect_callback)
    man_rb3.pack()
    man_rb3.place(x=10, y=85)
    man_rb4 = tk.Radiobutton(manual, text="User Input:          in Dec",value=8, variable=pattern_selection, command=userinput_callback)
    man_rb4.pack()
    man_rb4.place(x=10, y=115)

    #------------------------------------- Entry ----------------------------------------------------------------------------------------
    userinput = tk.Entry(manual,bd=4,textvariable=user_input,justify=RIGHT, width=6,state=DISABLED)
    userinput.place(x=160, y=115)    
    
    label_entry1 = tk.Label(manual,text="Frame Send Time:  in mS").place(x=15, y=147)
    timeentry = tk.Entry(manual,bd=4,textvariable=time_delay,justify=RIGHT, width=6)
    timeentry.place(x=160, y=147)
    timeentry.insert(0,"0")

    label_entry2 = tk.Label(manual,text="Number of Bytes to Send:").place(x=15, y=180)
    datalength = tk.Entry(manual,bd=4,textvariable=byte_count,justify=RIGHT, width=6)
    datalength.place(x=160, y=180)
    datalength.insert(0,"64")
    #------------------------------------- Buttons --------------------------------------------------------------------------------------
    manual_gettime = tk.Button(manual, text = " Get Time", height=1,width=9,bd=4, command=lambda:sendframe_callback(1))
    manual_gettime.pack()
    manual_gettime.place(x=230,y=25)
        
    manual_Sndfram = tk.Button(manual, text = " Send Frame ", height=1,width=9,bd=4, command=lambda:sendframe_callback(0))
    manual_Sndfram.pack()
    manual_Sndfram.place(x=230,y=69)

    manual_delaySndfram = tk.Button(manual, text = " Send Frame At Time", height=1,width=19,bd=4, command=lambda:sendframe_callback(1))
    manual_delaySndfram.pack()
    manual_delaySndfram.place(x=230,y=109)

    manual_waitfrEvnt = tk.Button(manual, text = " Wait For An Event", height=1,width=19,bd=4, command=lambda:sendframe_callback(1))
    manual_waitfrEvnt.pack()
    manual_waitfrEvnt.place(x=230,y=149)    

    manual_Cancel = tk.Button(manual, text = " Cancel ", height=4,width=6,bd=4, command=cancelbutton_callback)
    manual_Cancel.pack()
    manual_Cancel.place(x=321,y=25)

#----------------------------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    manual_test()
