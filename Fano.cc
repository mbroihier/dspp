/*
 This file is part of dspp.  It was taken from rtlsdr_wsprd

 File name: fano.cc

 Description: Soft decision Fano sequential decoder for K=32 r=1/2
 convolutional code.

 Copyright 1994, Phil Karn, KA9Q
 Minor modifications by Joe Taylor, K1JT
 C++ Mark Broihier 2023
*/

#define	LL 1	                // Select Layland-Lushbaugh code

#include "fano.h"

struct node {
    unsigned long encstate;	// Encoder state of next node
    long gamma;		        // Cumulative metric to this node
    int metrics[4];		// Metrics indexed by all possible tx syms
    int tm[2];		        // Sorted metrics for current hypotheses
    int i;			// Current branch being tested
};

// Convolutional coding polynomials. All are rate 1/2, K=32
#ifdef	NASA_STANDARD
/* "NASA standard" code by Massey & Costello
 * Nonsystematic, quick look-in, dmin=11, dfree=23
 * used on Pioneer 10-12, Helios A,B
 */
#define	POLY1	0xbbef6bb7
#define	POLY2	0xbbef6bb5
#endif

#ifdef	MJ
/* Massey-Johannesson code
 * Nonsystematic, quick look-in, dmin=13, dfree>=23
 * Purported to be more computationally efficient than Massey-Costello
 */
#define	POLY1	0xb840a20f
#define POLY2	0xb840a20d
#endif

#ifdef	LL
/* Layland-Lushbaugh code
 * Nonsystematic, non-quick look-in, dmin=?, dfree=?
 */
#define	POLY1	0xf2d05351
#define	POLY2	0xe4613c47
#endif

/* Convolutionally encode a packet. The input data bytes are read
 * high bit first and the encoded packet is written into 'symbols',
 * one symbol per byte. The first symbol is generated from POLY1,
 * the second from POLY2.
 *
 * Storing only one symbol per byte uses more space, but it is faster
 * and easier than trying to pack them more compactly.
 */
