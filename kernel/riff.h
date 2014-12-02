typedef struct
{
  unsigned char	r_ckID[4];		/* Chunk ID: "RIFF" */
  int	r_cksize;				/* Chunk size: 4+n */
  unsigned char 	r_WAVEID[4];	/* WAVE ID: "WAVE" */
} RIFFhdr;

#define R_ID1	'R'
#define R_ID2	'I'
#define R_ID3	'F'
#define R_ID4	'F'

#define W_ID1	'W'
#define W_ID2	'A'
#define W_ID3	'V'
#define W_ID4	'E'
