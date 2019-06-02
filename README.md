# Digital Signal Processing Pipes 


This repository contains C/C++ programs that read and write streams of data. My primary use is processing of RF I/Q data from a RTL-SDR dongle (see rtl-sdr.com).  This project was inspired by the CSDR github repository published by Simonyi Károly who acknowledges major contributions by Andras Retzler, Péter Horváth, PhD, and János Selmeczi, PhD. I encourage you to look at his github repository which is loaded with references that were useful to me.

My primary interest was to become familiar with the theory behind the processing implemented in CSDR.  I wanted to be able to understand what they were doing in their routines well enough that I could derive my own implementation using my own conventions. Also, I've always been interested in developing efficient and clean implementations that have high performance due to their lack of clutter and simplicity.

I have an assortment of Raspberry PIs so they are my primary target machine.  However, I have used Macs for decades and use it for a development environment and intend these algorithms to work on these machines also.

Installation (Raspberry PI example):
  1)  Install Stretch Lite from www.raspberrypi.org/downloads/raspbian
      I do headless installs of my PI 0's which, on the publication date
      means that I copy the raspbian image to the SD card plugged into my
      Mac, mount the card and touch the ssh file on the boot partition and
      and create a wpa_supplicant.conf file.
  2)  Boot off the installed image.
  3)  Change the password.
  4)  Change the node name to whatever you want.
  5)  Update the installation and install build tools
'''
  sudo apt-get update
'''
  6)  Install the repository
'''
  git clone https://github.com/mbroihier/dspp.git
'''
  7)  Build

```
  cd dspp
  make

```
Now you have an executable, dspp, that you can pipe data to and apply operations on each section of pipe.


Conventions:
  1) dspp is the program name and commands have the format:
     a) <do something> <parameter 1> ... <parameter n>
     b) do something consists of:
     	i) convert_x_y - convert an incoming stream from x format to y format
	   *) x is byte - signed byte
	   *)      uByte - unsigned byte
	   *)      f - internal float (always signed, 32 bits)
	   *)      sInt16 - signed short integer - little endian
	   *)      uInt16 - unsigned short integer - little endian
	   *)      tcp - TCP stream of unsigned bytes
	   *) y is byte - signed byte
	   *)      uByte - unsigned byte
	   *)      f - internal float (always signed, 32 bits)
	   *)      sInt16 - signed short integer - little endian
	   *)      uInt16 - unsigned short integer - little endian
	ii) shift_frequency_cc - shift the center frequency of the incoming signal, assumed to be in I/Q format, by x Hertz per sample.
	iii) decimate_cc - filter the incoming complex stream and take 1 out of every n samples of complex data
	iv) decimate_ff - filter the incoming real stream and take 1 out of every n samples
	v) fmdemod_cf - use FM demodulation on a complex incoming signal and produce a real signal having the frequency characteristics originally modulated into the RF signal
	