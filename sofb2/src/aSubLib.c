/* -*- mode: C; fill-column: 75; -*- */

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<dbAccess.h>
#include<dbDefs.h>
#include<dbFldTypes.h>
#include<registryFunction.h>
#include<aSubRecord.h>
#include<waveformRecord.h>
#include<epicsExport.h>
#include<epicsTime.h>
#include<epicsTypes.h>
#include<math.h>
#include<float.h>

#define NDEBUG

#define MERGE_INP(pval, dblk, val, sz)                  \
    if (dblk.type == CONSTANT) goto stats;              \
    memcpy(pval, val, sz);                              \
    pval += sz

static long mergePvs(aSubRecord *pasub)
{
    // output: waveform, avg, rms, min, max, std, var
    void *pval = pasub->vala;
    int i = 0;
    int szi = 1;
    double s1 = 0.0;
    double s2 = 0.0;
    double xmin = DBL_MAX;
    double xmax = DBL_MIN;
    double xstd = 0.0, xvar = 0.0, xrms = 0.0;
    /*
    if (pasub->inpa.type == CONSTANT) goto finish;
    memcpy(pval, pasub->a, pasub->noa * sizeof(double));
    pval += pasub->noa;
    */
    if (pasub->inpa.type == CONSTANT)  goto finish;
    
    assert(dbValueSize(pasub->ftva) == dbValueSize(pasub->fta));
    szi = dbValueSize(pasub->ftva);
    /* fprintf(stderr, "OUTA: %s\n", pasub->outa.value.pv_link.pvname); */
    /* fprintf(stderr, "OUTB: %s\n", pasub->outb.value.pv_link.pvname); */

    MERGE_INP(pval, pasub->inpa, pasub->a, pasub->noa*szi);
    MERGE_INP(pval, pasub->inpb, pasub->b, pasub->nob*szi);
    MERGE_INP(pval, pasub->inpc, pasub->c, pasub->noc*szi);
    MERGE_INP(pval, pasub->inpd, pasub->d, pasub->nod*szi);
    MERGE_INP(pval, pasub->inpe, pasub->e, pasub->noe*szi);
    MERGE_INP(pval, pasub->inpf, pasub->f, pasub->nof*szi);
    MERGE_INP(pval, pasub->inpg, pasub->g, pasub->nog*szi);
    MERGE_INP(pval, pasub->inph, pasub->h, pasub->noh*szi);
    MERGE_INP(pval, pasub->inpi, pasub->i, pasub->noi*szi);
    MERGE_INP(pval, pasub->inpj, pasub->j, pasub->noj*szi);
    MERGE_INP(pval, pasub->inpk, pasub->k, pasub->nok*szi);
    MERGE_INP(pval, pasub->inpl, pasub->l, pasub->nol*szi);
    MERGE_INP(pval, pasub->inpm, pasub->m, pasub->nom*szi);
    MERGE_INP(pval, pasub->inpn, pasub->n, pasub->non*szi);
    MERGE_INP(pval, pasub->inpo, pasub->o, pasub->noo*szi);
    MERGE_INP(pval, pasub->inpp, pasub->p, pasub->nop*szi);
    MERGE_INP(pval, pasub->inpq, pasub->q, pasub->noq*szi);
    MERGE_INP(pval, pasub->inpr, pasub->r, pasub->nor*szi);
    MERGE_INP(pval, pasub->inps, pasub->s, pasub->nos*szi);
    MERGE_INP(pval, pasub->inpt, pasub->t, pasub->not*szi);
    MERGE_INP(pval, pasub->inpu, pasub->u, pasub->nou*szi);

stats:
    if (pasub->ftva == epicsFloat64T) {
        double *pdval = (double*) pasub->vala;
        for (i = 0; i < pasub->nova; ++i) {
            s1 += pdval[i];
            s2 += pdval[i] * pdval[i];
            if (pdval[i] < xmin) xmin = pdval[i];
            if (pdval[i] > xmax) xmax = pdval[i];
        }
    } else if (pasub->ftva == epicsFloat32T) {
        float *pfval = (float*) pasub->vala;
        for (i = 0; i < pasub->nova; ++i) {
            s1 += pfval[i];
            s2 += pfval[i] * pfval[i];
            if (pfval[i] < xmin) xmin = pfval[i];
            if (pfval[i] > xmax) xmax = pfval[i];
        }
    } else if (pasub->ftva == epicsUInt8T) {
        epicsUInt8 *pfval = (epicsUInt8*) pasub->vala;
        for (i = 0; i < pasub->nova; ++i) {
            s1 += pfval[i];
            s2 += pfval[i] * pfval[i];
            if (pfval[i] < xmin) xmin = pfval[i];
            if (pfval[i] > xmax) xmax = pfval[i];
        }
    } else {
        fprintf(stderr, "Unknow ftva: %d (%d %d %d %d)\n", pasub->ftva,
                epicsUInt8T, epicsInt8T,
                epicsUInt16T, epicsInt16T);
        goto finish;
    }
    double ss = s1;
    s2 /= pasub->nova;
    s1 /= pasub->nova;
    xrms = sqrt(s2);
    xvar = s2 - s1 * s1;
    xstd = sqrt(xvar);
    /* sum, avg, rms, min, max, std, var */
    if (pasub->outb.type == CONSTANT) goto finish;
    *((double*)pasub->valb) = ss;
    if (pasub->outc.type == CONSTANT) goto finish;
    memcpy(pasub->valc, &s1, sizeof(double));
    if (pasub->outd.type == CONSTANT) goto finish;
    memcpy(pasub->vald, &xrms, sizeof(double));
    if (pasub->oute.type == CONSTANT) goto finish;
    memcpy(pasub->vale, &xmin, sizeof(double));
    if (pasub->outf.type == CONSTANT) goto finish;
    memcpy(pasub->valf, &xmax, sizeof(double));
    if (pasub->outg.type == CONSTANT) goto finish;
    memcpy(pasub->valg, &xstd, sizeof(double));
    if (pasub->outh.type == CONSTANT) goto finish;
    memcpy(pasub->valh, &xvar, sizeof(double));

finish:
    return 0;
}

