/*------------------------------------------------------------------------

     B U F R   E N C O D I N G   A N D   D E C O D I N G   S O F T W A R E 

FILE:          READDESC.C

AUTHOR:        Konrad Koeck
               Institute of Communication and Wave Propagation, 
               Technical University Graz, Austria

DATE CREATED:  23-FEB-1994

STATUS:        DEVELOPMENT FINISHED

This software may only be used, copied and distributed within members
of the Central European Weather Radar Network (CREAD) or within the 
Liaison Group on Operational European Weather Radar Networking (GORN). 
Any other use only with the permission of the author.


FUNCTIONAL DESCRIPTION:
-----------------------
Function(s) for reading the descriptor-file.

AMENDMENT RECORD:

ISSUE       DATE            SCNREF      CHANGE DETAILS
-----       ----            ------      --------------
V1.0        23-FEB-1994     Koeck       Initial Issue
--------------------------------------------------------------------------- */

#define READDESC_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "desc.h"
#include "bufr.h"

/*===========================================================================*/
/* internal functions                                                        */
/*===========================================================================*/

static int read_element_desc (P1(char *line));
static int read_sequence_line (P1(char *line));
static int read_table_line (P1(char *line));
static char * locate_string (P2(char *st1, char *st2));

/*===========================================================================*/
int read_desc (descfile)

char *descfile;

/* This function read the descriptor-file DESCFILE. The return-value ist 1 if
   the file has been successfully read, 0 if not 
*/

{
  FILE *fp;
  int l, status;
  char buf[200];

/******* open file */

  fp = fopen (descfile, "r");
  if (fp == NULL) return 0;

  ndes = ntab = 0;

/****** main loop */

  status = 0;                               /* Status-Variable */
  while (fgets (buf, 200, fp) != NULL) {
    l = strlen (buf);
    if (buf[l-1] == '\n') buf[l-1] = 0;     /* delete terminating \n */
    trim (buf);                             /* delete terminating blanks */
    /*printf ("%s\n", buf);*/
    if (buf[0] != '#' && strlen (buf) != 0) {  /* skip commets */

/******* ".descriptors" found */

      if (strcmp (buf, ".descriptors") == 0) status = 1;

/******* ".sequences" found */

      else if (strcmp (buf, ".sequences") == 0) status = 2;

/******* ".tables" found */

      else if (strcmp (buf, ".tables") == 0) status = 3;

/******* Line read is an element descriptor */

      else if (status == 1) {if (!read_element_desc (buf)) return 0;}

/******* Line is the first line of a sequence */

      else if (status == 2) {if (!read_sequence_line (buf)) return 0;}

/******* Line is the first line of a table */

      else if (status == 3) {if (!read_table_line (buf)) return 0;}
    }
  }

/******* Return 0 if an error occured during read, else return 1 */

  if (ferror (fp)) return 0;
  else {
    printf ("'%s' with %d data descriptos, %d tables read.\n", 
            descfile, ndes, ntab);
    return 1;
  }
}

/*===========================================================================*/
static int read_element_desc (line)

char *line;

/* This function reads one element descriptor form an input line.
   The return-value is 1 on success, 0 on a fault.
*/

{
  desc *d;
  char buf[80], *eln;
  int j;

  if (ndes == MAXDESC) {
    printf ("Number of descriptors exceeded, loading aborted !\n");
    return 0;
  }

/******* Allocate memory */

  d = des[ndes] = (desc *) malloc (sizeof (desc));
  if (des[ndes] == NULL) {
    printf ("Can't allocate memory, loading aborted !\n");
    return 0;
  }

/******* Scanf input-line */

  d->id = ELDESC;
  j = sscanf (line, "%d %d %d %s %d %lf %d %s",
              &d->elseq.el.d.f, &d->elseq.el.d.x, &d->elseq.el.d.y,    /* element descriptor */
              d->elseq.el.unit,                            /* unit */
              &d->elseq.el.scale,                          /* scale */
              &d->elseq.el.refval,                         /* reference value */
              &d->elseq.el.dw,                             /* data width */
              buf);                                  /* first word of element name */
  if (j != 8) {
    printf ("Error reading from\n'%s'\nloading aborted !\n", line);
    return 0;
  }

/******* get element name */

  eln = locate_string (line, buf);
  if (eln == NULL) {
    printf ("Error reading element-name from\n'%s'\nloading aborted !\n", line);
    return 0;
  }
  strcpy (d->elseq.el.elname, eln);
  ndes ++;
  return 1;
}

/*===========================================================================*/
static char * locate_string (st1, st2)

char *st1;
char *st2;

/* This function searches for ST2 int ST1 and returns a Pointer to the 
   position in ST1 where ST2 was found. NULL is returned, if St2 is not
   included in ST1
*/

