

static int HailTransf( prod, messin, lenin)
    ProdStruct	*prod;
    char  	*messin;
    int   	lenin;
    {
      	PDBproduct 	header;
      	NSSL_hail_alg	*hail;

      	int		i, t1, maxpts, totpts, ret, lenout;
      	int		got_some = FALSE;
      	char		*bptr, *ptr;
      	PDBpt		*pts, *tpts;
      	char		text[ 100];
	int startx, starty;

       	/* raw_structure */
       	hail = (NSSL_hail_alg *) messin;
       	ptr = messin + sizeof( NSSL_hail_alg);

      	header.type = prod->pdbs_id;
	header.text_size = 0;
      	header.line_type = PDB_LINETYPE_THICK_SOLID;
      	STRncopy(header.color, prod->color, PDB_MAX_COLOR_NAME);
      	header.id = hail->volume_id;
      	header.subid = hail->cell_id;

      	header.generate_time = hail->dtime;
      	header.start_time = hail->dtime;
	header.expire_time = header.start_time + 600;

    	bptr = BuffOut + sizeof(PDBproduct);
	lenout = sizeof(PDBproduct);
	totpts = 0;
	maxpts = (MAX_BUFFER_PTS - lenout) / sizeof( PDBpt); /* max points */

        /* pointers into the output buffer */
        pts = (PDBpt *) bptr;
        tpts = &pts[ HAIL_PTS];

	startx = hail->x / 10;
	starty = hail->y / 10;
	DPRINT2("hail %d %d loc %f %f km prob = %d\n", hail->volume_id, hail->cell_id, 
		(double) hail->x / 1000.0, (double) hail->y / 1000.0, hail->prob);

	for (i=0; i<HAIL_PTS; i++)
	    {
	    pts[i].x = startx + (int) Hail_graphic[i].dx;
	    pts[i].y = starty + (int) Hail_graphic[i].dy;
	    }

	totpts += HAIL_PTS;
	maxpts -= HAIL_PTS;
	lenout += HAIL_PTS * sizeof( PDBpt);

        /* text */
        sprintf( text, "%d %d", hail->cell_id, hail->prob);
        t1 = FontAddText( text, tpts, maxpts, startx - 90, starty - 10, 50);
	totpts += t1;
	maxpts -= t1;
	lenout += t1 * sizeof( PDBpt);
          		
        /* complete header and copy it to output buffer */
        header.npts = totpts;
        STRcopy( BuffOut, &header, sizeof(PDBproduct));

        /* send it off */
        ProductSendout( prod->pdbs_id, BuffOut, lenout);
        got_some = TRUE;

      	return (got_some);
    }


