/* -*- C -*-
 *
 * Zebra configuration parameters.
 * -------------------------------
 * In this file you can override any of the default configuration
 * parameters.  All of the parameters are definition directives for
 * the C pre-processor, of the form:

#define Symbol <definition>

 * To set a symbol to True or Yes or On, use

#define Symbol YES
  
 * To turn a parameter off or disable an option, use

#define Symbol NO

 * Some symbols are not YES or NO but actually directory names or commands.
 * If a Symbol is not defined in this file, it will be assigned a default
 * definition in the Zebra template file, Project.tmpl, in this directory.
 * Most of the defaults are mentioned in this file and there are
 * instructions for how to change many of the configuration parameters.
 * However, many more parameters exist.  If you think it necessary to
 * change some other parameters, such as when porting to a new platform,
 * look in the Project.tmpl file for all of the available parameters and
 * their defaults.  Also see the Imake chapter of the Zebra Developer's
 * Guide.  When in doubt about a parameter, try the default first and then
 * adjust it if it doesn't work.
 */

/*============================== Don't change anything in this section ==*/
/*_/////////////////////////////////////////////////////////////////////-*/

# ifndef _zebra_config_h_	/* protect against multiple inclusion */
# define _zebra_config_h_

/*
 * Required when being included from a source file
 */
# ifndef YES
#	define YES 1
# endif
# ifndef NO
#	define NO 0
# endif

# ifdef MAKING_MAKEFILE
XCOMM -----------------------------------------------------------------------
XCOMM Definitions from config.h included here.  The definitions here override
XCOMM the defaults in Project.tmpl.  See ...../zebra/config/project/config.h.
XCOMM $Id: config.h,v 1.1.1.1 2001-04-16 18:19:00 granger Exp $
XCOMM -----------------------------------------------------------------------
# endif

/*_/////////////////////////////////////////////////////////////////////-*/
/*=======================================================================*/

/*
 * Which help browser do you want to use?  If you set UseMosaic to YES,
 * then Mosaic will be used; otherwise the (old) xhelp browser will be
 * used.  Xhelp is almost certain to be removed from future releases, so
 * the use of Mosaic is recommended.  The default definition for UseMosaic
 * is YES.  Use the MOSAIC_COMMAND symbol to define a default path to
 * Mosaic; it can be either an absolute path or an executable name.  If
 * left commented out, zebra looks for Mosaic and xmosaic on your path.  If
 * defined, zebra checks the defined path before trying the other options.
 * The path must be quoted.
 *
 * MOSAIC_COMMAND may also be set to "netscape" and the right thing will
 * happen.
 *
 * The UseXHelp variable in the display manager can be used to override this
 * definition at run time.
 *
 * Mosaic's distribution conditions do not allow us to distribute the software,
 * but you can get it in binary or source form from ftp.ncsa.uiuc.edu.
 */
# define UseMosaic YES
/* # define MOSAIC_COMMAND "Mosaic" */

# ifdef MAKING_MAKEFILE
/*=======================================================================
 * CHECK THE FOLLOWING DEFINITIONS:
 * 
 * This section defines the directories in which Zebra files live.  These
 * are set-up for a default Zebra installation under /zebra, with the bin
 * and lib directories in /zebra/bin and /zebra/lib.  If you would like to
 * change the destination for Zebra, edit the definitions below.  The
 * definitions should be ABSOLUTE PATHS and they should NOT be enclosed in
 * quotes.  If you wish to install Zebra within the distribution tree,
 * set BaseDir to the absolute path of the top directory.
 * 
 * PLEASE NOTE:
 * Files will be installed into the LibDir, BinDir, and IncDir
 * directories.  If you wish to preserve files already stored in these
 * directories, either move the directories BEFORE building Zebra or
 * specify different directory paths for the symbols below.  If you wish to
 * overwrite a previous version of Zebra, you might as well completely
 * erase the contents of BinDir so that programs which are no longer used
 * are not left taking up space.
 */

# define BaseDir	/home/zebra
# define BinDir		BaseDir/bin
# define LibDir		BaseDir/lib
# define IncDir		BaseDir/include

/*
 * If RDSS is already installed on the system elsewhere, define the RDSSDir
 * symbol to the top directory of the RDSS distribution tree.  Otherwise,
 * if you have a Zebra distribution which already includes the needed RDSS
 * libraries, define the symbol BuildRDSS to YES.  If BuildRDSS is YES,
 * ignore all of the other RDSS include and library definitions which are
 * asked for further on in this file. 
 */
# define BuildRDSS	YES

# if !BuildRDSS
#  define RDSSDir	/rdss
# endif

/*
 * If the UseInstalledZebra symbol is YES, Zebra executables will link with
 * the installed libraries instead of the libraries built in the source 
 * directories.  Sometimes it is more convenient to link with the source.
 * If you don't understand or don't care about the difference, leave this
 * as YES.
 */
# define UseInstalledZebra	YES

/*=======================================================================*/

/* =========================================================================
 * The following definitions determine which parts of the Zebra system will
 * be compiled and installed in the build process.
 *
 * 	YES --- The capability WILL be compiled into the system.
 * 	NO  --- The capability WILL NOT be included into the system.
 *
   ------------------------------------------------------------------------ */

