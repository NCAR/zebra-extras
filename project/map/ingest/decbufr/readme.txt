BUFR-Encoding and Decoding Software Version 1.82 beta (October 23, 1997):

Author: K.Koeck.

        Institute of Communications and Wave
        Propagation, Technical University Graz,
        Austria.

        Inffeldgasse 12
        A-8010 Graz, Austria

        Phone:  ++43 316 873-7431
        FAX:    ++43 316 463697
        E-mail: kon@finwpc10.tu-graz.ac.at

This software may only be used, copied and distributed within members
of the Central European Weather Radar Network (CREAD) or within the 
Liaison Group on Operational European Weather Radar Networking (GORN). 
Any other use only with the permission of the author.

CONTENTS:


1.)  What this software is intended for.

2.)  How to compile and link the sourcefiles.

3.)  How to use this software.

4.)  Description of the "one byte per pixel" format.

5.)  How to link the sources to existing programs.

6.)  What does not comply with BUFR in this software.

6a.) Treatment of missing data

6b.) Using 4- or 8-bit-pixel-values.

6c.) How to encode radar images represented in dBZ.

7.)  Error reporting.

8.)  History.

============================================================

1.) What this Software is intended for:

  This software is able to encode/decode any data to/from 
  FM94-BUFR-format. Original data is stored in an ASCII-file
  (containing a link to a binary file holding the radar-image)
  complemented by a file containing all supported BUFR-data-
  descriptors. Besides the encoding of data according to the 
  data-descriptors this software is also able to handle 
  sequence descriptors as well as replication. The primary
  application of this software shall be in the exchange of
  BUFR-encoded weather-radar images.

============================================================

2.) How to compile and link the sourcefiles:

  The distribution-disk contains the following files:
  - README.TXT    This file.
  - BITIO.C       Software-modules for bitoriented input/output
                  to/from memory-buffers.
  - DESC.C        Modules that read all supported data-
                  descriptors from the file of supported
                  descriptors (DESCR.TXT).
  - RLENC.C       Functions to encode/decode a "one byte per
                  pixel" radar-image to/from the BUFR-runlength-
                  format.
  - BUFR.C        Contains the modules needed to encode/decode a 
                  sequence of data-values and their corresponding data-
                  descriptors to/from a data- and data-descriptor-
                  section.
  - ENCBUFR.C     Main-module to encode one data-source-file (ASCII) 
                  to a BUFR-file.
  - DECBUFR.C     Main-module to decode one BUFR-file to an ASCII-
                  file.
  - *.H           There is one *.H-file for each of the above *.C-files
                  each of them containing function-prototypes for the
                  function in the *.C-files as well as necessary 
                  definitions.
  - MAKEFILE.OS2  This is the makefile needed to compile/link the
                  sources for OS/2 using the IBM-CSet/2-compiler.
  - MAKEFILE.ULT  Makefile for UNIX/ULTRIX.
  - MAKEFILE.VMS  Makefile for VMS.
  - MAKEFILE.C6   Makefile for DOS using the Microsoft C 6.00 compiler.
  - MAKEFILE.SUN  Makefile for SunOS, non-ANSI-C-Compiler.
  - MAKEFILE.ALF  Makfeile for DEC/Alpha Digital Unix
  - MAKEFILE.VC4  Makefile for Microsoft Visual C++ 4.2
  - DESCR.TXT     ASCII-file that contains a list of the supported data-
                  descriptors. This file is read by the encoding/decoding
                  software during runtime to have access to scale-factors,
                  reference-values, data widths and definitions of sequence-
                  descriptors. This file also contains definition of 
                  code-tables that are not yet used. Do not modify this
                  file as the encoding/decoding depends on it.
  - MSG.SRC       Sample for a data-sourcefile that can be encoded to BUFR
                  with ENCBUFR.
  - IMG.RAW       This is a "one byte per pixel" radar-image containing
                  an Austrian composite weather radar image with a resolution 
                  of 412 columns by 324 rows.
  - MSG.BUF       Is MSG.SRC encoded to BUFR.

  To compile and link the encoding/decoding software use the appropriate
  makefile. Changes in the makefile will be needed if you wish to use
  different compilers or operating systems than the ones mentioned above.
  Note that ANSI-C-function-prototyping is used in this software. If you
  use a compiler that doesn't support ANSI-function-prototypes (e.g. SunOS)
  add the option -DNON_ANSI to your compiler options (makefile).

  In the includefile desc.h the internal floating-point-representation is 
  defined (typedef .... varfl). This type is used for the interface to the 
  general BUFR-encoding/decoding modules in BUFR.C. VARFL can be defined as 
  float or double. Floats have the advantage to consume less memory than 
  doubles, but need to be converted to double by your operating system 
  before any operation, which consumes more computing-time. It is
  recommended to use double for 32- or 64-bit machines and floats for
  16-bit machines (DOS). If you change the definition for varfl from
  double to float the format-strings of all scanf-calls in the whole 
  source must be adapted.

  Remark for those who intend to use DOS: As most compilers are only
  able to allocate memory-blocks with a size of less than 64 kB,
  problems may occur when encoding large images. In that case use
  a DOS 32-bit-compiler like WATCOM.
  
