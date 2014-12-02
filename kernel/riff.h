typedef struct
{
  unsigned char	r_ckID[4];		/* Chunk ID: "RIFF" */
  uint16_t	r_cksize;				/* Chunk size: 4+n */
  unsigned char 	r_WAVEID[4];	/* WAVE ID: "WAVE" */
} RIFFhdr;