/*
 * If you're using Mosaic exclusively and do not want to build or install
 * the xhelp utility, define BuildXHelp to NO.  It defaults to NO if
 * UseMosaic is YES and BuildXHelp is not defined.  If you want the default
 * to be Mosaic, but you still want to compile xhelp capability, define
 * BuildXHelp to YES and UseMosaic to YES.  If BuildXHelp is NO, then zebra
 * will always try to use Mosaic no matter what (since xhelp won't be there).
 *
 * If you have Mosaic and will not be using older project configurations
 * such as the StormFEST CD's (which have project-specific info under xhelp),
 * then you may as well define this to NO.
 */
# define BuildXHelp NO

/* 
 * Do you want to build any of Zebra's ingest modules?  Only recommended
 * for Sun machines, and only if you know that you need them.
 */
#define BuildIngest NO

/*
 * Here you can select which individual ingest modules to build.  
 * NONE of these will be built if BuildIngest is NO
 */
# define	BuildIngestScheduler	YES	/* Sun only	 	*/
# define	BuildClassIngest	YES
# define	BuildRadarIngest	YES	/* Sun only, see PCAP below*/
# define	BuildSLGrabber		YES	/* Sun only		*/
# define	BuildNowradIngest	YES
# define	BuildProfsIngest	YES
# define	BuildSatelliteIngest	YES
# define	BuildP3Composite	YES
# define	BuildTOGASlowTape	YES
# define	BuildSlowTapeIngest	YES
# define 	BuildTAOIngest		YES
# define	Build_GMS_ISCCP		YES
# define	Build_TRMM_SSMI		YES
# define	Build_TRMM_Rain		YES	/* Sun only		*/
# define	Build_TRMM_Sonde	YES
# define	Build_FEST_P3_C130	YES
# define	BuildPrecipIngest	YES
# define	BuildDAPIngest		YES
# define	BuildRAPDataServerIngest YES
# define	BuildWetnetIngest	YES

/*
 * The PAM ingest programs (pam_ingest and daypam_ingest) require libraries
 * which are compiled within the RDSS source tree.  Do not define this to YES
 * unless your RDSS source tree still contains the compiled mda, pamutil, and
 * unp libraries (i.e., it all built successfully and has not been cleaned).
 */
# define	BuildPamIngest		NO   /* Requires RDSS compiled tree */

/*
 * Set this if you want the ingest-oriented DataStore utilities (these being
 * NetXfr, LastData and Archiver).  Most sites do not need this stuff.  They
 * will not build properly under a non-ANSI compiler.
 */
# define	BuildRealtimeDataStoreTools	YES

/*
 * The data file utilities are the program for handling various file 
 * and converting files to Zebra's netCDF conventions.  Some examples
 * are gprotocdf and mudtocdf, for GENPRO and MUDRAS, respectively.
 * Unless you know you will need them, you might as well wait and only
 * compile the programs you need when you need them.
 */
# define	BuildDataUtilities		NO

/*
 * The map utilities allow converting maps from a few different existing
 * formats into a Zebra-usable format.  The forms handled include USGS
 * DLG/SDTS format, CIA World Database II CBD format, and NCAR Graphics map
 * format.  A couple of Zebra map "massaging" utilities are also provided. 
 */
# define	BuildMapUtilities		NO


/* //// That's all of the defintions for choosing pieces to install.///// */
/* //// Go the next section for other compile configuration options ///// */
/*========================================================================*/


/*========================================================================
 * COMPILATION OPTIONS
 * -------------------
 *
 * In the following section, you can override any of the configuration
 * defaults.  In general, here you choose whether you want to compile with
 * MIT's X Windows or Sun's OpenWindows, and whether you want to use Sun's
 * C compiler, cc, or GNU C, gcc, or some other C compiler.  Other
 * parameters indicate the availability of certain capabilities utilized by
 * Zebra, such as the X Shared Memory extension.
 *
 * By default, the configuration will attempt to compile with your systems
 * default C compiler and link with X libraries from /usr/lib or any other
 * standard library directories.  You can link with MIT's X and still use
 * Zebra under OpenWindows, and vice versa.  Zebra must know if it is being
 * linked with OpenWindows, however, because the build must make some
 * adjustments in order to link properly.  If you have MIT's X
 * distribution, Release 4 or 5, use it.  If you only have OpenWindows,
 * version 2 or 3, then use that (the most recent version possible) and
 * define the UseOpenWindows symbol below to YES and the OpenWindowsVersion
 * symbol to the appropriate version number
 */

/*
 * Using Open Windows?  Then define UseOpenWindows to YES.
 * If the home directory of OpenWindows is not the default /usr/openwin,
 * then define OpenWinHome to the correct directory.
 */
# define UseOpenWindows 	NO	
# define OpenWinHome		/usr/openwin

/*
 * Does your X system support the shared memory extension?  The answer is
 * YES for MIT X, release 4 or greater, for Openwin 2.0 or above, and for
 * HPUX.  If unsure, run "xdpyinfo" and look for a line indicating the MIT-SHM
 * extension.
 */
# define XSharedMemory YES