constexpr unsigned char Fano::Partab[];
constexpr float Fano::metric_tables[4][256];
int Fano::encode(
    unsigned char *symbols,	// Output buffer, 2*nbytes
    unsigned char *data,		// Input buffer, nbytes
    unsigned int nbytes) {	// Number of bytes in data
    unsigned long encstate;
    int sym;
    int i;

    encstate = 0;
    while(nbytes-- != 0) {
        for(i=7; i>=0; i--) {
            encstate = (encstate << 1) | ((*data >> i) & 1);
            ENCODE(sym,encstate);
            *symbols++ = sym >> 1;
            *symbols++ = sym & 1;
        }
        data++;
    }
    return 0;
}
uint32_t Fano::nhash( const void *key, size_t length, uint32_t initval) {
    uint32_t a,b,c;                                          /* internal state */
    union {
        const void *ptr;
        size_t i;
    } u;     /* needed for Mac Powerbook G4 */

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

    u.ptr = key;
    if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
        const uint32_t *k = (const uint32_t *)key;         /* read 32-bit chunks */

        /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
        while (length > 12) {
            a += k[0];
            b += k[1];
            c += k[2];
            mix(a,b,c);
            length -= 12;
            k += 3;
        }

        /*----------------------------- handle the last (probably partial) block */
        /*
         * "k[2]&0xffffff" actually reads beyond the end of the string, but
         * then masks off the part it's not allowed to read.  Because the
         * string is aligned, the masked-off tail is in the same word as the
         * rest of the string.  Every machine with memory protection I've seen
         * does it on word boundaries, so is OK with this.  But VALGRIND will
         * still catch it and complain.  The masking trick does make the hash
         * noticably faster for short strings (like English words).
         */
#ifndef VALGRIND

        switch(length) {
        case 12:
            c+=k[2];
            b+=k[1];
            a+=k[0];
            break;
        case 11:
            c+=k[2]&0xffffff;
            b+=k[1];
            a+=k[0];
            break;
        case 10:
            c+=k[2]&0xffff;
            b+=k[1];
            a+=k[0];
            break;
        case 9 :
            c+=k[2]&0xff;
            b+=k[1];
            a+=k[0];
            break;
        case 8 :
            b+=k[1];
            a+=k[0];
            break;
        case 7 :
            b+=k[1]&0xffffff;
            a+=k[0];
            break;
        case 6 :
            b+=k[1]&0xffff;
            a+=k[0];
            break;
        case 5 :
            b+=k[1]&0xff;
            a+=k[0];
            break;
        case 4 :
            a+=k[0];
            break;
        case 3 :
            a+=k[0]&0xffffff;
            break;
        case 2 :
            a+=k[0]&0xffff;
            break;
        case 1 :
            a+=k[0]&0xff;
            break;
        case 0 :
            return c;              /* zero length strings require no mixing */
        }

#else /* make valgrind happy */

        k8 = (const uint8_t *)k;
        switch(length) {
        case 12:
            c+=k[2];
            b+=k[1];
            a+=k[0];
            break;
        case 11:
            c+=((uint32_t)k8[10])<<16;  /* fall through */
        case 10:
            c+=((uint32_t)k8[9])<<8;    /* fall through */
        case 9 :
            c+=k8[8];                   /* fall through */
        case 8 :
            b+=k[1];
            a+=k[0];
            break;
        case 7 :
            b+=((uint32_t)k8[6])<<16;   /* fall through */
        case 6 :
            b+=((uint32_t)k8[5])<<8;    /* fall through */
        case 5 :
            b+=k8[4];                   /* fall through */
        case 4 :
            a+=k[0];
            break;
        case 3 :
            a+=((uint32_t)k8[2])<<16;   /* fall through */
        case 2 :
            a+=((uint32_t)k8[1])<<8;    /* fall through */
        case 1 :
            a+=k8[0];
            break;
        case 0 :
            return c;
        }

#endif /* !valgrind */

    } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
        const uint16_t *k = (const uint16_t *)key;         /* read 16-bit chunks */
        const uint8_t  *k8;

        /*--------------- all but last block: aligned reads and different mixing */
        while (length > 12) {
            a += k[0] + (((uint32_t)k[1])<<16);
            b += k[2] + (((uint32_t)k[3])<<16);
            c += k[4] + (((uint32_t)k[5])<<16);
            mix(a,b,c);
            length -= 12;
            k += 6;
        }

        /*----------------------------- handle the last (probably partial) block */
        k8 = (const uint8_t *)k;
        switch(length) {
        case 12:
            c+=k[4]+(((uint32_t)k[5])<<16);
            b+=k[2]+(((uint32_t)k[3])<<16);
            a+=k[0]+(((uint32_t)k[1])<<16);
            break;
        case 11:
            c+=((uint32_t)k8[10])<<16;     /* fall through */
        case 10:
            c+=k[4];
            b+=k[2]+(((uint32_t)k[3])<<16);
            a+=k[0]+(((uint32_t)k[1])<<16);
            break;
        case 9 :
            c+=k8[8];                      /* fall through */
        case 8 :
            b+=k[2]+(((uint32_t)k[3])<<16);
            a+=k[0]+(((uint32_t)k[1])<<16);
            break;
        case 7 :
            b+=((uint32_t)k8[6])<<16;      /* fall through */
        case 6 :
            b+=k[2];
            a+=k[0]+(((uint32_t)k[1])<<16);
            break;
        case 5 :
            b+=k8[4];                      /* fall through */
        case 4 :
            a+=k[0]+(((uint32_t)k[1])<<16);
            break;
        case 3 :
            a+=((uint32_t)k8[2])<<16;      /* fall through */
        case 2 :
            a+=k[0];
            break;
        case 1 :
            a+=k8[0];
            break;
        case 0 :
            return c;                     /* zero length requires no mixing */
        }

    } else {                        /* need to read the key one byte at a time */
        const uint8_t *k = (const uint8_t *)key;

        /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
        while (length > 12) {
            a += k[0];
            a += ((uint32_t)k[1])<<8;
            a += ((uint32_t)k[2])<<16;
            a += ((uint32_t)k[3])<<24;
            b += k[4];
            b += ((uint32_t)k[5])<<8;
            b += ((uint32_t)k[6])<<16;
            b += ((uint32_t)k[7])<<24;
            c += k[8];
            c += ((uint32_t)k[9])<<8;
            c += ((uint32_t)k[10])<<16;
            c += ((uint32_t)k[11])<<24;
            mix(a,b,c);
            length -= 12;
            k += 12;
        }

        /*-------------------------------- last block: affect all 32 bits of (c) */
        switch(length) {                 /* all the case statements fall through */
        case 12:
            c+=((uint32_t)k[11])<<24;
        case 11:
            c+=((uint32_t)k[10])<<16;
        case 10:
            c+=((uint32_t)k[9])<<8;
        case 9 :
            c+=k[8];
        case 8 :
            b+=((uint32_t)k[7])<<24;
        case 7 :
            b+=((uint32_t)k[6])<<16;
        case 6 :
            b+=((uint32_t)k[5])<<8;
        case 5 :
            b+=k[4];
        case 4 :
            a+=((uint32_t)k[3])<<24;
        case 3 :
            a+=((uint32_t)k[2])<<16;
        case 2 :
            a+=((uint32_t)k[1])<<8;
        case 1 :
            a+=k[0];
            break;
        case 0 :
            return c;
        }
    }

    final(a,b,c);
    c=(32767&c);

    return c;
}

