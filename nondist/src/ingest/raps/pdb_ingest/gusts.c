    
static int GfTransf( prod, messin, lenin)
    ProdStruct 	*prod;
    char  	*messin;
    int   	lenin;
    {
#define CHAR_SIZE 100
      	static int	GroupId = 0;
      	static int	LastId = -1;
      	static int	Last_deltav = -1;
	static int	max_lenout = 0;

      	PDBproduct 	header;
      	Gf_header	*gfh;
      	int		i, n, ret, lenout, totpts;
      	int		got_some = FALSE;
      	char		*bptr, *ptr;
      	char		text[CHAR_SIZE];

      	/* raw_structure */
      	gfh = (Gf_header *) messin;
      	ptr = messin + sizeof(Gf_header);
    
      	/* construct the fixed part of the header */
      	header.type = prod->pdbs_id;
      	STRncopy(header.color, prod->color, PDB_MAX_COLOR_NAME);
      	header.line_type = PDB_LINETYPE_THICK_SOLID;

      	header.start_time = Ctime6( gfh->start_time);
      	header.expire_time = header.start_time + 900;
      	header.generate_time = Ctime6( gfh->start_time);
      	header.text_size = 0;
      	header.id = GroupId++;
      	header.subid = 0;
      	GroupId %= 1000;

      	if (gfh->gf_num <= 0)
	    {
	    if (!Active[ prod->pdbs_id])
	    	return (FALSE);

 	    /* send a "NULL" product to indicate timeout */
            header.npts = 0;
            header.id = 0;
            header.subid = 0;
            ProductSendout( prod->pdbs_id, &header, sizeof(PDBproduct));
	    Active[ prod->pdbs_id] = FALSE;
            return (TRUE);
	    }
	Active[ prod->pdbs_id] = TRUE;

	bptr = BuffOut + sizeof( PDBproduct);
	lenout = sizeof( PDBproduct);
	totpts = 0;

        /* loop over all gustfronts in this message */
        for (n=0; n< gfh->gf_num; n++)
	    {
            Gf_body	*gfb;
            i16		npts, *xa, *ya;
	    PDBpt	*pts, *tpts;
	    int		count, penups, width, tnpts, n1, n2;


	    /* assign pointers, figure out how many points we got:
	       each gust front body has the follwing layout:
		gf_body
		npts (i16)
		x array (npts * i16)
		npts (i16)
		yarray (npts * i16)
	    */
	    gfb = (Gf_body *) ptr;
            ptr += sizeof( Gf_body);
    	    STRcopy( &npts, ptr, sizeof(i16));
	    ptr += sizeof( i16);
	    xa = (i16 *) ptr;
	    ptr += npts * sizeof( i16);
	    ptr += sizeof( i16);
	    ya = (i16 *) ptr;
	    ptr += npts * sizeof( i16);

            /* we want to eliminate duplicates : galsv sends out repeats every 10
               seconds 
            if ((gfb->event_handle < LastId) ||
             ((gfb->event_handle == LastId) && (gfb->delta_v == Last_deltav)))
	    	continue;
	    LastId = gfb->event_handle;
	    Last_deltav = gfb->delta_v;
	    */

	    /* count how much stroked text we need */
            sprintf( text, "AA");
	    count = FONTextent( Fc, text, &penups, &width);
	    tnpts = count + penups;

	    /* pointers into the output buffer */
	    pts = (PDBpt *) bptr; 
	    tpts = &pts[npts];

            /********* construct the array ***********/
	    for (i=0; i<npts; i++)
	    	{
	    	pts[i].x = xa[i];
	    	pts[i].y = ya[i];
	    	}
	      
            /* text array: put something at beg and at end, "along" the direction of the line*/
            sprintf( text, "A");
 	    n1 = FontAddCharAlong( text, tpts, tnpts, xa[1], xa[0], ya[1], ya[0], 
		CHAR_SIZE);
  	    n2 = FontAddCharAlong( text, &tpts[n1], tnpts-n1, xa[npts-2], xa[npts-1], 
		ya[npts-2], ya[npts-1], CHAR_SIZE);

	    /* add penup */
	    tpts[ n1+n2].x = 0x7FFF;
	    tpts[ n1+n2].y = 0x7FFF;

	    bptr += (npts + n1 + n2 + 1) * sizeof( PDBpt);
	    lenout += (npts + n1 + n2 + 1) * sizeof( PDBpt);
	    totpts += (npts + n1 + n2 + 1);

            got_some = TRUE;
	    } /* loop over gustfronts */


	/* complete header and copy it to output buffer */
        header.npts = totpts;
        header.text_size = 0;
	STRcopy( BuffOut, &header, sizeof(PDBproduct));

	/* send it off */
        ProductSendout( prod->pdbs_id, BuffOut, lenout);

	if (lenout > max_lenout)
	    {
	    max_lenout = lenout;
	    printf("GF max lenout = %d\n", max_lenout);
	    }
   out:
      return (got_some);
   }


