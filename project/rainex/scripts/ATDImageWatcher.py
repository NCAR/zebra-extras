#!/net/opt_lnx/local_rh90/bin/python2.3
#$Id: ATDImageWatcher.py,v 1.1.1.1 2005-08-04 19:30:56 burghart Exp $
import os
import sys
import time
import string
import signal
import shutil

from twisted.web import xmlrpc, server
from twisted.internet.interfaces import IReadDescriptor
from twisted.internet import reactor
from ftplib import *

"""
Directory Watcher class that processes files in a directory,
and provides a status reporting (XML-RPC based) function,
so a status monitoring program can determine the last file
processed (and when the file was processed)

"""

class Dir_Status(xmlrpc.XMLRPC):
    """An directory status object to be published."""
    def __init__(self):
	self._last_processed = ""
	self._last_time = 0

    def xmlrpc_echo(self, x):
        """Return all passed args."""
        return x

    def xmlrpc_status(self):
        """Return last file processed."""
#	print 'Dir_Status: status returns : %s:%d '% (self._last_processed, self._last_time)
        return (self._last_processed,self._last_time)

    def set_last_processed(self, file):
#	print 'Dir_Status: setting last processed file = ', file
        self._last_processed = file
	self._last_time = time.time()

#class FAM_Reader(IReadDescriptor):
class FAM_Reader:
    _shared_state={}
    """ Implement a FAM connection using the Twisted framework"""
    def __init__(self):
        # use the Borg "un-pattern", p208  of the "Python Cookbook" - since we only
        # want/need  a single connection to FAM
        self.__dict__ = self._shared_state
        import _fam
        if not self.__dict__.has_key("_fam_connection"):
            self._fam_connection = _fam.open()

    def logPrefix(self):
	"required method"
	return "FAM_Reader"
    
    def _process_fam_events(self, fe):
        if fe.userData:     
            fe.userData.handleEvent(fe)
        else :
            print 'Event w/o user data'
            print fe.filename, fe.code2str()

    def doRead(self):
	"called when fam connection has data to read"
        while self._fam_connection.pending():
            try :
                fe = self._fam_connection.nextEvent()
                self._process_fam_events(fe)       
            except IOError, er:
                # sometimes we get "unable to get next event"
#                errnumber, strerr = er
#                print "FAM_Reader.doRead() ", strerr
                print "FAM_Reader.doRead() ", er
                # attempt to recover - otherwise we may just call
                # this repeatedly
                _fam.close()
                self._fam_connection = _fam.open()
		return

    def fileno(self):
	"return the file number of this FAM connection"
        return self._fam_connection.fileno()

    def connection(self):
	"return the fam connection"
	return self._fam_connection
    
    def connectionLost(self, args):
	"required method"
        print 'lost FAM connection'

    def closeConnection(self):
        _fam.close()

class Dir_Watcher:
    """Monitor a directory for new files"""

    def __init__(self, dir, only_new = 0, see_dirs = 0, verbose=1):
        """ 
	dir - which directory to watch
	only_new - only notify for  newly created files
        """
        # create an XML-RPC status report server
        status = Dir_Status()
        # register the XML-RPC server on any available port
        listener = reactor.listenTCP(0, server.Site(status))
        if (verbose):
            print "listening on port", listener.getHost().port

        # register the FAM client
        fam_rdr = FAM_Reader()
        reactor.addReader(fam_rdr)
        self._dir = dir
        self._fam_request = \
            fam_rdr.connection().monitorDirectory(self._dir, self)
        self._status = status
        self._only_new = only_new
        self._see_dirs = see_dirs
        self._verbose=verbose

    def handleEvent(self, event):
        # we don't care about source directory or subdirectories,
        # only about src directory contents
        fullname = os.path.join(self._dir, event.filename)
        if os.path.isdir(fullname) and not self._see_dirs : 
            return
        eventCode = event.code2str()
        if eventCode == 'created':
            if self._verbose: print 'new file ', fullname
            self._processFile(event.filename)
        elif eventCode == 'exists' and not self._only_new:
            if self._verbose: print 'existing file ', fullname
            self._processFile(event.filename)
#        else:
#            print "handleEvent: ", event.filename, event.code2str()

    def _processFile(self, tailName):
        srcpath = os.path.join(self._dir, tailName)
	self._status.set_last_processed(srcpath)

class FtpJOSSCatalog:
    """ ftp images to the JOSS catalog site."""

    def __init__(self, projectName, verbose = 0):
        self._projectName = projectName
        self._verbose    = verbose
        
    def sendFile(self, srcName, platform, yyyymmddhhmm, product, nTries = 1):


        root, ext = os.path.splitext(srcName) # use the ext from the src file
        destName = 'research.%s.%d.%s%s' % (platform, yyyymmddhhmm,
                                            product, ext)
        
        print "ftp'ing %s to JOSS %s catalog as %s" % (srcName,
                                                       self._projectName,
                                                       destName)
    
        try:
            f = FTP('ftp.joss.ucar.edu')
            result = f.login(user='anonymous', passwd='burghart@ucar.edu')
            print result
            result = f.set_pasv(0)
            print result
            result = f.cwd('pub/incoming/catalog/' + self._projectName)
            print result
            result = f.storbinary("STOR " + destName, open(srcName, "rb"),
                                  1024)
            print result
            f.quit();

        except Exception, ftpExcept:
            print "Exception caught from FTP: ", ftpExcept

            nTries -= 1
            if nTries:
                print "Trying again to FTP %s " % srcName
                self.sendFile(srcName, platform, yyyymmddhhmm, product, nTries)
            else:
                raise ftpExcept