void Fano::unpack50( signed char *dat, int32_t *n1, int32_t *n2 ) {
    int32_t i,i4;

    i=dat[0];
    i4=i&255;
    *n1=i4<<20;

    i=dat[1];
    i4=i&255;
    *n1=*n1+(i4<<12);

    i=dat[2];
    i4=i&255;
    *n1=*n1+(i4<<4);

    i=dat[3];
    i4=i&255;
    *n1=*n1+((i4>>4)&15);
    *n2=(i4&15)<<18;

    i=dat[4];
    i4=i&255;
    *n2=*n2+(i4<<10);

    i=dat[5];
    i4=i&255;
    *n2=*n2+(i4<<2);

    i=dat[6];
    i4=i&255;
    *n2=*n2+((i4>>6)&3);
    fprintf(stderr, "n1: %8.8x, n2: %8.8x\n", *n1, *n2);
}

int Fano::unpackcall( int32_t ncall, char *call ) {
    char c[]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E',
               'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T',
               'U','V','W','X','Y','Z',' '
              };
    int32_t n;
    int i;
    char tmp[7];

    n=ncall;
    strcpy(call,"......");
    if (n < 262177560 ) {
        i=n%27+10;
        tmp[5]=c[i];
        n=n/27;
        i=n%27+10;
        tmp[4]=c[i];
        n=n/27;
        i=n%27+10;
        tmp[3]=c[i];
        n=n/27;
        i=n%10;
        tmp[2]=c[i];
        n=n/10;
        i=n%36;
        tmp[1]=c[i];
        n=n/36;
        i=n;
        tmp[0]=c[i];
        tmp[6]='\0';
        // remove leading whitespace
        for(i=0; i<5; i++) {
            if( tmp[i] != c[36] )
                break;
        }
        sprintf(call,"%-6s",&tmp[i]);
        // remove trailing whitespace
        for(i=0; i<6; i++) {
            if( call[i] == c[36] ) {
                call[i]='\0';
            }
        }
    } else {
      fprintf(stderr, "unpackcall n:%8.8x\n", n);
      return 0;
    }
    return 1;
}

int Fano::unpackgrid( int32_t ngrid, char *grid) {
    char c[]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E',
               'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T',
               'U','V','W','X','Y','Z',' '
              };
    int dlat, dlong;

    ngrid=ngrid>>7;
    if( ngrid < 32400 ) {
        dlat=(ngrid%180)-90;
        dlong=(ngrid/180)*2 - 180 + 2;
        if( dlong < -180 )
            dlong=dlong+360;
        if( dlong > 180 )
            dlong=dlong+360;
        int nlong = 60.0*(180.0-dlong)/5.0;
        int n1 = nlong/240;
        int n2 = (nlong - 240*n1)/24;
        grid[0] = c[10+n1];
        grid[2]=  c[n2];

        int nlat = 60.0*(dlat+90)/2.5;
        n1 = nlat/240;
        n2 = (nlat-240*n1)/24;
        grid[1]=c[10+n1];
        grid[3]=c[n2];
    } else {
        strcpy(grid,"XXXX");
        return 0;
    }
    return 1;
}

