#!/bin/env python
#
# Parse aircraft location info for G-LUXE (FAAM's BAE-146) aircraft,
# obtained from http://www.faam.ac.uk/public/gluxe_position/posrep.html
# by default.  Pass the parsed lines to Zebra's actrack ingestor.
#

import sys
import os
import re
import calendar
import datetime
import urllib
import HTMLParser

FeetToKilometers = 0.0003048

class FAAM_Parser(HTMLParser.HTMLParser):
    def __init__(self):
        HTMLParser.HTMLParser.__init__(self)
        self.inTable = 0
        self.inRow = 0
        self.columns = []
        self.badval = sys.maxint

    def doURL(self, url):
        self.feed(urllib.urlopen(url).read())
        
    def handle_starttag(self, tag, attrs):
        if (tag == "tr"):
            if (self.inRow):
                self.handle_endtag("tr")
            self.inRow = 1
            self.columns = []
        
        elif (tag == "td"):
            self.columns.append("")  # add an empty column entry

    def handle_endtag(self, tag):
        # end of table
        if (tag == "table" and self.inRow):
            self.handle_endtag("tr")
        # end of row
        elif (tag == "tr"):
            self.processRow()
            self.inRow = 0

    def processRow(self):
        #
        # Skip the header row, which gives us zero columns
        #
        if (len(self.columns) == 0):
            return
        time = self.getTime(0)
        latitude = self.getLatitude(2)
        longitude = self.getLongitude(4)
        altitude = self.getAltitude(6)
        if (time == None or latitude == None or longitude == None or
            altitude == None):
            return

        os.system("/opt/zebra/bin/actrack faam_bae146 %f %f %f %d" %
                  (latitude, longitude, altitude, time))

    #
    # return aircraft latitude, in decimal degrees, from the given column
    #
    def getLatitude(self, column):
        #
        # parse latitude, e.g., " 52&deg;04.0W"
        #
        m = re.match(" *(\d*)&deg;(\d*\.?\d*)([NS])", self.columns[column])
        if (m == None or len(m.groups()) < 3):
            sys.stderr.write("Bad latitude '%s'\n" % self.columns[column])
            return None
        
        latdeg = float(m.group(1))
        latmin = float(m.group(2))
        ns = m.group(3)
        lat = latdeg + latmin / 60
        if (ns == "S"):
            lat *= -1
        return lat

    #
    # return aircraft longitude, in decimal degrees, from the given column
    #
    def getLongitude(self, column):
        #
        # parse longitude, e.g., " 0&deg;37.2W"
        #
        m = re.match(" *(\d*)&deg;(\d*\.?\d*)([EW])", self.columns[column])
        if (m == None or len(m.groups()) < 3):
            sys.stderr.write("Bad longitude '%s'\n" % self.columns[column])
            return None
        
        londeg = float(m.group(1))
        lonmin = float(m.group(2))
        ew = m.group(3)
        lon = londeg + lonmin / 60
        if (ew == "W"):
            lon *= -1
        return lon

    #
    # return aircraft altitude, in km, from the given column
    #
    def getAltitude(self, column):
        #
        # parse altitude, e.g., "   534 ft"
        #
        m = re.match(" *(\d*) ft", self.columns[column])
        if (m == None or len(m.groups()) < 1):
            sys.stderr.write("Bad altitude '%s'\n" % self.columns[column])
            return None
        
        alt = float(m.group(1)) * FeetToKilometers
        return alt

    #
    # return time of this point, in seconds since 1-Jan-1970 00:00 UTC,
    # from the given column
    #
    def getTime(self, column):
        #
        # parse time, e.g., "  16/12/04 1010Z"
        # 
        m = re.match(" *(\d*)/(\d*)/(\d*) (\d*)Z", self.columns[column])
        if (m == None or len(m.groups()) < 4):
            sys.stderr.write("Bad time '%s'\n" % self.columns[column])
            return None
        
        day = int(m.group(1))
        month = int(m.group(2))
        year = int(m.group(3)) + 2000

        hhmm = int(m.group(4))
        hour = hhmm / 100
        minute = hhmm % 100

        dt = datetime.datetime(year, month, day, hour, minute)
        return calendar.timegm(dt.utctimetuple())

    def handle_entityref(self, name):
        self.handle_data("&" + name + ";")
        
    def handle_data(self, data):
        columnNum = len(self.columns) - 1
        if (self.inRow and columnNum >= 0):
            self.columns[columnNum] += data

if __name__ == '__main__':
    parser = FAAM_Parser()
    
    urls = sys.argv[1:]
    if (len(urls) == 0):
        urls = ["http://www.faam.ac.uk/public/gluxe_position/posrep.html"]
        
    for url in urls:
        try:
            parser.doURL(url)
        except IOError, (errno, strerror):
            sys.stderr.write("Error %d (%s) opening %s\n" % (strerror, url))