class ATDImageWatcher(Dir_Watcher):
    """Deal with images sent from S-Pol, ftp'ing some to the JOSS
       catalog, and moving some into the ATD web page"""


    #
    # New image watcher, watching srcDir.  If timeout is non-zero, exit
    # the given reactor after timeout seconds of inactivity.  If doneDir
    # is defined, files that we act on are moved there when we're done;
    # otherwise, they are removed.
    #
    def __init__(self, srcDir, timeout=0, reactor=None, doneDir=None):
        Dir_Watcher.__init__(self, srcDir, only_new = 0)
        self._srcDir = srcDir
        self._timeout = timeout
        self._reactor = reactor
        
        #
        # Save the "done" directory, if any, and make sure it exists
        #
        self._doneDir = doneDir
        if (self._doneDir):
            try:
                os.makedirs(self._doneDir)
            except OSError, err:
                if err.errno != 17:  # 'File exists'
                    raise OSError, err
            
        self._JOSSCatalog = FtpJOSSCatalog("name")
        if (self._timeout):
            signal.signal(signal.SIGALRM, self._handleTimeout)
            signal.alarm(self._timeout)


    def _processFile(self, fileName):
        #
        # Reset our timer
        #
        if (self._timeout):
            signal.alarm(self._timeout)

        #
        # Full path for the file
        #
        srcPath = os.path.join(self._srcDir, fileName)
	self._status.set_last_processed(srcPath)

        actedOnImage = 0

        #
        # Split the file name on '.' characters
        #
        namePieces = string.split(fileName, ".")
        imgPrefix = namePieces[0]

        #
        # map from image name prefix to JOSS catalog product names
        #
        jossDict = { "dbz_auto":"base-reflectivity",
                     "vr_auto":"base-radial-velocity",
                     "precip_auto":"hourly-precip" }
        #
        # FTP image to JOSS if the prefix is in the catalog above
        #
        ftpFailed = 0
        
        if (jossDict.has_key(imgPrefix)):
            productName = jossDict[imgPrefix]
            yyyymmddhhmm = int(namePieces[1])
            actedOnImage = 1
            try:
                self._JOSSCatalog.sendFile(srcPath, "S-Pol", yyyymmddhhmm,
                                           productName, nTries = 2)
            except Exception, ftpExcept:
                ftpFailed = 1
                print("Failed to send %s to JOSS catalog: " % srcPath,
                      ftpExcept)

        #
        # Change the name of dBZ images to comply with Bob Rilling's naming
        # convention, then execute his Perl script on the image.  This
        # assumes that incoming dbz_auto images are all of SUR scans at 0.8
        # degrees
        #
        if (imgPrefix == "dbz_auto"):
            yyyymmddhhmm = int(namePieces[1])
            root, ext = os.path.splitext(fileName) # keep the same extension
            newName = 'SPOL_%d00_0.8_SUR_DBZ%s' % (yyyymmddhhmm, ext)
            newPath = os.path.join("/tmp", newName)
            print "making link %s -> %s" % (newPath, srcPath)

            os.symlink(srcPath, newPath)
            os.system("/net/opt_lnx/local_rh90/bin/update_SPOL_images.pl " +
                      newPath)
            os.remove(newPath)
            actedOnImage = 1

        #
        # If we did something with the file, move it into the "done"
        # directory or remove it
        #
        if (actedOnImage):
            if (not ftpFailed):
                #
                # Now move the file to the "done" directory or remove it
                #
                if (self._doneDir):
                    newPath = os.path.join(self._doneDir, fileName)
                    print 'moving %s to %s' % (srcPath, newPath)
                    shutil.copy(srcPath, newPath)
                    os.remove(srcPath)
                else:
                    print 'removing %s' % srcPath
                    os.remove(srcPath)
        else:
            print "Ignored file", fileName

    def _handleTimeout(self, signum, stackFrame):
        print 'Timeout: no activity for %d seconds' % self._timeout
        if (self._reactor):
            self._reactor.stop()
            

                 

if __name__ == '__main__':
    if (len(sys.argv) > 1):
        dir = sys.argv[1]
    else:
        dir = '/scr/incoming/spol'

    #
    # We need to use timeout here, until fam works on NFS-mounted
    # directories.
    #
    ATDImageWatcher(dir, timeout=30, reactor=reactor,
                    doneDir='/scr/hallertau/burghart/name2004_done')

    reactor.run()