{
  char substr[200];
  int pos, endpos;

  assert (strlen (st2) < strlen (st1));
  endpos = strlen (st1) - strlen (st2);

  for (pos = 0; pos <= endpos; pos ++) {
    strncpy (substr, st1 + pos, strlen (st2));
    substr[strlen(st2)] = 0;
    if (strcmp (substr, st2) == 0) return st1 + pos;
  }
  return NULL;
}

/*===========================================================================*/
static int read_table_line (line)

char *line;

/* This function reads a table-code from an input line
   The return-value is 1 on success, 0 on a fault.
*/

{
  dd d1;
  int i, code, ind;
  char buf[100], *mean;
  ctab *t;

/******* Scanf input line */

  i = sscanf (line, "%d %d %d %d", &d1.f, &d1.x, &d1.y, &code);

  if (i == 4) {      /* First line in a table */
    i = sscanf (line, "%d %d %d %d %s", &d1.f, &d1.x, &d1.y, &code, buf);
    if (i != 5) {
      printf ("Error reading table enty\n'%s'\nloding aborted !\n", line);
      return 0;
    }
    ind = ntab;
    ntab ++;
    t = tab[ind] = (ctab *) malloc (sizeof (ctab));
    if (tab[ind] == NULL) {
      printf ("Can't allocate memory, loading aborted !\n");
      return 0;
    }
    memcpy (&t->d, &d1, sizeof (dd));
    t->ne = 0;
    t->ce[t->ne].code = code;
    mean = locate_string (line, buf);
    assert (mean != NULL);
    assert (strlen (mean) < MTCMENL);
    strcpy (t->ce[t->ne].mean, mean);
    t->ne ++;
  }

  else if (i == 1) {   /* Subsequent line in a table */
    i = sscanf (line, "%d %s", &code, buf);
    if (i != 2) {
      printf ("Error reading table enty\n'%s'\nloding aborted !\n", line);
      return 0;
    }
    ind = ntab - 1;
    t = tab[ind];
    if (t->ne == MAXCTEL) {
      printf ("Number of code table entries exceeded !\n");
      return 0;
    }
    t->ce[t->ne].code = code;
    mean = locate_string (line, buf);
    assert (mean != NULL);
    assert (strlen (mean) < MTCMENL);
    strcpy (t->ce[t->ne].mean, mean);
    t->ne ++;
  }

  else {
    printf ("error reading table-enty\n'%s'\nloading aborted !\n", line);
    return 0;
  }
  return 1;
}

/*===========================================================================*/
static int read_sequence_line (line)

char *line;

/* This function reads a sequence-code from an input line
   The return-value is 1 on success, 0 on a fault.
*/

{
  int i, ind;
  dd d1, d2;
  desc *d;

/******* Scanf input-line */

  i = sscanf (line, "%d %d %d %d %d %d", &d1.f, &d1.x, &d1.y,   /* First descriptor */
                                         &d2.f, &d2.x, &d2.y);  /* Second descriptor (optional) */
  if (i == 6) {       /* first line of a sequence */
    if (ndes == MAXDESC) {
      printf ("Number of descriptors exceeded, loading aborted !\n");
      return 0;
    }
    ind = ndes;
    ndes ++;
    d = des[ind] = (desc *) malloc (sizeof (desc));
    if (des[ind] == NULL) {
      printf ("Can't allocate memory, loading aborted !\n");
      return 0;
    }
    d->id = SEQDESC;
    d->elseq.seq.nel = 0;
    memcpy (&d->elseq.seq.d, &d1, sizeof (dd));
    memcpy (&d->elseq.seq.del[d->elseq.seq.nel], &d2, sizeof (dd));
    d->elseq.seq.nel ++;
  }

  else if (i == 3) {  /* subsequent line of a sequence */
    ind = ndes -1;
    d = des[ind];
    if (d->elseq.seq.nel == MAXDEL) {
      printf ("Too many sequence-descriptors found in\n'%s'\nloading aborted !\n", line);
      return 0;
    }
    memcpy (&d->elseq.seq.del[d->elseq.seq.nel], &d1, sizeof (dd));
    d->elseq.seq.nel ++;
  }

  else {
    printf ("Error readind sequence entry\n'%s'\nloading aborted !\n", line);
    return 0;
  }
  return 1;
}

/*===========================================================================*/
void trim (buf)

char *buf;

/* This functions deletes all terminating blanks in a string.
*/

{
  int i, len;

  len = strlen (buf);
  for (i = len - 1; i >= 0; i --) {
    if (*(buf + i) == ' ') *(buf + i) = 0;
    else break;
  }
}

/* end of file */