============================================================

3.) How to use this software

  The following files are needed to encode a radar-image to a BUFR-
  message:




  ------------
  | DESCR.TXT|--\
  ------------	|      ---------
		\----->|       |
  ------------	       | ENC-  |      ------------
  | MSG.SRC  |-------->| BUFR  |----->| MSG.BUF  |
  ------------	       | .EXE  |      ------------
		/----->|       |
  ------------	|      ---------
  | IMG.RAW  |--/
  ------------

  DESCR.TXT: Is a list of supproted data-descriptors. In that file all
             data widths, reference values and scaling factors as well as
             sequence-descriptors are defined.

  MSG.SRC:   ASCII-file containing the data-source (data/time-info,
             number of lines, columns, etc.) except the radar image
             itself. MSG.SRC contains a link to a binary file holding
             the "one byte per pixel" radar-image (IMG.RAW).

  IMG.RAW:   "one byte per pixel" radar-image-file.

  MSG.BUF:   BUFR-encoded output-file.


  The following files are needed to decode a radar image from BUFR:

  ------------	       ---------	 ------------
  | DESCR.TXT|-------->|       |-------->| MSG.DST  |
  ------------	       | DEC-  |	 ------------            
	      	       | BUFR  |
  ------------	       | .EXE  |	 ------------
  | MSG.BUF  |-------->|       |-------->| IMG.DEC  |
  ------------	       ---------	 ------------

  DESCR.TXT: see above.

  MSG.BUF:   BUFR-encoded radar-image.

  MSG.DST:   ASCII-file containing the decoded data (data/time-info,
             number of lines, columns, etc.) except the radar image
             itself. MSG.DST contains a link to a binary file holding
             the "one byte per pixel" radar-image (IMG.DEC).


  After compiling the software you should now have two EXE-files
  available: ENCBUFR.EXE and DECBUFR.EXE. Off course the file-name
  extension depends on the operating system you are using.

  Now you can try to encode the enclosed sample-data-source (MSG.SRC)
  to BUFR. This (ASCII-)file contains the following lines (comments are
  marked with a '#'):

  #
  # data/time info
  #
   3 01 011    1993
               8
               22
   3 01 012    23
               5
  #
  # number of rows/columns
  # 
   0 30 021    412
   0 30 022    324
  #
  # level slicing
  #
   3 13 010    0
               7
               0.2
               0.6
               1.7
               5
               15
               50
               90
   3 21 192    img.raw

  This file consists of a list of data-descriptors each of them with the
  appropriate number of data-values. Beginning at the data/time-info you
  find the sequence-descriptor 3 01 011 being defined as 0 04 001, 
  0 04 002 and 0 04 003 (see WMO manual of codes, part B: binary codes)
  for which you need three data-values (1993, 8, 22). The same for the
  3 01 012-descriptor as well as for the number of rows/columns-descriptor.

  The level-slicing is more complicated: 3 13 010 stands for 

           0 21 036  (bottom rainrate-value for pixel-value 0)
           1 01 000  (delayed replication of one descriptor)
           0 31 001  (replication factor; corresponds to 7 in data-section in
                      our example)
           0 21 036  (rainrate-value)

  Note that 1 01 000 is a replication-descriptor indicating that 0 31 001 
  defines a replication-factor. The following 0 21 036 is replicated 7
  (delayed replication factor in the data-section) times.

  Now we come to the radar-image itself: 3 12 192 is a sequence-descriptor
  defining a radar-image. The encoding software checks for this
  3 12 192 and assumes that this value is followed by the name of a file 
  containing the radar-image in a "one byte per pixel"-format. In our 
  example the radar-image is stored in the file "IMG.RAW".

  After describing the format of the data-source-file you can try to encode
  this file to BUFR. Enter the command "ENCBUFR MSG.SRC MSG.BUF" and your
  machine creates the BUFR-encoded file MSG.BUF. Note that the GTS-bulletin-
  header is not added to the BUFR-file by this software. That task must be
  done by your GTS-people.

  To decode the BUFR-file enter the command "DECBUFR MSG.BUF MSG.DST" and
  the BUFR-message will be decoded to the file MSG.DST. The format of this
  file is equivalent to the source file. The radar-image itself is again 
  stored in a "one byte per pixel"-format. The default-name of this file
  is IMG.DEC but can be modified by enterimg a file-name as a third parameter
  of the command (e.g. "DECBUFR MSG.BUF MSG.DST MY_FILE.DEC")

  The BUFR-file can be copied from any machine to any other for decoding.
  Problems could occur on VMS because of record-length and/or file-formats 
  of binary files. I transferred the files to/from VMS via FTP
  and had no problems because FTP toke care of it.

