#!/bin/csh
#
# Script to run mdv2netCDF converters to get lat/lon netCDF 
# satellite data for BAMEX.
#
# The program will remap the data to a lat/lon projection and
# reformat it to netCDF.
#
# This should be run on a machine that can ping anlab2.rap.ucar.edu.
#
# The files mdv2netCDF.bamex_IR and mdv2netCDF.bamex_Vis control
# the settings for the program for the IR and the visible data,
# respectively.
#
/code/burghart/mdv2netCDF \
	-params /code/burghart/bamex/scripts/mdv2netCDF.bamex_IR >& \
	/code/burghart/bamex/mdv2netCDF_bamex_IR.log &
/code/burghart/mdv2netCDF \
	-params /code/burghart/bamex/scripts/mdv2netCDF.bamex_Vis >& \
	/code/burghart/bamex/mdv2netCDF_bamex_Vis.log &