/*
 * Compiler information:
 *
 * An ANSI-compliant C compiler is required.
 *
 * If you have gcc (GNU C), we recommend that you use it.
 * If you wish to use gcc, simply define UseGcc to YES.
 *  - This setting automatically sets the CCompiler command
 *    (so don't override it) and any other necessary options
 *
 * If you wish to use a C compiler other than the default,
 * define CCompiler to the C compiler command.  For most systems,
 * the default will be 'cc'.  Example: To use "acc" (the Sun ANSI C 
 * Compiler), use the following:
 *	# define CCompiler acc
 *
 * HPUX: Either gcc or the default "cc" command will work.  As of this
 * 	 writing, the version of gcc distributed by FSF does not support
 *	 debugging on the HP.  Either turn off "-g" below, or get the
 *	 special version at jaguar.cs.utah.edu.
 *
 * SGI:	 GCC works iff (1) you have Irix 5.2 or greater, and (2) you have 
 *	 gcc 2.6.1 or greater.  Lacking those, use of the native compiler
 *	 is highly recommended.
 */
# define UseGcc YES

/*
 * Zebra uses the RDSS UI compiler to generate load files for much of its
 * user interface.  If the UI compiler, uic, is not in your path, define
 * the full path of uic, including the executable name, in the symbol
 * UicCmd.  The default value is usually just 'uic'.  If BuildRDSS is true,
 * a default will be chosen based on UseInstalledZebra.
 */
/* # define UicCmd /rdss/bin/uic */

/* 
 * Zebra compilation now also requires a C++ compiler and (for dsmanage and
 * tapeindex) GNU's libg++.  In turn, libg++ essentially requires that the
 * C++ compiler be GNU's g++. (Known reasons are: use of the "#include_next"
 * preprocessor directive, and assumption of "bool" as a built-in type.)
 * Hence: we must use g++ and libg++.  Period.
 *
 * In general, defaults generated for finding g++ and libg++ will suffice.
 * In some instances, however, one or more of the following variables may 
 * need to be #defined below:
 *
 *	CplusplusCmd			path to g++, if other than just "g++"
 * 	CplusplusOptions		options specific to the C++ compiler
 *	CplusplusSpecialOptions		special library options for C++
 *
 * E.g., if your g++ compiler is not in your normal execution path, and lives
 * in directory /weird/bin, you need a line below of the form:
 *	# define CplusplusCmd /weird/bin/g++
 *
 * If you want debug flags for the C++ compiler and linker, define the
 * CplusplusDebugFlags macro accordingly.  The CDebugFlags setting is no
 * longer included automatically on the C++ command line.
 *
 * Caution: Enabling debugging can significantly slow compilation time and
 * increase library and executable size.  If disk space is critical, you
 * might want to consider either stripping the binaries after installation
 * or disabling debugging altogether.
 */
# define HasCplusplus	YES
# define CplusplusCmd	g++
# define CplusplusDebugFlags -g

/*
 * To set debugging flags for the FORTRAN and C compilers' command
 * lines, define the symbols CDebugFlags and FortDebugFlags.
 * The default is no debugging.  Note that for most compilers
 * (gcc is an exception), the -g and -O flags cannot be combined.
 * On Sun's, a debugging flag will disable optimization.  The FORTRAN
 * should probably always be optimized, so make sure FortDebugFlags
 * does not disable optimization by specifying any debug flags.
 *
 */
# define CDebugFlags -g
/* # define FortDebugFlags -g */

/*
 * To change other compiler command line options, such as required by your
 * compiler (floating point stuff, etc.), or optimization flags, define
 * CCOptions and FortOptions.  The default is optimization, -O.
 *
 * HP-UX: If you will be including the "Proj" map projection library and
 *	  compiling with the HP native compiler, the HP cc must be given
 *	  the ANSI "-Aa" option to be able to compile the Proj library
 *	  header file.  The CCOptions default of "-Aa" for the native
 *	  compiler should suffice.  If you change CCOptions and you are
 * 	  compiling Proj, be sure to include the -Aa option. 
 *
 * HP-UX: If you override the default for FortOptions, you must also
 *	  provide the "+ppu" option or you will get undefined externals when
 *	  you link the graphics process.  Ordinarily the default will suffice.
 *
 * IBM-AIX : If you override the default for FortOptions, you must also
 *           provide the "-qextname" option or you will get some undefined
 *           external symbols when linking.
 */

/* # define CCOptions -O */

/* # define FortOptions -O +ppu */

/* # define FortOptions -O -qextname */  /* For IBM-AIX xlf fortran */

/*
 * During compilation, Zebra's source files must be able to find lots
 * of include files.  By default, your compiler should check the
 * standard include directories like /usr/include.  If the header files
 * for any package that Zebra requires are not in some standard location,
 * the correct directories need to be defined here.  Note that even if
 * an include directory is in a compiler's default path, it may not be
 * in makedepend's. If in doubt, try the defaults first.
 * The available symbols are:
 *
 *	XIncDir		-- Location of X header files.  By default,
 *			   this will be $OPENWINHOME/include if you
 *			   are using OpenWindows.  Otherwise it will
 *			   be empty (i.e. rely on the compiler to search
 *			   the standard include directories).
 *	NetCDFIncDir    -- Location of netCDF header files.  The default
 *			   is empty (use system's default include directories).
 *	RDSSIncDir	-- RDSS include files.  The default is RDSSDir/include.
 *			   If not defined above, RDSSDir is /rdss.
 *	UDUnitsIncDir	-- Header file location for udunits.  The default is 
 *			   /usr/local/include, the standard install directory
 *			   for udunits.h.
 *
 * Note that gcc will search /usr/local/include by default, most other
 * compilers and makedepend will not.  Even if using gcc, go ahead and 
 * specify /usr/local/include if it contains needed header files.
 */