============================================================

4.) Description of the "one byte per pixel" format:

  To interface with the BUFR encoding and decoding software you have to
  store your radar-image in a "one byte per pixel" format:

  Assuming you have a radar-image consisting of 100 rows by
  200 columns your "one byte per pixel"-file must contain
  20.000 Bytes with the following content:

  Byte     0: row   0, column   0
  Byte     1: row   0, column   1
  .
  .
  Byte   199: row   0, column 199
  .
  .
  Byte   200: row   1, column   0
  Byte   201: row   1, column   1
  .
  .
  Byte 19800: row  99, column   0
  Byte 19801: row  99, column   1
  .
  .
  Byte 19998: row  99, column 198
  Byte 19999: row  99, column 199

============================================================

5.) How to link the sources to existing programs:

  ENCBUFR and DECBUFR which were described in the chapter above are the
  "easy" interface to the general BUFR-software. If you wish to link this
  software to an existing package you should do that as follows:

  In BUFR.C there is one function called BUFR_CREATE_MSG that creates a
  BUFR-data-descriptor- and data-section which is the most complicated
  part of the software. This function takes also care of sequence descriptors 
  and replication. As described in the source-code you must supply this
  function with a number of data-descriptors and the appropriate number of
  data-values in varfl-format (see chapter 2, definition of type varfl).
  Looking at the example of chapter 3 you would have to setup the DESCS- and
  VALS-array as follows:

  descs[0] =    3 01 011    vals[0]  = 1993
                            vals[1]  = 8
                            vals[2]  = 22
  descs[1] =    3 01 012    vals[3]  = 23
                            vals[4]  = 5
  descs[2] =    0 30 021    vals[5]  = 412
  descs[3] =    0 30 022    vals[6]  = 324
  descs[4] =    3 13 010    vals[7]  = 0
                            vals[8]  = 7
                            vals[9]  = 0.2
                            vals[10] = 0.6
                            vals[11] = 1.7
                            vals[12] = 5
                            vals[13] = 15
                            vals[14] = 50
                            vals[15] = 90
  descs[5] =   3 21 192     vals[16] = ...
                            vals[17] = ...
                            vals[18] = ...
                            vals[19] = ...
  
  vals[16] to vals[XX] contain the runlength-encoded radar-image. That array
  can be setup by the function RLENC.C. Setup the VALS-array as follows:

    varfl *vals;
    size_t nvals;
    vals = NULL;
    val_to_array (&vals, 1993, &nvals);
    val_to_array (&vals, 8,    &nvals);
    val_to_array (&vals, 22,   &nvals);
    .
    .
    val_to_array (&vals, 50,   &nvals);
    val_to_array (&vals, 90,   &nvals);
    rlenc ("img.raw", 324, 412, &vals, &nvals);
    .
    .
    bufr_create_msg (descs, 5, vals, nvals, .....);
    free (vals);

  VAL_TO_ARRAY allocates memory to hold one varfl-value and reallocates the
  memory-block for all subsequent values. RLENC encodes the radar-image to
  a runlength-format and stores the result also by a call to VAL_TO_ARRAY.
  Don't forget to free VALS after encoding to BUFR.

  The general decoding function is also in the file BUFR.C and is called
  BUFR_READ_MSG and works "the other way round" than BUFR_CREATE_MSG does. 
  There is also a runlength decoding-function called RLDEC in the file RLENC.C

  Before calling BUFR_CREATE_MSG or BUFR_READ_MSG the supported 
  data-descriptors must be read from the file DESCR.TXT which can be done by 
  a call to READ_DESC.