int Fano::unpackpfx( int32_t nprefix, char *call) {
    char nc, pfx[4]="", tmpcall[7]="";
    int i;
    int32_t n;
    const size_t sizeOfCall = 13;

    //strcpy(tmpcall,call);
    snprintf(tmpcall, sizeof(tmpcall), "%s", call);
    if( nprefix < 60000 ) {
        // add a prefix of 1 to 3 characters
        n=nprefix;
        for (i=2; i>=0; i--) {
            nc=n%37;
            if( (nc >= 0) & (nc <= 9) ) {
                pfx[i]=nc+48;
            } else if( (nc >= 10) & (nc <= 35) ) {
                pfx[i]=nc+55;
            } else {
                pfx[i]=' ';
            }
            n=n/37;
        }

        //strcpy(call,pfx);
        //strncat(call,"/",sizeof("/"));
        //strncat(call,tmpcall,sizeof(tmpcall));
        snprintf(call, sizeOfCall, "%s/%s", pfx, tmpcall);

    } else {
        // add a suffix of 1 or 2 characters
        nc=nprefix-60000;
        if( (nc >= 0) & (nc <= 9) ) {
            pfx[0]=nc+48;
            //strcpy(call,tmpcall);
            //strncat(call,"/",sizeof("/"));
            //strncat(call,pfx,sizeof(pfx));
            snprintf(call, sizeOfCall, "%s/%s", tmpcall, pfx);
        } else if( (nc >= 10) & (nc <= 35) ) {
            pfx[0]=nc+55;
            //strcpy(call,tmpcall);
            //strncat(call,"/",sizeof("/"));
            //strncat(call,pfx,sizeof(pfx));
            snprintf(call, sizeOfCall, "%s/%s", tmpcall, pfx);
        } else if( (nc >= 36) & (nc <= 125) ) {
            pfx[0]=(nc-26)/10+48;
            pfx[1]=(nc-26)%10+48;
            //strcpy(call,tmpcall);
            //strncat(call,"/",sizeof("/"));
            //strncat(call,pfx,sizeof(pfx));
            snprintf(call, sizeOfCall, "%s/%s", tmpcall, pfx);
        } else {
            return 0;
        }
    }
    return 1;
}