#define SPLIT_INP(pval, dblk, val, sz)                  \
    if (dblk.type == CONSTANT) goto finish;             \
    memcpy(val, pval, sz * sizeof(double));          \
    pval += sz

static long splitPvs(aSubRecord *pasub)
{
    double *pval = (double *)pasub->a;

    /* fprintf(stderr, "%s [%d] %f %f %f\n", */
    /*         pasub->name, */
    /*         pasub->noa, pval[0], pval[1], pval[2]); */

    /*
    if (pasub->inpa.type == CONSTANT) goto finish;
    memcpy(pval, pasub->a, pasub->noa * sizeof(double));
    pval += pasub->noa;
    */
    SPLIT_INP(pval, pasub->outa, pasub->vala, pasub->nova);
    SPLIT_INP(pval, pasub->outb, pasub->valb, pasub->novb);
    SPLIT_INP(pval, pasub->outc, pasub->valc, pasub->novc);
    SPLIT_INP(pval, pasub->outd, pasub->vald, pasub->novd);
    SPLIT_INP(pval, pasub->oute, pasub->vale, pasub->nove);
    SPLIT_INP(pval, pasub->outf, pasub->valf, pasub->novf);
    SPLIT_INP(pval, pasub->outg, pasub->valg, pasub->novg);
    SPLIT_INP(pval, pasub->outh, pasub->valh, pasub->novh);
    SPLIT_INP(pval, pasub->outi, pasub->vali, pasub->novi);
    SPLIT_INP(pval, pasub->outj, pasub->valj, pasub->novj);
    SPLIT_INP(pval, pasub->outk, pasub->valk, pasub->novk);
    SPLIT_INP(pval, pasub->outl, pasub->vall, pasub->novl);
    SPLIT_INP(pval, pasub->outm, pasub->valm, pasub->novm);
    SPLIT_INP(pval, pasub->outn, pasub->valn, pasub->novn);
    SPLIT_INP(pval, pasub->outo, pasub->valo, pasub->novo);
    SPLIT_INP(pval, pasub->outp, pasub->valp, pasub->novp);
    SPLIT_INP(pval, pasub->outq, pasub->valq, pasub->novq);
    SPLIT_INP(pval, pasub->outr, pasub->valr, pasub->novr);
    SPLIT_INP(pval, pasub->outs, pasub->vals, pasub->novs);
    SPLIT_INP(pval, pasub->outt, pasub->valt, pasub->novt);
    SPLIT_INP(pval, pasub->outu, pasub->valu, pasub->novu);

finish:
 #ifndef NDEBUG
    fprintf(stderr, "%d: %f %f %f\n", pasub->nova, ((double*)pasub->vala)[0],
            ((double*)pasub->vala)[1], ((double*)pasub->vala)[2]);

    fprintf(stderr, "%d: %f\n", pasub->novb, *(double*)pasub->valb);
    fprintf(stderr, "%d: %f\n", pasub->novc, *(double*)pasub->valc);
 #endif
    return 0;
}

