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
  sudo apt-get upgrade
  sudo apt-get install libfftw3-dev
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


Operation:
  1) dspp is the program name and commands have the format

\<do something\> \<parameter 1\> ... \<parameter n\>

   do something consists of:
  * convert_x_y - convert an incoming stream from x format to y format
  
    where x is:
  
        byte - signed/generic byte
        uByte - unsigned byte
        f - internal float (always signed, 32 bits)
        sInt16 - signed short integer - little endian
        uInt16 - unsigned short integer - little endian
        tcp - TCP stream of generic bytes
    and y is:

        byte - signed/generic byte
        uByte - unsigned byte
        f - internal float (always signed, 32 bits)
        sInt16 - signed short integer - little endian
        uInt16 - unsigned short integer - little endian
        tcp - TCP stream of generic bytes

  * shift_frequency_cc - shift the center frequency of the incoming signal, assumed to be in I/Q format, by x Hertz per sample
  * decimate_cc - filter the incoming complex stream and take 1 out of every n samples of complex data
  * decimate_ff - filter the incoming real stream and take 1 out of every n samples
  * fmdemod_cf - use FM demodulation on a complex incoming signal and produce a real signal having the frequency characteristics originally modulated into the RF signal
  * custom_fir_ff - FIR filter a real stream with custom coefficients
  * custom_fir_cc - FIR filter a complex stream with custom coefficients
  * fmmod_fc - modulate a real audio stream to FM modulated quadrature (I/Q) stream
  * head - take first n bytes of a stream
  * tail - take bytes after first n bytes of a stream
  * fft_cc - FFT of a complex real stream
  * tee - stream to another stream while forwarding down the same pipe

2) So a processing flow could look like this:


rtl_sdr -s 2400000 -f 145000000 - | ./dspp convert_uByte_f | ./dspp shift_frequency_cc `python -c "print float(144390000-145000000)/2400000"` | ./dspp decimate_cc 0.005 79 50 40 HAMMING | ./dspp fmdemod_cf | sox -t raw -b 32 -e float -r 48000 /dev/stdin -e signed-integer -b16 -t raw -r 22050 - | multimon-ng -t raw -A /dev/stdin

Which translates to:

* sample RF data from an rtl-sdr dongle centered at 145 MHz at a sampling rate of 2.4 MHz
* pipe that to dspp and convert the unsigned I/Q bytes to floating point
* shift the center frequency to 144.39 MHz
* pass that floating point I/Q data to a decimation filter that is a low pass filter that saves 0.005 of the bandwidth using a HAMMING window FIR filter with 79 taps, 1 out of every 50 samples, with a sample size of 40 samples on the output
* send the filtered data (now at a 48 kHz sample rate) to the FM demodulator
* pass the derived sound stream to sox to convert it to 22,050 Hz
* then send it to multimon-ng to extract APRS data. 
