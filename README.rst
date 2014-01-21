Powerlink Controlled Node (401 IO profile) Demo
===============================================

:Latest release: 0.1.0beta0
:Maintainer: andrewstanfordjason
:Description: Powerlink

:scope: Example
:keywords: Powerlink, Industrial Comms, Ethernet
:boards: XA-SKC-L16, XA-SK-GPIO, XA-SK-E100

Key Features
------------

   - Jitter under 70ns
   - Isochronous Controlled Node
   - Data Link Layer running entirely in software
   - CN Cycle State Machine
   - Asynchronous SDO Sequence Layer
      - SDO history buffer of arbitrary size
   - Asynchronous SDO Command Layer
      - Command: Write by Index
      - Command: Read by Index
   - Process Data Object (PDO) Processing
   - NMT State Machine
   - NMT State Command Services

Still to be implemented
-----------------------

   - Event based application interface
   - Optional NMT commands
   - Optional SDO Commands
   - Asychronous only support
   - SDO over PDO
   - IPv4 support / Basic ethernet mode
   - SDO over UDP
   - CANopen access and range checking
   - Phy state reporting
   - MII CRC fail reporting
   - Multiplexed Slot Timing
   - Error Handling Table
   - PDO Error Handling
   - Configuration Management
   - NMT Info Services
   - Diagnostics

Known Issues
------------

   - The demo uses polling. This means that when the application wants to know if a PDO has changed then it has to poll the CANopen object that represents it.
   - Cross process communication have to go through the Object Dictionary (OD). This if implemented fully would cause a race condition.
   - The over head of accesses to the OD will limit the performace for large PDOs. 
   - Hub relay latency is 1.1us
   - The DLL and NMT are seperate logical cores, they communicate their state through a shared memory.

Support
-------

For all support issues please contact your FAE

 

Required software (dependencies)
================================

  * sc_util (git://github.com/xcore/sc_util)
  * sc_slicekit_support (git@github.com:xcore/sc_slicekit_support)
  * sc_otp (git@github.com:xcore/sc_otp)
  * sc_ethernet (https://github.com/xcore/sc_ethernet.git)