============================================================

6.) What does not comply with BUFR in this software.

  Some restrictions to the BUFR-definition had to be made to write
  this software with moderate effort:

  a) Radar-images can only be encoded using 4 bit per pixel in the BUFR-format
     (not in the "one byte per pixel"-file !!). If the encoding-software 
     detects a pixel-value consuming more that 4 bits it creates a warning. 
     An Extension of the software which allows to use a variable number of 
     bits per pixel is planned for the future.

     Note for version 1.8: From this version on also 8-bit data can be en-
     coded.

  b) Rainrate-values are encoded in mm/h and not in m/s as defined in the manual
     of codes. Rainrate-values in m/s are not handsome 
     (e.g. 1 mm/h = 0.000 000 277 m/s).

  c) Sequence descriptors "3 21 192" and "3 21 193" indicating the radar-image 
     had to be defined.

============================================================

6a.) Treatment of missing data

  According to the BUFR-definition a missing value is indicated by a data-
  value with all bits set to 1. If you have pixel-values with missing data
  just set all bits of the one-byte-per-pixel-file to 1 (For 4-bit-pixels
  the lowest 4 bit, for 8-bit-pixels 8 bits). Be sure that YOUR APPLICATION
  which is actually dealing with the BUFR-encoded radar images "knows" how
  to handle missing data. Note for users being involved in the CERAD-
  project: The CERAD compositing center is currently (April 1997) NOT
  ready to handle missing data !

============================================================

6b.) Using 4- or 8-bit-pixel-values.

  The BUFR-software is able to create BUFR-messages with 4-bit-pixels as
  well as with 8-bit-pixels. Therefore 2 different sequence-descriptors
  ("3 21 192" and "3 21 193") had to be defined. 

  If you want to encode 4-bit-pixel data use "3 21 192" and the software 
  will only take the 4 least significant bits of the one-byte-per-pixel 
  file. Note that the encoding-software generates a warning if your
  one-byte-per-pixel file contains values >= 0xf !

  If you want to encode 8-bit-pixel data use "3 21 193" and the software
  will take all 8 bits of the one-byte-per-pixel file.

