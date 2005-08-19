//
// Dump the latest time for a given NOAA LF netCDF file
//
# include <netcdf.hh>

main()
{
  NcError* ncerr = new NcError(NcError::silent_nonfatal);
  _ncFile = new NcFile(filename.c_str(), NcFile::ReadOnly);
  if (! _ncFile->is_valid())
    throw SweepError("Error opening NOAA LF netCDF file " + filename);

  _timeDim = _ncFile->get_dim("time");
  if (! _timeDim->is_valid())
    throw SweepError("No 'time' dimension in " + filename);

  _nRadials = _timeDim->size();
  //
  // Get the times from the file
  //
  double* dtime = new double[_nRadials];
  if (! getPerRadialVar("time")->get(dtime, _nRadials))
    throw SweepError("'time' is not of type double in " + filename);

  printf("%d\n", dtime[_nRadials - 1]);
}

