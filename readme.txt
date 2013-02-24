Winuac2 project README
======================

Version 20130224 BSB initial


Introduction
============

This is a loose collection of hints on how to build and load kernel-mode
drivers. 


Toolchain
=========

The project uses Microsoft WDK 7600.16385.1. Download the .iso file at
  http://www.microsoft.com/en-us/download/details.aspx?displaylang=en&id=11800

For debug setup see 
  http://www.osronline.com/article.cfm?article=295 and debugview.reg

Use DebugView, download from
  http://download.cnet.com/DebugView/3000-2218_4-10213957.html
IMPORTANT: DebugView -> Capture -> Capture Kernel

Driver loader, use the WLH (Vista) build on Windows 7 of:
  http://www.osronline.com/article.cfm?article=157 

To build a driver, use a checked build environment corresponding to the 
configuration of your test computer. CD to the correct folder and give the 
command "build". You probably want to run all these programs as Administrator.

Copy the resulting .sys file to the test computer's hard drive. Browse to it in
the Driver Loader. Start DebugVeiw with the Capturre Kernel option enabled. In
Driver Loader ensure that start="Demand", then register and start the driver
service. Hope and pray that your test computer doesn't blue-screen!

After a re-build you must stop and the driver, copy in the new .sys file and
start the service again. It doesn't seem like you have to unregister and 
register the service again. Only tested on a driver consisting of a single
.sys file.


Documentation
=============

Download documentation from 
  http://msdn.microsoft.com/en-us/library/windows/hardware/gg487458.aspx
  http://msdn.microsoft.com/en-us/library/windows/hardware/gg463002.aspx
  
Driver tutorial http://www.catch22.net/tuts/introduction-device-drivers


Signing
=======

Signatures (or signature disable) are required to load Win7-64. 

Create test certificate:
  MakeCert -r -pe -ss TestCertStore -n "CN=TestCert01" TestCert01.cer

Register test certificate Win7:
  CertMgr -add TestCert01.cer -s -r localMachine root

TODO: generate .cat file (.inf file needed??) to load driver on Win7-64