#ifdef HPArchitecture
# define XIncDir /usr/contrib/mitX11R5/include	/* HP-UX version */
#else
/* # define XIncDir /local/X11R5/include */
#endif

/* #define NetCDFIncDir /usr/local/include */

# define UDUnitsIncDir /home/zebra/include

/*
 * Zebra can build an interface to HDF (Hierarchical Data Format) satellite
 * files generated by SeaSpace's TDF-to-HDF converter.  TDF is the
 * terra-scan data format used by SeaSpace.  HDF comes from the National
 * Center for Supercomputing Applications at the University of Illinois,
 * Urbana-Champaign.  Zebra's HDF access is limited to SeaSpace satellite
 * images and will not work with general Raster, Palette, or SDS data
 * objects.  If you need to read other HDF files, let us know so that we
 * have an idea of the demand and the types of access needed.
 *
 * Source code and documentation for HDF are on
 * ftp://ftp.ncsa.uiuc.edu/HDF. Some general information on HDF, including
 * a FAQ, is available from
 * http://www.ncsa.uiuc.edu/SDG/Software/HDF/HDFIntro.html.  The Zebra
 * interface has only been tested with HDF 3.3r2.
 *
 * If you don't have HDF, just leave HasHDF as its default of NO.  Usually
 * only those working with ARM project data will need HDF support.  If you
 * have HDF and HDF satellite images you'd like to display, define HasHDF
 * to YES.  HDFIncDir should be the path to the HDF header files, and
 * HDFLibDir is the directory of the HDF library.  Or you can define
 * HDFLibrary to the full path, including the file name, of the HDF library
 * (e.g. /usr/local/ncsa/libdf.a). 
 */
#define HasHDF NO
#define HDFIncDir /usr/local/include/hdf
#define HDFLibDir /usr/local/lib

/*
 * Zebra can now cope with radar sweepfiles as well.  Unless you have
 * the needed libraries, and you know that you want to use this capability,
 * your best bet is to leave it disabled.
 *
 * If enabled, use SweepFileIncDir to say where the dorade library
 * includes live, and SweepFileLibrary to specify the library itself
 * (or SweepFileLibDir to just give the directory).
 */
# define HasSweepFiles NO
# define SweepFileIncDir /scr/lucy/spol/include
# define SweepFileLibrary /scr/lucy/spol/ddutils/ddmlib.a

/*
 * During linking, the linker must be told where to find the libraries 
 * for the various packages which Zebra requires.  The locations of these
 * libraries can be defined in the following parameters.  All of these
 * default to empty, meaning that the libraries will be found in the
 * system's default library search path.  If any of these are defined, then
 * they will automatically be included in a -L option to the compiler,
 * unless they are overridden by XLibraries, NetCDFLibrary, or RDSSLibrary
 * below.
 *
 * 	XLibDir		-- Location of X libraries.
 *	NetCDFLibDir    -- Location of the netCDF library.
 *	RDSSLibDir	-- RDSS Library directory.  IF NOT INSTALLED IN
 *			   /usr/local/lib OR /usr/lib, DEFINE A DIRECTORY
 *			   BELOW.
 *	UDUnitsLibDir	-- Location of the udunits library.
 *
 * WARNING: If any of the following libraries are in /usr/lib and you are
 * building on a Sun, DO NOT specify /usr/lib explicitly.  Otherwise you
 * will run into extremely weird linking problems involving the fact that
 * Sun provides at least three math libraries and you HAVE to get the right
 * one.
 */

# define XLibDir /usr/X11R6/lib
/* # define XLibDir /usr/contrib/mitX11R5/lib */ /* for HPUX */

# if !BuildRDSS
/* # define RDSSLibDir /rdss/lib */		/* common RDSS installation */
/* # define RDSSLibDir /usr/local/lib */	/* not needed with gcc */
# endif

/* # define NetCDFLibDir /usr/local/lib */	/* not needed with gcc */
# define UDUnitsLibDir /home/zebra/lib