============================================================

6c.) How to encode radar images represented by dBZ.

  In GORN (Group on radar networking) it was agreed to encode radar-images
  commonly on rainrate (mm/h). Nevertheless radar images can also be
  encoded in dBZ. To mange this descriptor 0 13 009 must be used
  instead of 0 13 010 to describe the level-slicing.

  Evereything else can be handled in the same way as for mm/h.

============================================================

7.) Error reporting:

  If any error is detected in this software it should be reported to the
  author. The error will be fixed in the next release. A detailed description
  of the error should be sent by E-mail or FAX to the author including
  the following information:

  - Detailed error-description.
  - Operating system used.
  - Compiler used.
  - Description how to reproduce the error.
  - Diskette containing the encoded/decoded data if necessary. Those files
    may be copied via Internet on request if not sent by diskette.
  
  If anyone fixes an error on his own he should report it to the author, so 
  that everybody can be supplied with the corrected version of the software.

============================================================

8.) History:

  * Version 1.0 (May  5, 1994): Initial Version.

  * Version 1.1 (May 24, 1994): 
    - Byte-Order in the BUFR-message was changed from LSB first to 
      MSB first.
    - Number of Bytes for descriptor 0 21 036 was changed from 16 bits to
      12 bits (change in DESCR.TXT). This complies with BUFR-definition now.
    - Some errors in DESCR.TXT were fixed.
    - Non-ANSI-function-declarations are supported now (see chapter 2).

  * Version 1.2 (May 25, 1994):
    - The name of the "one byte per pixel"-file after decoding can be
      modified by entering a file-name as the third parameter of the decoding-
      command (e.g. "DECBUFR MSG.BUF MSG.DST MY_FILE.DEC").

  * Version 1.3 (Sep 8, 1994):
    - Added function BUFR_CLEAN in file BUFR.C that frees all memory allocated
      by the BUFR-functions.

  * Version 1.4 (Dec 20, 1994):
    - Changed the sequence-descriptor 3 21 192

  * Version 1.5 (Feb 28, 1995):
    - In the decoding software the lengths of BUFR-sections are compared 
      with the length of the input-file. The software returns an error if
      the lengths of the BUFR-sections > length of BUFR-file.
    - Before decoding the BUFR-message is checked for "BUFR". A GTS-header
      is skipped if present.

  * Version 1.6 (March 29, 1995):
    - Changed sequence descriptor 3 21 192 to be compient with BUFR.
    - Added support of rescaling of descriptors to the BUFR-software.
    - Added version-number to all *.h and *.c files.

  * Version 1.7 (November 3, 1995):
    - Increased internal buffer im RLENC.C
    - As BUFR is only specified to handle compressable goups having a 
      size < 256 all groups >= 256 are split into subgroups < 256

  * Version 1.81 beta (April 18, 1997):
    - To encode 8-bit-pixel values the descriptors "3 21 193" and  "0 30 002"
      have been added to descr.txt. To encode 4-bit data use descriptor 
      "3 21 192", to encode 8-bit-data use descriptor "3 21 193". See also 
      new section 6b.
    - Treatment of missing data has been defined (see new section 6a).
    - The date/time-info in section one is now the same as the date/time-
      info in the data section (commonly defined by 3 01 011 and 3 01 012).
      If there is no date/time-info in the data-section date/time-info is
      taken from the system-time.
    - Added 0 02 135 to descr.txt for Antenna elevation
    - Added 0 21 001 to descr.txt for horizontal Reflectivity
    - Added 0 21 002 to descr.txt for vertical Reflectivity
    - Added 3 13 009 to descr.txt for encoding radar images in dBZ.
    - Added support of radar-images represented by dBZ (see new section 6c).

  * Version 1.82 (August 19, 1997):
    - Changed a minor bug in rlenc.c

K.Koeck, Oct 23, 1997
