
/* Dixon Storm Tracks */
#define TS_ARROW_HEAD 60
#define VOLUME_THRESHOLD 50.0

#ifdef notdef		/* UNDER CONSTRUCTION */
static int  
TsTransf(prod, index)
   ProdStruct *prod;
   tdata_basic_index_t *index;

{
      PDBproduct  header;

      long        icomplex,
                  isimple,
                  ientry;
      tdata_basic_complex_params_t *c_params;
      tdata_basic_simple_params_t **s_params_ptr;
      tdata_basic_track_entry_t **t_entry_ptr;
      tdata_basic_track_entry_t ***t_entry_ptr_ptr;

      static int  GroupId = 0;
      int         i,
                  j,
                  n,
                  npts,
                  maxpts,
                  totpts,
                  ret,
                  lenout,
                  nextraps;
      int         got_some = FALSE;
      int         is_predict = FALSE,
                  is_forecast = FALSE;
      char       *bptr,
                 *ptr;
      time_t      start_time;

      PDBpt      *pts,
                 *tpts;
      int         t1,
                  t2,
                  avgx,
                  maxy;
      char        text[100];

      if (index->header.n_complex_tracks <= 0)
      {
	 if (!Active[prod->pdbs_id])
	    return (FALSE);

	 /* send a "NULL" product to indicate timeout */
	 header.npts = 0;
	 header.id = 0;
	 header.subid = 0;
	 ProductSendout(prod->pdbs_id, &header, sizeof(PDBproduct));
	 Active[prod->pdbs_id] = FALSE;
	 return (TRUE);
      }
      Active[prod->pdbs_id] = TRUE;
      is_predict = (prod->product_id == 10099);
      is_forecast = (prod->product_id == 10097);

      /* construct the fixed part of the header */
      header.type = prod->pdbs_id;
      header.text_size = 0;
      header.line_type = PDB_LINETYPE_THICK_SOLID;
      STRncopy(header.color, prod->color, PDB_MAX_COLOR_NAME);
      header.id = GroupId++;
      header.subid = 0;

      header.generate_time = index->header.data_start_time;
      header.start_time = index->header.data_start_time;
      header.expire_time = header.start_time + 600;

      bptr = BuffOut + sizeof(PDBproduct);
      lenout = sizeof(PDBproduct);
      totpts = 0;
      maxpts = (MAX_BUFFER_PTS - lenout) / sizeof(PDBpt);	/* max points */

      /* pointers into the output buffer */
      pts = (PDBpt *) bptr;

      c_params = index->complex_params;
      s_params_ptr = index->simple_params;
      t_entry_ptr_ptr = index->track_entry;

      for (icomplex = 0; icomplex < index->header.n_complex_tracks; icomplex++)
      {
	 tdata_basic_simple_params_t *s_params = *s_params_ptr;

	 t_entry_ptr = *t_entry_ptr_ptr;

	 for (isimple = 0; isimple < c_params->n_simple_tracks; isimple++)
	 {
	    tdata_basic_track_entry_t *t_entry = *t_entry_ptr;

	    for (ientry = 0; ientry < s_params->duration_in_scans; ientry++)
	    {
	       double      x,
	                   y,
	                   a,
	                   b,
	                   o,
	                   factor,
	                   dx_dt,
	                   dy_dt;

	       if (t_entry->time != index->header.time)
		  goto next;

	       got_some = TRUE;

	       factor = (double) index->header.factor;
	       x = (double) t_entry->projected_area_centroid_x / factor;
	       y = (double) t_entry->projected_area_centroid_y / factor;
	       a = (double) t_entry->projected_area_major_radius / factor;
	       b = (double) t_entry->projected_area_minor_radius / factor;
	       o = 90.0 - (double) t_entry->projected_area_orientation / factor;
	       dx_dt = (double) t_entry->projected_area_centroid_dx_dt / factor;
	       dy_dt = (double) t_entry->projected_area_centroid_dy_dt / factor;

	       if ((is_predict) || (is_forecast))
	       {
		  double      volume,
		              dvol_dt,
		              area,
		              darea_dt;
		  double      f_time,
		              f_volume,
		              f_area,
		              f_x,
		              f_y,
		              f_scale,
		              f_a,
		              f_b;
		  double      last_x,
		              last_y;
		  int         i,
		              nsteps,
		              interval,
		              min_until;

		  volume = (double) t_entry->volume / factor;
		  dvol_dt = (double) t_entry->dvolume_dt / factor;
		  area = (double) t_entry->projected_area / factor;
		  darea_dt = (double) t_entry->dprojected_area_dt / factor;

		  last_x = x;
		  last_y = y;

		  if (is_predict)
		  {
		     nsteps = 5;
		     interval = 600;
		  }
		  else
		  {
		     time_t      now,
		                 then;
		     UTIMstruct  utim;

		     /* find the desired forecast time */
		     time(&now);
		     UTIMunix_to_date(now, &utim);
		     if ((utim.min > 15) && (utim.min <= 45))
		     {
			utim.hour++;
			utim.min = 15;
		     }
		     else
		     {
			if (utim.min > 45)
			   utim.hour++;
			utim.min = 45;
		     }
		     then = UTIMdate_to_unix(&utim);
		     interval = then - index->header.time;
		     min_until = (then - now) / 60;

		     nsteps = 1;
		  }

		  for (i = 0; i < nsteps; i++)
		  {
		     f_time = ((double) (i + 1) * interval / 3600.0);
		     f_volume = volume + dvol_dt * f_time;
		     f_area = area + darea_dt * f_time;

		     if (f_area < 1.0)
			f_area = 1.0;

		     f_x = x + dx_dt * f_time;
		     f_y = y + dy_dt * f_time;
		     f_scale = sqrt(f_area / area);
		     f_a = a * f_scale;
		     f_b = b * f_scale;

		     /* break out if volume or projected_area is too low */
		     if (f_volume < VOLUME_THRESHOLD || f_area < 1.1)
			break;

		     n = AddArrow(pts, (int) (100.0 * f_x), (int) (100.0 * f_y),
				  (int) (100.0 * last_x), (int) (100.0 * last_y), TS_ARROW_HEAD);
		     totpts += n;
		     maxpts -= n;
		     pts += n;

		     n = Gen_ellipse(pts, maxpts, f_x, f_y, f_a, f_b, o, 36);
		     totpts += n;
		     maxpts -= n;
		     pts += n;

		     if (is_forecast)
		     {
			sprintf(text, "%d", min_until);
			n = FontAddText(text, pts, maxpts, (int) (100.0 * f_x),
					(int) (100.0 * f_y), 50);
			totpts += n;
			maxpts -= n;
			pts += n;

			printf(" Forec x,y = %f %f radius = %f %f min_until %d\n",
			       f_x, f_y, f_a, f_b, min_until);

		     }

		     last_x = f_x;
		     last_y = f_y;
		  }		/* loop over FORECAST_STEPS */
	       }
	       else		/* is_predict, is_forecast */

		  /*
		   * double speed;
		   * 
		   * printf(" x,y = %f %f radius = %f %f orient = %f\n", x, y,
		   * a, b, o);
		   * 
		   * n = Gen_ellipse( pts, maxpts, x, y, a, b, o, 36); totpts +=
		   * n; maxpts -= n; pts += n;
		   * 
		   * speed = sqrt( dx_dt * dx_dt + dy_dt * dy_dt); sprintf(text,
		   * "%d", (int) speed); n = FontAddText( text, pts, maxpts,
		   * (int)(100.0 *x), (int) (100.0*y), 50); totpts += n;
		   * maxpts -= n; pts += n; }
		   * 
		   * next: t_entry++; } /* ientry
		   */
		  t_entry_ptr++;
	       s_params++;
	    }			/* isimple */

	    t_entry_ptr_ptr++;
	    s_params_ptr++;
	    c_params++;
	 }			/* icomplex */


	 if (got_some)
	 {
	    /* complete header and copy it to output buffer */
	    header.npts = totpts;
	    STRcopy(BuffOut, &header, sizeof(PDBproduct));
	    lenout += totpts * sizeof(PDBpt);

	    /* send it off */
	    ProductSendout(prod->pdbs_id, BuffOut, lenout);

	    /* send a "product_info" message if got a product */
	    SendInfo(prod->product_id);
	 }

	 return (got_some);
      }
#endif /* notdef  --- UNDER CONSTRUCTION */