/*
 * To specify the whole command-line option for a library use the symbols
 * XLibraries, NetCDFLibrary, UDUnitsLibrary, RDSSLibrary.  For example, to 
 * use a version of RDSS which has a different name, use 
 *
 * #define RDSSLibrary -L/usr/local/lib -lrdssui2 -lrdssutil2
 *
 * which on most UNIX systems is equivalent to
 *
 * #define RDSSLibrary /usr/local/lib/librdssui2.a \
 * 		       /usr/local/lib/librdssutil2.a
 *
 * Note that defining one of the 'Library' symbols as above will completely
 * circumvent the definition of the corresponding 'LibDir' symbol.  
 * FOR ALMOST ALL INSTALLATIONS, it should only be necessary to specify
 * library directories via the XLibDir, RDSSLibDir, and NetCDFLibDir symbols,
 * meaning the symbols XLibraries, NetCDFLibrary, and RDSSLibrary do not
 * need to be defined here. (The names of the libraries will be added
 * automatically to the LibDir symbols.)
 *
 * If you need to define the actual command-line option for specifying
 * some or all of the libraries, you can change several parameters.
 * All of these parameters will default to the specified library directory
 * (such as those that may have been defined above) and the standard
 * name of the library: e.g. -L<libdir> -l<libname>.  The default is
 * listed to the right of each of the parameter names.
 *
 * For these 4 libraries, it is unlikely that the defaults will not be
 * sufficient:
 *
 *	XLibraries	-lXaw -lXmu -lXt -lXext -lX11
 *	NetCDFLibrary	-lnetcdf
 *	UDUnitsLibrary	-ludunits
 * 	RDSSLibrary	-lrdssui -lrdssutil
 *
 * The following 2 libraries will probably need to be explicitly
 * defined here, but ONLY if you need them for ingest modules.
 *
 * The SUDS library is only needed for the PAM and CLASS ingest modules.
 *
 *	SudsLibrary	$(RDSSDIR)/suds/libsuds.a
 *
 * The 'PAM configuration library', libunp.a, is only needed by
 * the PAM ingest module.
 *
 *	PamCfgLibrary	-lunp
 *
 * If you don't need to ingest any PAM or CLASS data (which will be the
 * case 99% of the time), you can safely ignore the definition of the
 * two symbols above.
 */

/*
 * This one works if the RDSS source tree is in /rdss and has not been cleaned
 */
/* # define PamCfgLibrary /rdss/pam/cfg/access/libunp.a */

/*
 * This one is for RDSS libraries installed into /usr/local/lib
 */
/* # define PamCfgLibrary /usr/local/lib/libunp.a */

/*
 * If you have a nonstandard Fortran compiler (i.e. you "make" won't figure
 * it out automatically) set it here.  (This example for the GNU compiler).
 * If you are compiling under Linux, the appropriate FORTRAN compiler and
 * libraries are given defaults further down in this file, and you don't
 * need to uncomment this definition.
 */
/* # define FortranCompiler g77 */

/*
 * FORTRAN libraries tend to be very system-specific.  Define this
 * parameter to be the command-line options necessary to link with
 * the FORTRAN libraries on your system.  If not using g77 or Suns,
 * the defaults below are probably fine.  Otherwise, uncomment the
 * g77 example definition below, or choose the correct definition
 * from among the examples for Sun architectures.
 *
 * Fortran libraries are no longer needed to build the graphics process.
 * They are only necessary if building the data utilities or certain
 * ingestors, such as the satellite GOES and GVAR ingestors.
 *
 * Similar to FortranCompiler, FortranLibraries has a default for
 * Linux defined below.
 */

/* G77 -- may also have to give -L if not linking with gcc */
/* # define FortranLibraries -lf2c */

#ifndef FortranLibraries
#ifdef SunArchitecture
/*
 * Change the FortranLibraries definition below if you are not using the
 * SC4.2 version of the Fortran compiler.
 */
#ifndef FortranLibraries
# define FortranLibraries -L/opt/SUNWspro/SC4.2/lib -lF77 -lV77 -lM77 -lsunmath
#endif

#else
#ifdef HPArchitecture /* HP-UX */

# define FortranLibraries -lU77 -lf -lisamstub

#else
#ifdef SGIArchitecture /* IRIX 5.x */

# define FortranLibraries -lF77 -lI77 -U77 -lisam

#else
#ifdef LinuxArchitecture

# define FortranCompiler g77
# define FortranLibraries -lf2c

#else 
#ifdef OSF1Architecture
# define FortranCompiler f77
# define FortranLibraries -lfor -lFutil -lUfor -lots
 
#else
#ifdef AIXArchitecture

# define FortranCompiler xlf
# define FortranLibraries -lxlf -lxlf90
 
#endif /* AIX */
#endif /* OSF */
#endif /* Linux */
#endif /* SGI */
#endif /* !HP */
#endif /* !Sun */
#endif /* !FortranLibraries */

/*
 * Some of Zebra's routines require the terminal capability library,
 * usually /usr/lib/libtermcap.a, and the math library, /usr/lib/libm.a.
 * These are the symbols and their defaults:
 *
 *	MathLibrary	-lm
 *	TermcapLibrary	-ltermcap
 *
 * To change these definitions, use your compiler's options for 
 * specifying libraries on the command line, such as -L for 
 * search directories and -l for library names. Linux systems may
 * need -lncurses instead of -ltermcap.
 */

# define MathLibrary	-lm
# define TermcapLibrary -ltermcap

/*
 * Building the radar ingestor now requires the PCAP library from
 * ftp.ee.lbl.gov.  If you want this module be sure that the symbol
 * below is set such that the linker will find the library; if you're
 * not using the radar ingestor (99.9% of all Zebra users), you can ignore
 * all this.
 */
# define PCAPLibrary 	-lpcap
# define PCAPIncludes 	-I/usr/local/include

/*
 * The graphics process can be built to do map projections.  Projections
 * require the proj library, produced by the U.S. Geological Survey, and a
 * C compiler which supports ANSI prototypes, such as gcc.  The proj library
 * may be obtained from the ATD FTP site, or from charon.er.usgs.gov.  We
 * thank Gerald Evenden of USGS for allowing us to use and redistribute
 * this library.
 *
 * To use projections, set MapProjections to YES and ProjLibrary to the
 * appropriate value. 
 */
