# include <stdio.h>
# include <defs.h>
# include <message.h>
# include <DataStore.h>



int MHandler FP ((Message *));
DataChunk *MakeDC FP ((PlatformId, FieldId));

/*
 * merlin file yymmdd
 */

main (argc, argv)
int argc;
char **argv;
{
	char line[80];
	DataChunk *dc;
	FILE *fp;
	PlatformId pid;
	date uidate;
	ZebTime base, zt;
	int nsamp, prev_sec;
	float seconds;
	FieldId fid;
/*
 * Check.
 */
 	if (argc != 3)
	{
		fprintf (stderr, "Usage: %s file yymmdd\n", argv[0]);
		exit (1);
	}
/*
 * Open the damn file.
 */
	if ((fp = fopen (argv[1], "r")) == NULL)
	{
		perror (argv[1]);
		exit (1);
	}
/*
 * Hook in.
 */
	usy_init ();
	msg_connect (MHandler, "Merlin");
	ds_Initialize ();
	if ((pid = ds_LookupPlatform ("merlin")) == BadPlatform)
	{
		fprintf (stderr, "Unknown platform: merlin\n");
		exit (1);
	}
	fid = F_Lookup ("bogus");
/*
 * figure out times.
 */
	uidate.ds_yymmdd = atoi (argv[2]);
	uidate.ds_hhmmss = 0;
	TC_UIToZt (&uidate, &base);
/*
 * Initial DC.
 */
	dc = MakeDC (pid, fid);
	nsamp = 0;
/*
 * Now we plow.
 */
	prev_sec = 0;

	while (fgets (line, 80, fp) != NULL)
	{
		Location loc;
	/*
	 * Read the next line
	 */
		if (sscanf (line, "%f %f %f %f", &seconds, &loc.l_lat,
				&loc.l_lon, &loc.l_alt) != 4)
		{
			fprintf (stderr, "Weird line: %s", line);
			continue;
		}
	/*
	 * Look for a day change
	 */
		if (seconds < prev_sec)
			base.zt_Sec += 86400;

		prev_sec = seconds;
	/*
	 * Put together this sample
	 */
		zt = base;
		zt.zt_Sec += seconds;
		dc_AddScalar (dc, &zt, nsamp, fid, &loc.l_alt);
		dc_SetLoc (dc, nsamp, &loc);
	/*
	 * Perhaps store.
	 */
		if (++nsamp > 100)
		{
		    ds_Store (dc, FALSE, 0, 0);
		    dc_DestroyDC (dc);
		    dc = MakeDC (pid, fid);
		    nsamp = 0;
		}
	}
/*
 * Flush out the remainder
 */
	if (nsamp)
	{
	    ds_Store (dc, FALSE, 0, 0);
	    dc_DestroyDC (dc);
	}
}






DataChunk *
MakeDC (pid, fid)
PlatformId pid;
FieldId fid;
/*
 * Make the DC.
 */
{
	DataChunk *dc = dc_CreateDC (DCC_Scalar);
	dc->dc_Platform = pid;
	dc_SetScalarFields (dc, 1, &fid);
	return (dc);
}



int 
MHandler (msg)
Message *msg;
{
}
