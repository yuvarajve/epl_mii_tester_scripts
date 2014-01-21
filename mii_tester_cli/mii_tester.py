#! /usr/bin/env python

###############################################################################
#
#    MII Test Script for Testing Ethernet PowerLink MII Layer
#
# This script sends command(s) received from user input (console) & sends it to
# the device (xcore). The machine which is running this test script is called
# as 'Host' and the xmos test environment connected to that machine using xtag 
# are called as 'Device(s)' hereafter.
###############################################################################
import os
import sys
import argparse
import logging
import time
import cmd
import platform
import string, sys
from random import randint
from mii_frame_gen import *

class main(cmd.Cmd):
    'Command-line test script for epl mii layer testing'
    
    prompt = 'eplmii> '
    
    def __init__(self):
        return cmd.Cmd.__init__(self)

    def do_sendframe(self, args):
        """ sendframe:
             Generates frames, updates 'FrameToDevice.txt' file
             Compares the frame data with 'FrameToHost.txt' file
             On 'auto' mode, at the end of frame, error frame is sent
        """
        argv = args.split()
        num_of_args = len(argv)
        if (num_of_args != 3):
            print "syntax error: sendframe takes exactly 3 arguments(",num_of_args,"given)\n"
            self.help_sendframe()
            return
        else:
            no_of_bytes = int(argv[2])  # Is this needed??
            time_delay = int(argv[1])   # Is this needed??
            
            if ( (time_delay < 0) or (time_delay > 5000) ):
                print "argument error: timedelay should be between 0-5000(",argv[1],"given)\n"
                self.help_sendframe()
                return
            
            elif ( (no_of_bytes < 64) or (no_of_bytes > 1522) ):
                print "argument error: datalength should be between 64-1522(",argv[2],"given)\n"
                self.help_sendframe()
                return
            
            if (argv[0] == 'manual'):
                print "manual mode chosen,", time.asctime( time.localtime(time.time()) )
                data = randint(1,255)       # choose some random data between 1 to 255
                                
                checksum = send_frame_to_host(no_of_bytes,data,time_delay)
                
                print "-----------------------------------------------------------"     #TODO: this needs to be changed
                print "| Mode\t TimeDelay\tDatalength\tData\tChecksum  |"
                print "-----------------------------------------------------------"
                print " {0:^6}\t".format(argv[0]),"{0:^6}\t".format(time_delay),\
                       "\t{0:^10}\t".format(no_of_bytes),"{0:^4}\t".format(data),hex(checksum)
            elif (argv[0] == 'auto'):
                print 'auto mode chosen,', time.asctime( time.localtime(time.time()) )
                print "-----------------------------------------------------------"     #TODO: this needs to be changed
                print "| Mode\t TimeDelay\tDatalength\tData\tChecksum  |"
                print "-----------------------------------------------------------"
                while no_of_bytes <= 1522:
                    data = randint(1,255)       # choose some random data between 1 to 255
                    checksum = send_frame_to_host(no_of_bytes,data,time_delay)
                    print " {0:^6}\t".format(argv[0]),"{0:^6}\t".format(time_delay),\
                       "\t{0:^10}\t".format(no_of_bytes),"{0:^4}\t".format(data),hex(checksum)
                    no_of_bytes += 1

                #TODO: Send error frame???

        return
        
    def help_sendframe(self):
        print "syntax: sendframe <mode> <timedelay> <datalength>\n"
        print "\t<mode> mode of testing - manual or auto"
        print "\t<timedelay> time delay between each frame - 0 to 5000mSec"
        print "\t<datalength> datalength of each frame - 64 to 1522 bytes"
        print "\t -- on auto mode: frames are send starting from the length mentioned here\n"
        print "\tEg: sendframe manual 2500 1000/sendframe auto 100 200"

    def do_senderror(self,args):
        """ senderror:
             Generates frames, injects error checksum, updates 'FrameToDevice.txt' file
             Compares the frame data with 'FrameToHost.txt' file
        """
        print "YTB implemented !!!"
        return

    def help_senderror(self):
        print "syntax: senderror"
        print "\tSends error frame on mii layer.Generally crc error."
        print "\tPacket size is fixed to 500 bytes with random data picked"
        
    def do_quit(self,args):
        sys.exit(1)

    def help_quit(self):
        print "syntax: quit",
        print "-- terminates the application"
       
    # shortcuts
    do_q = do_quit

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='EPL Mii Layer Tester Script')
    parser.add_argument("-v", "--verbose",
                        help="select the interface to use",
                        action="store_true")
    args = parser.parse_args()
    
    main().cmdloop()