static long splitMarkedVals(aSubRecord *prec)
{
    int i = 0, j = 0, k = 0;
    double *src = (double *)prec->a;

    const int NUM_ARGS = 21;
    /* Check Output Links */
    for (i = 0; i < NUM_ARGS; ++i) {
        /* (&prec->noa)[i], (&prec->inpa)[i], (&prec->a)[i] */
        struct link *plink = &(&prec->outa)[i];
        if (plink->type == CONSTANT) {
         #ifndef NDEBUG
            fprintf(stderr, "CONSTANT link: %d\n", i);
         #endif
            continue;
        }
        double *dst = (double*) (&prec->vala)[i];
        const int NOV = (&prec->nova)[i];
     #ifndef NDEBUG
        fprintf(stderr, "  capacity of link %d: [%d], ", i, NOV);
     #endif
        int k = 0;
        while (k < NOV && j < prec->nea) {
            dst[k++] = src[j++];
        }
        if (NOV > 1 || (NOV == 1 && j < prec->nea && src[j] > 0.0)) {
            dbPutLink(&(&prec->outa)[i], (&prec->ftva)[i],
                       (&prec->vala)[i], (&prec->neva)[i]);
        } else {
         #ifndef NDEBUG
            fprintf(stderr, "NOV= %d, j= %d, src[j]= %f\n", NOV, j, src[j]);
         #endif
        }
     #ifndef NDEBUG
        fprintf(stderr, "  link %d  j= %d, realsize= %d\n", i, j, k);
     #endif
        (&prec->neva)[i] = k;
        if (j % 2 == 1) ++j;
    }

finish:
    if (&(prec->outa) != CONSTANT && j < prec->nea) {
        memcpy(prec->vala, (double*) prec->a + j, (prec->nea - j)*sizeof(double));
        memcpy(prec->valb, (short*) prec->b + j, (prec->nea - j)*sizeof(short));
    }
 #ifndef NDEBUG
    fprintf(stderr, "RETURN\n");
 #endif

    return -1;
}

static long maskWaveforms(aSubRecord *prec)
{
    short *sel = (short*) prec->a;   /* i <- a[i] (SHORT) */
    int i = 0;
    const int NUM_ARGS = 21;
    /* Check Output Links */
    for (i = 1; i < NUM_ARGS; ++i) {
        /* (&prec->noa)[i], (&prec->inpa)[i], (&prec->a)[i] */
        struct link *plink = &(&prec->outa)[i];
        if (plink->type == CONSTANT) {
            /*
         #ifndef NDEBUG
            fprintf(stderr, "CONSTANT link: %d\n", i);
         #endif
            */
            continue;
        }
        double *src = (double*) (&prec->a)[i];
        double *dst = (double*) (&prec->vala)[i];
        const int NOV = (&prec->nova)[i];
     #ifndef NDEBUG
        fprintf(stderr, "  capacity of link %d: [%d], %d ", i, NOV, prec->nea);
     #endif
        int k = 0, j = 0;
        while ( j < NOV && k < prec->noa) {
            /* fprintf(stderr, "%d: %d\n", k, sel[k]);  */
            if (sel[k] > 0) dst[j++] = src[k++];
            else ++k;
        }
        /* #ifndef NDEBUG
        fprintf(stderr, "  link %d  j= %d, k= %d\n", i, j, k);
         #endif */
        (&prec->neva)[i] = j;
    }

    return 0;
}