# define MapProjections YES
# define ProjLibDir /home/zebra/lib
# define ProjIncDir /home/zebra/include
	
/*
 * Examples of command-line options which override the above definitions
 */
/* # define ProjLibrary -L/usr/local/proj/lib -lproj */
/* # define ProjIncludes -I/usr/local/proj/include */

/*========================================================================
 * Distribution Options
 *------------------------------------------------------------------------
 * This section of definitions is meant for those who are creating a
 * customized Zebra distribution.  If you are only installing Zebra at your
 * local site, then you are finished configuring Zebra.  Save this file and
 * continue with the installation instructions.  Otherwise, read on.
 *
 * Zebra's Imakefiles contain several built-in features meant for creating
 * and customizing Zebra distribution trees.  In particular, here are some
 * important targets to be familiar with:
 *
 * distfiles::		Echoes to the terminal the path names of all files
 *			which are meant to be distributed in a directory
 *			and all of the subdirectories.  By setting the
 *			DISTCURDIR variable on the command line, all of the
 *			file names printed will be prefixed with the value
 *			of the variable.  See the tarfile: target in the
 *			top-level Imakefile for an example of how this can
 *			be used.
 *
 * distmakefiles::	Makes Makefiles for ALL of the distribution 
 *			directories, not just those which have been "turned
 *			on" to be built.  This is necessary before performing
 *			other dist targets in order to create the Makefile
 *			which the dist targets will operate on.
 *
 * distclean::		Cleans in ALL directories in the distribution.  This
 *			is useful when programs have been compiled
 *			explicitly from within a directory which has been
 *			disabled in the regular distribution tree.  From the
 *			top-level, this target also removes the ./bin and
 *			./lib directories, and cleans in ./imake, so use
 *			it with caution.
 *
 * In addition to the symbols above which control which parts of a 
 * distribution are being compiled, other symbols control which parts
 * belong to the distribution:
 *
 * DistributeIngest	If YES, zebra/src/ingest is expected to be in the
 *			distribution, and all of its files and directories
 *			will be included in any distfiles operations.
 *
 * DistributeOptimzer	If YES, zebra/src/Optimizer is expected to be in the
 *			distribution tree.
 *
 * DistributeDataUtils	YES when the zebra/src/datautil directory is part 
 *			of the distribution.
 *
 * DistributeXHelp	If YES, the makefile in zebra/src tries to build
 *			and install in zebra/src/xhelp.  Otherwise, the
 *			xhelp directory can be removed without harm.  Use
 *			this to build distributions which will always
 *			use the Mosaic help interface.
 *
 * Therefore, if you wanted to create a distribution which did not contain
 * the ingest directory, Optimizer, data utilities, map utilities, or xhelp, 
 * uncomment the following lines:
 */

/* #define DistributeIngest 	NO */
/* #define DistributeOptimizer 	NO */
/* #define DistributeDataUtil 	NO */
/* #define DistributeMapUtil 	NO */
/* #define DistributeXHelp	NO */

/* Next, update ALL of the distributed Makefiles with 'make distmakefiles'.
 * Now you have two options:
 *
 *	Use the top-level tarfile target to create a distribution tar file,
 *	which will NOT contain ingest, Optimizer, datautils, or xhelp.
 *
 * 	Explicitly remove the ingest, Optimizer, datautils, and xhelp
 * 	directories from your distribution.  As long as the above symbols
 * 	are NO, the Imakefiles will never try to access those directories,
 * 	even for any of the dist* targets.  If using a "pruned"
 * 	distribution tree, make sure the symbols always stay set
 * 	appropriately.
 */
 
/* One more symbol is also available: */

/* #define DistributeRealtimeDataStoreTools NO */

/* Any of the distribution symbols which are not defined default to YES.
 * Questions about any of this can go to granger@ncar.ucar.edu
 * Also see the chapter on Imake in the Zebra Developer's Guide.
 */

/*========================================================================*/
# endif /* MAKING_MAKEFILE */

/*========================================================================
 * Selecting Graphics Process Plot Types
 *------------------------------------------------------------------------
 * The following definitions allow you to configure in or out various
 * pieces of the Zebra graphics process.  There are very few cases in which
 * it is desirable to change any of the following.
 *------------------------------------------------------------------------*/
/*
 * --- The various plot types ---
 */
# define C_PT_CAP	YES		/* Constant altitude plots	*/
# define C_PT_SKEWT	YES		/* Skew T plots			*/
# define C_PT_XSECT	YES		/* Cross section plots		*/
# define C_PT_TSERIES	YES		/* Time series plots		*/
# define C_PT_XYGRAPH	YES		/* XY-Graph plots		*/
# define C_PT_XYWIND	YES		/* XY-Graph wind plots		*/
# define C_PT_HISTOGRAM	YES		/* Histograms			*/
# define C_PT_THETAPLOT	YES		/* Theta plots			*/

/*
 * --- CAP subplots ---
 *
 * Define these YES if you want them.  If C_PT_CAP above is not YES, then
 * ALL of these definitions will be ignored and none of these plots will
 * be built.  Again, it is rare that anyone would want or need to define
 * any of these to NO.
 *
 * The one possible exception is POLAR, which enables the "polar"
 * representation and contours of polar data.  This is probably not useful
 * to the bulk of people out there.  Polar plots currently only work from
 * sweepfile data, so you will need to have enabled sweepfile access as
 * well.
 */
