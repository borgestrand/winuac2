HULA and XMIDI drivers are the USB Audio and MIDI drivers for 
the EMU 04/04, 02/02, TrackerPre, and XMIDI products.  These drivers
also offer basic functionality to support EMU CA0189-based future products. 

This code allows customiztion of the drivers for any future products based on
EMU CA0189.  




BUILDING THE HULA AND XMIDI DRIVERS

Prerequisites:

In addition to the actual huladrv source code, you'll also need to
install the following items

        1.   A recent Windows WDK distribution.   WDK version 6001.18001 is 
			 known to work. 
		
        2.   A driver installer is not included.  However, the INF files are 
			 included.  If you already have an EMU 04/04.  Simply copying the 
			 emusba10.sys file on top of the existing one in the drivers directory
			 should suffice for testing. 

        3.   You will need to add the HULAROOT variable to your environment.
             It should point to the huladrv directory.  

        4.   The Stienberg Asio.h file.   We don't have explicit permission to redistribute, 
			 but it's easy to google up.  There is a placeholder Asio.h file in the directory
			 asiodll\include.  You'll need to replace that file w/ the one from Stienberg. 


General Procedure:
        1.  Start up a shell in the WDK build environment of your choice.
        2.  Make sure that you have the HULAROOT environment variable set.
        3.  Make sure that you have the bldpkg.cmd in you PATH.  A copy can
            be found in %HULAROOT%\bin\win\scripts.
        4.  Change your current directory to %HULAROOT%
        5.  Type
                        bldpkg -@ hula.pkg 
                    or
                        bldpkg -@ xmidi.pkg 

        6.  When bldpkg is finished, you should end up with an installable
            driver in the package subdirectory.


Miscellaneous tips and suggestions:

	* You can pass flags to the underlying build system using the -build flag.
	  For example, bldpkg -build -cef -@ hula.pkg %INSTALLROOT% will perform
	  a clean build.

	* Use -retail off to create a debug build.


ADDENDUM:
	Note the .dsw file in the build directory.  This can be used to build the 
	driver.  This works with VC 6.0.  
