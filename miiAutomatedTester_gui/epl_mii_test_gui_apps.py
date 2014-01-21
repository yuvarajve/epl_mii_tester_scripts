## -------- XMOS Ethernet PowerLink MII Test Tool -------- ##
import Tkinter as tk
from Tkinter import *
from Tkinter import Label
import tkMessageBox
from Tkinter import Checkbutton
from epl_mii_auto import *
from epl_mii_manual import *
import time
import sys
import os


#------------------------------------- ExitButton CallBack ----------------------------------------------------------
def exitbutton_callback():
    main.destroy()
    
#------------------------------------- GoButton CallBack ----------------------------------------------------------
def gobutton_callback(testoption):
    if (testoption.get() == 1):
        automated_test()
    else:
        manual_test()
    
#------------------------------------- Starting Images ------------------------------------------------------------    
main = tk.Tk()
main.iconbitmap(default='xmos.ico')
main.title('XMOS: EPL MII Test Tool')
main.geometry("300x215")
logo1 = tk.PhotoImage(file="xmos.gif")
w1 = Label(main, image=logo1, justify=CENTER)
w1.pack()
logo2 = tk.PhotoImage(file="logo_epl.gif")
w2 = tk.Label(main, image=logo2, justify=CENTER)
w2.pack()
w2.place(x=20,y=125)

localtime = time.asctime( time.localtime(time.time()) )
datetime = tk.Label(main, text=localtime)
datetime.pack()
datetime.place(x=50, y=190)
    
#------------------------------------- RadioButton ----------------------------------------------------------------
testoption = IntVar()
tk.Radiobutton(main, text ="Automated Testing", variable = testoption, value = 1).place(x=20, y=90)
tk.Radiobutton(main, text ="Manual Testing", variable = testoption, value = 0).place(x=172,y=90)

#------------------------------------- Buttons ---------------------------------------------------------------------   
gobutton = tk.Button(main, text = " Go ", height=1,width=5,bd=4,command = lambda: gobutton_callback(testoption))
gobutton.pack()
gobutton.place(x=240, y=128)

exitbutton = tk.Button(main, text = " Exit ", height=1,width=5,bd=4,command = exitbutton_callback)
exitbutton.pack()
exitbutton.place(x=240, y=175)
#------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    main.mainloop()