# if C_PT_CAP
#	define C_CAP_OVERLAY	YES	/* Overlays			*/
#	define C_CAP_VECTOR	YES	/* Vector plots			*/
# 	define C_CAP_LIGHTNING	YES	/* Lightning location		*/
#	define C_CAP_RASTER	YES	/* Raster plots			*/
#	define C_CAP_TRACKS	YES	/* Track plots (e.g. aircraft)	*/
# 	define C_CAP_POLAR	NO	/* Plots of polar data		*/
# endif
/*------------------------------------------------------------------------*/


/*========================================================================
 * Customization Options
 *------------------------------------------------------------------------
 * Several parameters used throughout the Zebra distribution can be
 * modified by setting them here rather than changing them in all of the
 * include files.  The intention is that certain projects can keep their
 * settings in this file and turn them on by defining a single symbol, like
 * ARM_PROJECT for the ARM project.  All of the symbols defined here have
 * the prefix 'CFG_'.  If a symbol is not defined, then the include files
 * use their own defaults.
 */

/* #define ARM_PROJECT		*/
/* #define NEXUS_PROJECT	*/

/*------------------------------------------------------------------------
 * //////////////////// Settings for the ARM project /////////////////////
 *------------------------------------------------------------------------*/
#ifdef ARM_PROJECT

/* 
 * The default altitude units for datachunks and for netCDF files whose alt
 * field does not have a units attribute (or has a blank one).  If the
 * altitude units of a datachunk are not explicitly overridden, this will
 * be the units which are stored in the "units" attribute of the "alt"
 * field in netCDF files.
 */
#define CFG_ALTITUDE_UNITS	AU_mMSL

#define DistributeIngest 	NO
#define DistributeOptimizer 	NO

#endif /* ARM_PROJECT */
/*------------------------------------------------------------------------*/


/*------------------------------------------------------------------------
 * //////////////////////// Settings for NEXUS ///////////////////////////
 *------------------------------------------------------------------------*/
#ifdef NEXUS_PROJECT

#define DistributeIngest 	NO
#define DistributeOptimizer 	NO
#define DistributeDataUtil 	NO
#define DistributeRealtimeDataStoreTools YES

#endif /* NEXUS_PROJECT */
/*------------------------------------------------------------------------*/


/*------------------------------------------------------------------------*/
/* ------------------ Defaults for compile-time constants ----------------*/
/*------------------------------------------------------------------------*/
/* The definitions below will not usually need to be changed, but they are
 * provided here so that they can be conveniently found and changed in
 * single file.  DO NOT EDIT THESE UNLESS YOU KNOW WHAT YOU ARE DOING!  If
 * any of these definitions are unreasonable or downright incorrect, you
 * may not be able to compile or run Zebra.
 **************************************************************************/

/* #define CFG_FULL_YEARS */	/* Use four-digit years in netcdf file names */
/* #define CFG_WARN_FIELD_NAMES *//* Warn about CDL-illegal field names */

/*
 * If defined, the 'alt' field will not be given a 'units' attribute in
 * netCDF files.
 */
/* #define CFG_NC_NO_ALT_UNITS */

/*
 * Various string length limits
 */
#ifndef CFG_PLATNAME_LEN
#define CFG_PLATNAME_LEN 100	/* Len of platform names		     */
#endif
#ifndef CFG_DIMNAME_LEN
#define CFG_DIMNAME_LEN	32	/* Longest allowable dimension name	     */
#endif
#ifndef CFG_PDPARAM_LEN
#define CFG_PDPARAM_LEN	40	/* Length of parameter names	*/
#endif
#ifndef CFG_ASCTIME_LEN
#define CFG_ASCTIME_LEN	40	/* Length needed for encoded times*/
#endif
#ifndef CFG_PLATCLASS_LEN
#define CFG_PLATCLASS_LEN 80	/* Len of platform class names		     */
#endif
#ifndef CFG_FILENAME_LEN
#define CFG_FILENAME_LEN 32	/* Len of general file names (not full paths)*/
#endif
#ifndef CFG_DATAFILE_LEN
#define CFG_DATAFILE_LEN 50	/* Len of data file names (not full paths)   */
#endif
#ifndef CFG_FILEPATH_LEN
#define CFG_FILEPATH_LEN 256	/* Len of full path filenames		     */
#endif
#ifndef CFG_PDNAME_LEN
#define CFG_PDNAME_LEN	40	/* Len of name of a plot description table   */
#endif
#ifndef CFG_MSGNAME_LEN
#define CFG_MSGNAME_LEN	64	/* Max length of a message client name	     */
#endif
#ifndef CFG_MSGEVENT_LEN
#define CFG_MSGEVENT_LEN 200	/* Length of extended logger event messages  */
#endif
#ifndef CFG_FIELD_NAME_LEN
#define CFG_FIELD_NAME_LEN 40	/* Length of field names		     */
#endif
#ifndef CFG_FIELD_LONG_LEN
#define CFG_FIELD_LONG_LEN 256	/* Length of field description (long name)   */
#endif
#ifndef CFG_FIELD_UNITS_LEN
#define CFG_FIELD_UNITS_LEN 40	/* Length of field units string		     */
#endif
#ifndef CFG_SEARCHPATH_LEN
#define CFG_SEARCHPATH_LEN 512	/* Length of exec search paths and such	     */
#endif

