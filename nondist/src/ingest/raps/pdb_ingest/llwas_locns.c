/*
 * Read in the locations of the llwas data, convert to x,y
 */

int Setup_locations()
   {
      int count = 0;
      FILE *fp_prod;
      char fname[ 30], token[MAX_TOKEN], *ptr;
      char line[ MAX_LINE];
      PDBpt	*pt;
      double	lat, lon, x, y;

      /* initialize projection: true north */
      PJGflat_init( Mhr_loc_lat, Mhr_loc_long, 0.0);
    
      STRncopy( fname, "llwas.locs", 30);
      fp_prod = fopen(fname,"r");
      if (fp_prod == NULL)
         {
	 ERRprintf(ERR_WARNING, "ERROR opening file %s\n", fname);
	 return 0;
         }
    
      DPRINT2("\nLlwas locs:\n");
      while (NULL != fgets( line, MAX_LINE, fp_prod))
	 {
	 /* remove comments */
	 if (line[0] == '#')
	    continue;
     
         /* remove blank lines */
	 if (STRblnk( line) == 0)
	    continue;

	 /* check for too many locs */
         if (count >= MAX_LLWAS)
	    {
	    ERRprintf(ERR_WARNING, "too many lwas locs max=%d \n",
	       MAX_LLWAS);
            break;
	    }

	 pt = &Llwas_loc[ count];

	 /* parse the line : long lat */
	 ptr = line;
	 STRtokn( &ptr, token, MAX_TOKEN, " ");
         lon = atof( token);
	 STRtokn( &ptr, token, MAX_TOKEN, " ");
         lat = atof( token);

	 PJGflat_latlon2xy( lat, lon, &x, &y);
         pt->x = (i16) (100.0 * x);
         pt->y = (i16) (100.0 * y);

	 DPRINT2(" lat, long = %f %f x, y = %d %d \n", lat, lon, pt->x, pt->y);
  	 count++;
         }
    
      fclose(fp_prod);
      return( count);
   }

