# include <math.h>
# include <unistd.h>
# include <message.h>
# include <DataStore.h>

const float	Bad = -9.9e9;

int	msg_handler (struct message* msg);
double	drandom (double max);


main (int argc, char *argv[])
{
	DataChunk	*dc = dc_CreateDC (DCC_Scalar);
	FieldId		flds[16];
	int		nflds;
	float		spd, dir;
	ZebTime		t;
	PlatformId	plats[2];
	Location	locs[2];

	usy_init();

	if (! msg_connect (msg_handler, "FakeData"))
	{
		fprintf (stderr, 
			 "FakeData (%s): unable to connect to msg handler\n",
			 argv[0]);
		exit (1);
	}

	ds_Initialize ();

	plats[0] = ds_LookupPlatform ("eagle_crest");
	locs[0].l_lat = 58.40;
	locs[0].l_lon = -134.95;
	locs[0].l_alt = 0;

	plats[1] = ds_LookupPlatform ("sheep_mtn");
	locs[1].l_lat = 58.25;
	locs[1].l_lon = -134.41;
	locs[1].l_alt = 1000;

	nflds = 0;
	/* flds[nflds++] = F_DeclareField ("wspd", "wind speed", "m/s"); */
	flds[nflds++] = F_DeclareField ("wspd_k", "wind speed", "knots");
	flds[nflds++] = F_DeclareField ("wdir", "wind direction", "deg");
	flds[nflds++] = F_DeclareField ("wdir_mag", "magnetic wind direction", 
					"deg mag");

	dc_SetScalarFields (dc, nflds, flds);
	dc_SetBadval (dc, Bad);

	while (1)
	{
		long	i, itime = time(0);
		int	sleeptime = 16 - (itime % 15);

		itime = (itime / 15) * 15;

		for (i = 0; i < 2; i++)
		{
			int	period = (i + 1) * 15;
			float	wspd, wdir, wdir_mag;

			if (itime % period)
				continue;

			dc->dc_Platform = plats[i];
			dc_SetStaticLoc (dc, locs + i);
			t.zt_Sec = itime;
			t.zt_MicroSec = 0;

			wspd = drandom (30.0);
			wdir = drandom (360.0);
			wdir_mag = wdir + 28.;
			if (wdir_mag > 360.0)
			  wdir_mag -= 360.0;

			dc_AddScalar (dc, &t, 0, flds[0], &wspd);
			dc_AddScalar (dc, &t, 0, flds[1], &wdir);
			dc_AddScalar (dc, &t, 0, flds[1], &wdir_mag);

			ds_Store (dc, 0, 0, 0);
		}

		sleep (sleeptime);
	}
}

	

	
int
msg_handler (msg)
struct message *msg;
{
	fprintf (stderr, "FakeData got a message?  Time to die!");
	exit (1);
}



double
drandom (double max)
{
	double factor = (double) random() / (double) 0x7FFFFFFF;
	return (factor * max);
}

	