/*
 * Maximum number of fields (field IDs) in the fields table
 */
#ifndef CFG_FIELD_MAX_ID
#define CFG_FIELD_MAX_ID	512
#endif

/*
 * Maximum number of platforms (including subplatforms)
 */
#ifndef CFG_MAX_PLATFORMS
#define CFG_MAX_PLATFORMS 50000
#endif

/*
 * DataChunk symbols
 */
#ifndef CFG_DC_MAXFIELD
#define CFG_DC_MAXFIELD	255	/* Maximum allowable fields in one datachunk */
#endif
#ifndef CFG_DC_MF_HASH_SIZE
#define CFG_DC_MF_HASH_SIZE 256	/* Least power of 2 > (not =) CFG_DC_MAXFIELD*/
#endif
#ifndef CFG_DC_MAXDIMN
#define CFG_DC_MAXDIMN	30	/* Max number of dimensions for NSpace chunk,*/
                                /* should be less than netCDF limit	     */
#endif
#ifndef CFG_DC_DEFAULT_BADVAL
#define CFG_DC_DEFAULT_BADVAL -99999.	/* Default floating poing bad value  */
		/* WARNING: Make sure this value contains a decimal point!   */
#endif

/*
 * Plot desctiption limits
 */
#ifndef CFG_PD_MAXCOMP
#define CFG_PD_MAXCOMP	50	/* Maximum # components expected in a PD     */
#endif
#ifndef CFG_PD_RAWTEMP
#define CFG_PD_RAWTEMP	40960	/* Raw temp buffer space for raw PD's	     */
#endif

/*
 * Message manager limits
 */
#ifndef CFG_MSG_MAXBCAST
#define CFG_MSG_MAXBCAST 1500	   /* Maximum length of a broadcast message  */
#endif
#ifndef CFG_MSG_DEFAULT_PORT
#define CFG_MSG_DEFAULT_PORT 1500  /* Default tcp port			     */
#endif
#ifndef CFG_MSG_SOCKET_NAME
#define CFG_MSG_SOCKET_NAME "/tmp/fcc.socket"
                         /* UNIX domain socket name in file system namespace */
#endif

/*
 * Display manager limits
 */
#ifndef CFG_DM_CODELEN
#define CFG_DM_CODELEN	20	/* Length of a DM event code	*/
#endif
#ifndef CFG_DM_MAXADATA
#define CFG_DM_MAXADATA 128	/* Length of DM action data	*/
#endif

/*
 * Graphics process limits
 */
#ifndef CFG_GP_MAX_CONTOURS
#define CFG_GP_MAX_CONTOURS 50	/* Contour levels allowed without aborting */
#endif

/*
 * Other parameters
 */
#ifndef CFG_ALTITUDE_UNITS
#define CFG_ALTITUDE_UNITS 	AU_kmMSL /* Default units for "alt" field */
#endif
/*------------------------------------------------------------------------*/



/*-//////////////////////////////////////////////////////////////////////-*/
/*========================================================================
 * ////////    You are now finished configuring Zebra!     ///////////////
 * ////////           DO NOT EDIT ANY FURTHER!	           ///////////////
 * ////////     Save this file and continue with the	   ///////////////
 * ////////         installation instructions...	   ///////////////
 *========================================================================*/
/*-//////////////////////////////////////////////////////////////////////-*/



/*------------------------------------------------------------------------*/
/*
 * These are always NO -- do not change them!
 */
# if !C_PT_CAP
#	define C_CAP_OVERLAY	NO	/* Overlays			*/
#	define C_CAP_VECTOR	NO	/* Vector plots			*/
# 	define C_CAP_LIGHTNING	NO	/* Lightning location		*/
#	define C_CAP_RASTER	NO	/* Raster plots			*/
#	define C_CAP_TRACKS	NO	/* Track plots			*/
# 	define C_CAP_POLAR	NO	/* Plots of polar data		*/
# endif

/*
 * Some parameters for dealing with buffer lengths.
 */
# define MAX_PLAT_LEN	160	/* Maximum platform len -- remember lists */
# define MAX_PARAM_LEN	40	/* Length of parameter names		*/
# define TIME_LEN	40	/* Length needed for encoded times	*/

/* --- Some obsolete configuration symbols put here to be forgotten --- */
/* --- and to simplify the installation process                     --- */

/*
 * The PD monitor utility is an emacs program which provides interactive
 * run-time editing, checking, and monitoring of graphics processes' plot
 * descriptions.  If you have emacs (not version 19), and wish to compile
 * the emacs lisp code for the PD monitor, define HaveEmacs to YES and make
 * sure EmacsPath points to the emacs program.  The default is NO.  Even if
 * the elisp code is not compiled and installed, the elisp source files
 * will still be installed, so these can be used as is or byte-compiled
 * later.  If you have Emacs 19, the elisp code cannot be batch compiled,
 * so set HaveEmacs to NO. The Emacs 19 lisp file will still be installed.
 */
# define HaveEmacs NO
# define EmacsPath emacs

# endif /* ! _zebra_config_h_ */