static long shuffleData(void *pdst, int n, short *idx, void *psrc, int dtype)
{
  /* fprintf(stderr, "%d, %d %d %d\n", n, idx[0], idx[1], idx[2]); */
  int i = 0;
  if (dtype == epicsInt8T) {
    /* fprintf(stderr, "shuffle int8 data\n"); */
    epicsInt8 *src = (epicsInt8*) psrc;
    epicsInt8 *dst = (epicsInt8*) pdst;
    for (i = 0; i < n; ++i) {
      dst[i] = src[idx[i]];
    }
  } else if (dtype == epicsFloat64T) {
    /* fprintf(stderr, "shuffle float64 data\n"); */
    epicsFloat64 *src = (epicsFloat64*) psrc;
    epicsFloat64 *dst = (epicsFloat64*) pdst;
    for (i = 0; i < n; ++i) {
      /* fprintf(stderr, "  move %d -> %d\n", i, idx[i]); */
      dst[i] = src[idx[i]];
    }
    
  } else {
    fprintf(stderr, "invalid data type: %d\n", dtype);
  }

  return 0;
}


#define SHUFFLE_WFM(INP, INPOUT, INPVAL, INPFTV, INPNEV) \
    if (INPOUT.type != CONSTANT) { \
        shuffleData(INPVAL, pasub->nea, idx, INP, INPFTV); \
        INPNEV = pasub->nea; \
    }

static long shuffleWaveforms(aSubRecord *prec)
{
    /* fprintf(stderr, "in shuffleWaveforms:\n"); */
    if (prec->inpa.type == CONSTANT)  goto finish;

    if (prec->fta != epicsInt16T && prec->fta != epicsUInt16T) {
        goto finish;
    }
    /* fprintf(stderr, "checking ftvb: %d\n", prec->ftvb); */
    int i = 0, j = 0, k = 0;
    epicsInt16 *idx = (epicsInt16*) prec->a;   /* new position [0, noa) */
    for (i = 1; i < 21; ++i) {
        struct link *plink = &(&prec->outa)[i];
        if (plink->type == CONSTANT) {
         #ifndef NDEBUG
            fprintf(stderr, "CONSTANT link: %d\n", i);
         #endif
            continue;
        }

        /* void *src = (&prec->a)[i]; */
        /* void *dst = (&prec->vala)[i]; */
        /* double *src = (double*)(&prec->a)[i]; */
        /* double *dst = (double*)(&prec->vala)[i]; */
        const unsigned char *src = (&prec->a)[i];
        unsigned char *dst = (&prec->vala)[i];
        int szi = dbValueSize((&prec->fta)[i]);
     #ifndef NDEBUG
        fprintf(stderr, "data %d size= %d [%d]\n", i, szi, prec->nea);
     #endif
        for (j = 0; j < prec->nea; ++j) {
            /* copy src[idx[j]] to dst[j] */
            /* fprintf(stderr, "COPY %d <- %d, ", j, idx[j]); */
            for (k = 0; k < szi; ++k) dst[j*szi+k] = src[idx[j]*szi+k];
            /* memcpy(dst+j, src+idx[j], 4*szi*sizeof(char)); */
            /* dst[j] = src[idx[j]]; */
            /* fprintf(stderr, "%g %g\n", ((double*)dst)[j], ((double*)src)[idx[j]]); */
        }
        (&prec->neva)[i] = prec->nea;
    }

 finish:
    return 0;
}

epicsRegisterFunction(mergePvs);
epicsRegisterFunction(splitPvs);
epicsRegisterFunction(shuffleWaveforms);
epicsRegisterFunction(maskWaveforms);
epicsRegisterFunction(splitMarkedVals);