void Fano::deinterleave(unsigned char *sym) {
    unsigned char tmp[162];
    unsigned char p, i, j;

    p=0;
    i=0;
    while (p<162) {
        j=((i * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
        if (j < 162 ) {
            tmp[p]=sym[j];
            p=p+1;
        }
        i=i+1;
    }
    for (i=0; i<162; i++) {
        sym[i]=tmp[i];
    }
}
int Fano::unpk(signed char *message, char *hashtab, char *call_loc_pow, char *call, char *loc, char *pwr, char *callsign) {
    int n1,n2,n3,ndbm,ihash,nadd,noprint=0;
    char grid[5],grid6[7];
    const int sizeOfCallAll = 23;
    const int sizeOfCall = 13;
    const int sizeOfPwr = 3;
    const int sizeOfLoc = 7;

    unpack50(message,&n1,&n2);
    if( !unpackcall(n1,callsign) ) return 1;
    if( !unpackgrid(n2, grid) ) return 1;
    int ntype = (n2&127) - 64;
    callsign[12]=0;
    grid[4]=0;

    /*
     Based on the value of ntype, decide whether this is a Type 1, 2, or
     3 message.

     * Type 1: 6 digit call, grid, power - ntype is positive and is a member
     of the set {0,3,7,10,13,17,20...60}

     * Type 2: extended callsign, power - ntype is positive but not
     a member of the set of allowed powers

     * Type 3: hash, 6 digit grid, power - ntype is negative.
     */

    if( (ntype >= 0) && (ntype <= 62) ) {
        int nu=ntype%10;
        if( nu == 0 || nu == 3 || nu == 7 ) {
            ndbm=ntype;
            memset(call_loc_pow,0,sizeof(char)*23);
            snprintf(call_loc_pow, sizeOfCallAll, "%s %s %2d", callsign, grid, ndbm);
            ihash=nhash(callsign,strlen(callsign),(uint32_t)146);
            strcpy(hashtab+ihash*13,callsign);

            memset(call,0,strlen(callsign)+1);
            memset(loc,0,strlen(grid)+1);
            memset(pwr,0,2+1);
            snprintf(call, sizeOfCall, "%s", callsign);
            snprintf(loc, sizeOfLoc, "%s", grid);
            snprintf(pwr, sizeOfPwr, "%2d", ndbm);

        } else {
            nadd=nu;
            if( nu > 3 ) nadd=nu-3;
            if( nu > 7 ) nadd=nu-7;
            n3=n2/128+32768*(nadd-1);
            if( !unpackpfx(n3,callsign) ) return 1;
            ndbm=ntype-nadd;
            memset(call_loc_pow,0,sizeof(char)*23);
            snprintf(call_loc_pow, sizeOfCallAll, "%s %2d", callsign, ndbm);
            int nu=ndbm%10;
            if( nu == 0 || nu == 3 || nu == 7 || nu == 10 ) { //make sure power is OK
                ihash=nhash(callsign,strlen(callsign),(uint32_t)146);
                strcpy(hashtab+ihash*13,callsign);
            } else noprint=1;

            memset(call,0,strlen(callsign)+1);
            memset(loc,0,1);
            memset(pwr,0,2+1);
            snprintf(call, sizeOfCall, "%s %2d", callsign, ndbm);
            snprintf(loc, sizeOfLoc, "");
            snprintf(pwr, sizeOfPwr, "%2d", ndbm);
        }
    } else if ( ntype < 0 ) {
        ndbm=-(ntype+1);
        memset(grid6,0,sizeof(char)*7);
        strncat(grid6,callsign+5,1);
        strncat(grid6,callsign,5);
        int nu=ndbm%10;
        if( (nu == 0 || nu == 3 || nu == 7 || nu == 10) &&        \
                (isalpha(grid6[0]) && isalpha(grid6[1]) &&    \
                 isdigit(grid6[2]) && isdigit(grid6[3]) ) ) {
            // not testing 4'th and 5'th chars because of this case: <PA0SKT/2> JO33 40
            // grid is only 4 chars even though this is a hashed callsign...
            //         isalpha(grid6[4]) && isalpha(grid6[5]) ) ) {
            ihash=nhash(callsign,strlen(callsign),(uint32_t)146);
            strcpy(hashtab+ihash*13,callsign);
        } else noprint=1;

        ihash=(n2-ntype-64)/128;
        if( strncmp(hashtab+ihash*13,"\0",1) != 0 ) {
            sprintf(callsign,"<%s>",hashtab+ihash*13);
        } else {
            sprintf(callsign,"%5s","<...>");
        }

        memset(call_loc_pow,0,sizeof(char)*23);
        snprintf(call_loc_pow, sizeOfCallAll, "%s %s %2d", callsign, grid6, ndbm);

        memset(call,0,strlen(callsign)+1);
        memset(loc,0,strlen(grid6)+1);
        memset(pwr,0,2+1);
        snprintf(call, sizeOfCall, "%s", callsign);
        snprintf(loc, sizeOfLoc, "%s", grid6);
        snprintf(pwr, sizeOfPwr, "%2d", ndbm);

        // I don't know what to do with these... They show up as "A000AA" grids.
        if( ntype == -64 ) noprint=1;
    }
    return noprint;
}

/* Decode packet with the Fano algorithm.
 * Return 0 on success, -1 on timeout
 */
int Fano::fano( unsigned int  *metric,	// Final path metric (returned value)
          unsigned int  *cycles,	// Cycle count (returned value)
          unsigned int  *maxnp,     // Progress before timeout (returned value)
          unsigned char *data,	    // Decoded output data
          unsigned char *symbols,   // Raw deinterleaved input symbols
          unsigned int nbits,	    // Number of output bits
          int delta,		        // Threshold adjust parameter
          unsigned int maxcycles) { // Decoding timeout in cycles per bit

    struct node *nodes;		        // First node
    struct node *np;	            // Current node
    struct node *lastnode;	        // Last node
    struct node *tail;		        // First node of tail
    int t;			                // Threshold
    int  m0,m1;
    int ngamma;
    unsigned int lsym;
    unsigned int i;

    if((nodes = (struct node *)malloc((nbits+1)*sizeof(struct node))) == NULL) {
        printf("malloc failed\n");
        return 0;
    }
    lastnode = &nodes[nbits-1];
    tail = &nodes[nbits-31];
    *maxnp = 0;

    /* Compute all possible branch metrics for each symbol pair
     * This is the only place we actually look at the raw input symbols
     */
    for(np=nodes; np <= lastnode; np++) {
        np->metrics[0] = mettab[0][symbols[0]] + mettab[0][symbols[1]];
        np->metrics[1] = mettab[0][symbols[0]] + mettab[1][symbols[1]];
        np->metrics[2] = mettab[1][symbols[0]] + mettab[0][symbols[1]];
        np->metrics[3] = mettab[1][symbols[0]] + mettab[1][symbols[1]];
        fprintf(stderr, "metric 0: %d, 1: %d, 2: %d, 3: %d, symbol 1: %d, symbol 2: %d\n",
                np->metrics[0], np->metrics[1], np->metrics[2], np->metrics[3],
                symbols[0], symbols[1]);
        symbols += 2;
    }
    np = nodes;
    np->encstate = 0;

    // Compute and sort branch metrics from root node */
    ENCODE(lsym,np->encstate);	// 0-branch (LSB is 0)
    m0 = np->metrics[lsym];

    /* Now do the 1-branch. To save another ENCODE call here and
     * inside the loop, we assume that both polynomials are odd,
     * providing complementary pairs of branch symbols.

     * This code should be modified if a systematic code were used.
     */

    m1 = np->metrics[3^lsym];
    if(m0 > m1) {
        np->tm[0] = m0; // 0-branch has better metric
        np->tm[1] = m1;
    } else {
        np->tm[0] = m1; // 1-branch is better
        np->tm[1] = m0;
        np->encstate++;	// Set low bit
    }
    np->i = 0;	        // Start with best branch
    maxcycles *= nbits;
    np->gamma = t = 0;

    // Start the Fano decoder
    for(i=1; i <= maxcycles; i++) {
        if((int)(np-nodes) > (int)*maxnp) *maxnp=(int)(np-nodes);

        // Look forward */
        ngamma = np->gamma + np->tm[np->i];
        if(ngamma >= t) {
            if(np->gamma < t + delta) {  // Node is acceptable
                /* First time we've visited this node;
                 * Tighten threshold.
                 *
                 * This loop could be replaced with
                 *   t += delta * ((ngamma - t)/delta);
                 * but the multiply and divide are slower.
                 */
                while(ngamma >= t + delta) t += delta;
            }
            // Move forward
            np[1].gamma = ngamma;
            np[1].encstate = np->encstate << 1;
            if( ++np == (lastnode+1) ) {
                break; // Done!
            }

            /* Compute and sort metrics, starting with the
             * zero branch
             */
            ENCODE(lsym,np->encstate);
            if(np >= tail) {
                /* The tail must be all zeroes, so don't
                 * bother computing the 1-branches here.
                 */
                np->tm[0] = np->metrics[lsym];
            } else {
                m0 = np->metrics[lsym];
                m1 = np->metrics[3^lsym];
                if(m0 > m1) {
                    np->tm[0] = m0;  // 0-branch is better
                    np->tm[1] = m1;
                } else {
                    np->tm[0] = m1;  // 1-branch is better
                    np->tm[1] = m0;
                    np->encstate++;	 // Set low bit
                }
            }
            np->i = 0;
            // Start with best branch
            continue;
        }

        // Threshold violated, can't go forward
        for(;;) {
            // Look backward
            if(np == nodes || np[-1].gamma < t) {
                /* Can't back up either.
                 * Relax threshold and and look
                 * forward again to better branch.
                 */
                t -= delta;
                if(np->i != 0) {
                    np->i = 0;
                    np->encstate ^= 1;
                }
                break;
            }

            // Back up
            if(--np < tail && np->i != 1) {
                np->i++;                     // Search next best branch
                np->encstate ^= 1;
                break;
            }                                // else keep looking back
        }
    }
    *metric =  np->gamma;	                 // Return the final path metric

    // Copy decoded data to user's buffer
    nbits >>= 3;
    np = &nodes[7];

    fprintf(stderr, "copying into data nbits: %d\n", nbits);
    while(nbits-- != 0) {
        *data++ = np->encstate;
        fprintf(stderr, "  %2.2x\n", np->encstate);
        np += 8;
    }
    *cycles = i+1;

    free(nodes);
    if(i >= maxcycles) {
      fprintf(stderr, "i is %d, maxcycles is %d\n", i, maxcycles);
      return -1;	 // Decoder timed out
    }

    fprintf(stderr, "Successful Fano decode: i is %d, maxcycles is %d\n", i, maxcycles);
    return 0;		 // Successful completion
}
Fano::Fano(void) {
  // Setup metric table
  float bias=0.42;
  for(int i=0; i<256; i++) {
    mettab[0][i]=round( 10*(metric_tables[2][i]-bias) );
    mettab[1][i]=round( 10*(metric_tables[2][255-i]-bias) );
    fprintf(stderr, " %5d, %5d\n", mettab[0][i], mettab[1][i]);
  }
}
Fano::~Fano(void) {
}
