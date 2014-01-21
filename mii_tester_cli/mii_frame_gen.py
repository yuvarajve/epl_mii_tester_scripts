## --------------- XMOS Ethernet Powerlink Frame Generator ------------------- ##
import Tkinter as tk
from Tkinter import *
from Tkinter import Label
from array import array
import os
import sys
import time
import struct

file_obj = None  ## File object
#####################################################################################################################################################
## Incorporate a word into a Cyclic Redundancy Checksum.
## The calculation performed is
## \code
## for (int i = 0; i < no_of_bits; i++) {
##   int xorBit = (crc & 1);
##
##   checksum  = (checksum >> 1) | ((data & 1) << 31);
##   data = data >> 1;
##
##   if (xorBit)
##     checksum = checksum ^ poly;
## }
#####################################################################################################################################################
def my_crc(checksum,data,nofbits):
    polynomial = 0xEDB88320
    xorBit = 0
    #print 'my_crc(): Enter: Input Checksum',hex(checksum),'data',hex(data)
    for i in range(0,nofbits):
        xorBit = (checksum & 1)
        
        checksum = ((checksum >> 1) | ((data & 1) << 31))
        data = (data >> 1)

        if (xorBit == 1):
            checksum = (checksum ^ polynomial)

    #print 'my_crc(): Exit: Checksum',hex(checksum) #,'xorBit',xorBit
    return checksum
        
#####################################################################################################################################################
def epl_frame_gen(nofbytes,userdata):
        
    actual_bytes = nofbytes - 14
    crc = 0x9226F562

    ## No need to send Ethernet Preamble & SFD
    ## For all the frames, make BROADCASTING IP as MAC Address (src, dstn)
    ## Ethernet type as 'EPL'
    
    ############### MAC Dstn Address               MAC Src Address                Ether type 0x88AB(LSB First)
    default_data = [0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0xAB,0x88]
    data_buffer  = [] #empty list

    ## Calculate Frame for default
    for i in range(len(default_data)):
        data_buffer.append(default_data[i])
        crc = my_crc(crc,data_buffer[i],8)
        
    for m in range(0,actual_bytes):
        data_buffer.append(userdata)
        crc = my_crc(crc,data_buffer[m+14],8)
    
    return data_buffer,crc

#####################################################################################################################################################
def send_frame_to_host(nofbytes,userdata,timeDelay):

    global file_obj
    
    epl_frame,checksum = epl_frame_gen(nofbytes,userdata)
    
    ## Check if file is already created/open
    if(file_obj == None):
        file_obj = open("FrameToDevice.txt","wb",1)
    else:
        ##print 'File already created\n'
        file_obj = open("FrameToDevice.txt","ab",1)  ## if file already created, open it to append

    file_obj.write("<")           ## Start of Frame
    
    ###################################################################################
    ## Formatting is done for making all the frame data's in file as 3 digit aligned ##
    ###################################################################################
    ## Load timeDelay Value - In automating mode, there is no time delay
    file_obj.write( struct.pack('>H',timeDelay) )                #unsigned short - 2 byte
        
    ## Load the byte length
    file_obj.writelines( struct.pack('>H',len(epl_frame)) )      #unsigned short - 2 byte

    ## Load the epl frame data
    for i in epl_frame:
        file_obj.write(struct.pack('B',i))                      #unsigned char - 1 byte


    ## Load the checksum value (LSB First)
    file_obj.writelines( struct.pack('>L',checksum) )            #unsigned long - 4 byte
    
    file_obj.write(">")           ## End of Frame
    file_obj.write("\n")          ## Newline
    
    file_obj.close()
    time.sleep(10)  #reduced wait time ##TODO: Compare both the file 

    
    open("FrameToDevice.txt","wb").close()  ## on manual mode, erase the previous write data
    #open("FrameToHost.txt","w").close()  ## on manual mode, erase the previous write data
    time.sleep(2) #File to refresh

    return checksum
        
#####################################################################################################################################################






    
