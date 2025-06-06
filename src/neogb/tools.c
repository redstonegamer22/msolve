/* This file is part of msolve.
 * msolve is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * msolve is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with msolve.  If not, see <https://www.gnu.org/licenses/>
 *
 * Authors:
 * Jérémy Berthomieu
 * Christian Eder
 * Mohab Safey El Din */


#include "tools.h"
#include <sys/stat.h>

/* cpu time */
double cputime(void)
{
	double t;
	t =   CLOCKS_PER_SEC / 100000.;
	t +=  (double)clock();
	return t / CLOCKS_PER_SEC;
}

/* wall time */
double realtime(void)
{
        struct timeval t;
        gettimeofday(&t, NULL);
        t.tv_sec -= (2017 - 1970)*3600*24*365;
        return (1. + (double)t.tv_usec + ((double)t.tv_sec*1000000.)) / 1000000.;
}

int save_matrices = 0;

static void format_duration(double dur, char *buf, size_t n)
{
    int h = (int)(dur / 3600.0);
    dur -= h * 3600.0;
    int m = (int)(dur / 60.0);
    dur -= m * 60.0;
    int s = (int)dur;
    int ms = (int)((dur - s) * 1000.0 + 0.5);
    snprintf(buf, n, "%02d_%02d_%02d.%03d", h, m, s, ms);
}

void dump_dense_matrix_cf32(cf32_t **mat, len_t nrows, len_t ncols,
                            int reduced, double duration)
{
    if (!save_matrices || !mat)
        return;

    mkdir("matrix_archive", 0755);

    unsigned int id = (unsigned int)(rand() & 0xFFFFFF);
    char ts[32] = "";
    char fname[256];
    if (reduced) {
        format_duration(duration, ts, sizeof(ts));
        snprintf(fname, sizeof(fname),
                 "matrix_archive/rref_%lu_%lu_%s_%06x.smat",
                 (unsigned long)nrows, (unsigned long)ncols, ts, id);
    } else {
        snprintf(fname, sizeof(fname),
                 "matrix_archive/unrref_%lu_%lu_%06x.smat",
                 (unsigned long)nrows, (unsigned long)ncols, id);
    }

    FILE *f = fopen(fname, "w");
    if (!f)
        return;

    size_t nnz = 0;
    if (!reduced) {
        for (len_t i = 0; i < nrows; ++i) {
            if (!mat[i])
                continue;
            for (len_t j = 0; j < ncols; ++j) {
                if (mat[i][j] != 0)
                    nnz++;
            }
        }
    } else {
        for (len_t idx = 0; idx < ncols; ++idx) {
            len_t m = ncols - 1 - idx;
            if (!mat[m])
                continue;
            len_t len = ncols - m;
            for (len_t j = 0; j < len; ++j) {
                if (mat[m][j] != 0)
                    nnz++;
            }
        }
    }

    fprintf(f, "%lu %lu %lu\n", (unsigned long)nrows, (unsigned long)ncols,
            (unsigned long)nnz);

    if (!reduced) {
        for (len_t i = 0; i < nrows; ++i) {
            if (!mat[i])
                continue;
            for (len_t j = 0; j < ncols; ++j) {
                if (mat[i][j] != 0) {
                    fprintf(f, "%lu %lu %u\n", (unsigned long)i,
                            (unsigned long)j, (unsigned)mat[i][j]);
                }
            }
        }
    } else {
        len_t ridx = 0;
        for (len_t idx = 0; idx < ncols; ++idx) {
            len_t m = ncols - 1 - idx;
            if (!mat[m])
                continue;
            len_t len = ncols - m;
            for (len_t j = 0; j < len; ++j) {
                if (mat[m][j] != 0) {
                    fprintf(f, "%lu %lu %u\n", (unsigned long)ridx,
                            (unsigned long)(m + j), (unsigned)mat[m][j]);
                }
            }
            ridx++;
        }
    }

    fclose(f);
}

void dump_sparse_matrix_cf32(mat_t *mat, const bs_t *tbr, const bs_t *bs,
                             int reduced, double duration)
{
    if (!save_matrices)
        return;

    mkdir("matrix_archive", 0755);

    unsigned int id = (unsigned int)(rand() & 0xFFFFFF);
    len_t ncols = mat->nc;
    len_t nrows = reduced ? mat->np : mat->nru + mat->nrl;

    char ts[32] = "";
    char fname[256];
    if (reduced) {
        format_duration(duration, ts, sizeof(ts));
        snprintf(fname, sizeof(fname),
                 "matrix_archive/rref_%lu_%lu_%s_%06x.smat",
                 (unsigned long)nrows, (unsigned long)ncols, ts, id);
    } else {
        snprintf(fname, sizeof(fname),
                 "matrix_archive/unrref_%lu_%lu_%06x.smat",
                 (unsigned long)nrows, (unsigned long)ncols, id);
    }

    FILE *f = fopen(fname, "w");
    if (!f)
        return;

    size_t nnz = 0;
    if (!reduced) {
        for (len_t i = 0; i < mat->nru; ++i) {
            if (mat->rr[i])
                nnz += mat->rr[i][LENGTH];
        }
        for (len_t i = 0; i < mat->nrl; ++i) {
            if (mat->tr[i])
                nnz += mat->tr[i][LENGTH];
        }
    } else {
        for (len_t i = 0; i < mat->np; ++i) {
            if (mat->tr[i])
                nnz += mat->tr[i][LENGTH];
        }
    }

    fprintf(f, "%lu %lu %lu\n", (unsigned long)nrows, (unsigned long)ncols,
            (unsigned long)nnz);

    if (!reduced) {
        len_t idx = 0;
        for (len_t i = 0; i < mat->nru; ++i, ++idx) {
            hm_t *row = mat->rr[i];
            if (!row)
                continue;
            cf32_t *cfs = bs->cf_32[row[COEFFS]];
            len_t len = row[LENGTH];
            hm_t *cols = row + OFFSET;
            for (len_t j = 0; j < len; ++j) {
                fprintf(f, "%lu %lu %u\n", (unsigned long)idx,
                        (unsigned long)cols[j], (unsigned)cfs[j]);
            }
        }
        for (len_t i = 0; i < mat->nrl; ++i, ++idx) {
            hm_t *row = mat->tr[i];
            if (!row)
                continue;
            cf32_t *cfs = tbr->cf_32[row[COEFFS]];
            len_t len = row[LENGTH];
            hm_t *cols = row + OFFSET;
            for (len_t j = 0; j < len; ++j) {
                fprintf(f, "%lu %lu %u\n", (unsigned long)idx,
                        (unsigned long)cols[j], (unsigned)cfs[j]);
            }
        }
    } else {
        for (len_t i = 0; i < mat->np; ++i) {
            hm_t *row = mat->tr[i];
            if (!row)
                continue;
            cf32_t *cfs = mat->cf_32[row[COEFFS]];
            len_t len = row[LENGTH];
            hm_t *cols = row + OFFSET;
            for (len_t j = 0; j < len; ++j) {
                fprintf(f, "%lu %lu %u\n", (unsigned long)i,
                        (unsigned long)cols[j], (unsigned)cfs[j]);
            }
        }
    }

    fclose(f);
}

static void construct_trace(
        trace_t *trace,
        mat_t *mat
        )
{
    len_t i, j;
    len_t ctr = 0;

    const len_t ld  = trace->ltd;
    const len_t nru = mat->nru;
    const len_t nrl = mat->nrl;
    rba_t **rba     = mat->rba;

    /* check if there are any non zero new elements, otherwise we
     * do not need to do this matrix at all in the application steps. */
    i = 0;
    while (i < nrl && mat->tr[i] == NULL) {
        ++i;
    }
    if (i == nrl) {
        return;
    }

    /* non zero new elements exist */
    if (trace->ltd == trace->std) {
        trace->std  *=  2;
        trace->td   =   realloc(trace->td,
                (unsigned long)trace->std * sizeof(td_t));
        memset(trace->td+trace->std/2, 0,
                (unsigned long)trace->std/2 * sizeof(td_t));
    }

    const unsigned long lrba = nru / 32 + ((nru % 32) != 0);

    rba_t *reds = (rba_t *)calloc(lrba, sizeof(rba_t));

    for (i = 0; i < nrl; ++i) {
        if (mat->tr[i] != NULL) {
            rba[ctr]  = rba[i];
            ctr++;
        } else {
            free(rba[i]);
            rba[i]  = NULL;
        }
    }
    mat->rbal = ctr;
    mat->rba  = rba = realloc(rba, (unsigned long)mat->rbal * sizeof(rba_t *));

    const len_t ntr = ctr;

    /* construct rows to be reduced */
    trace->td[ld].tri  = realloc(trace->td[ld].tri,
            (unsigned long)ntr * 2 * sizeof(len_t));
    trace->td[ld].tld = 2 * ntr;

    ctr = 0;
    for (i = 0; i < nrl; ++i) {
        if (mat->tr[i] != NULL) {
            trace->td[ld].tri[ctr++]  = mat->tr[i][BINDEX];
            trace->td[ld].tri[ctr++]  = mat->tr[i][MULT];
        }
    }
    /* get all needed reducers */
    for (i = 0; i < ntr; ++i) {
        for (j = 0; j < lrba; ++j) {
            reds[j] |= rba[i][j];
        }
    }

    /* construct rows to reduce with */
    trace->td[ld].rri  = realloc(trace->td[ld].rri,
            (unsigned long)nru * 2 * sizeof(len_t));
    trace->td[ld].rld = 2 * nru;

    ctr = 0;
    for (i = 0; i < nru; ++i) {
        if (reds[i/32] >> (i%32) & 1U) {
            trace->td[ld].rri[ctr++]  = mat->rr[i][BINDEX];
            trace->td[ld].rri[ctr++]  = mat->rr[i][MULT];
        }
    }
    trace->td[ld].rri = realloc(trace->td[ld].rri,
            (unsigned long)ctr * sizeof(len_t));
    trace->td[ld].rld = ctr;
    const len_t nrr   = ctr;

    /* new length of rbas, useless reducers removed,
     * we only need nrr/2 since we do not store the
     * multipliers in rba */
    const unsigned long nlrba = nrr / 2 / 32 + (((nrr / 2) % 32) != 0);

    /* construct rba information */
    trace->td[ld].rba = realloc(trace->td[ld].rba,
            (unsigned long)ntr * sizeof(rba_t *));
    /* allocate memory     */
    for (i = 0; i < ntr; ++i) {
        trace->td[ld].rba[i]  = calloc(nlrba, sizeof(rba_t));
    }

    ctr = 0;
    /* write new rbas for tracer with useless reducers removed */
    for (i = 0; i < nru; ++i) {
        if (reds[i/32] >> (i%32) & 1U) {
            for (j = 0; j < ntr; ++j) {
                trace->td[ld].rba[j][ctr/32] |=
                    ((rba[j][i/32] >> i%32) & 1U) << ctr%32;
            }
            ctr++;
        }
    }
    free(reds);

    trace->td[ld].deg = mat->cd;
}

/* Only trace reducer rows for saturation steps to keep
 * multiplication of saturated elements more flexible. */
static void construct_saturation_trace(
        trace_t *trace,
        len_t pos,
        mat_t *mat
        )
{
    len_t i;
    len_t ctr = 0;

    const len_t ld  = pos;
    const len_t nru = mat->nru;


    /* construct rows to reduce with */
    trace->ts[ld].rri  = realloc(trace->ts[ld].rri,
            (unsigned long)nru * 2 * sizeof(len_t));
    trace->ts[ld].rld = 2 * nru;

    ctr = 0;
    for (i = 0; i < nru; ++i) {
        trace->ts[ld].rri[ctr++]  = mat->rr[i][BINDEX];
        trace->ts[ld].rri[ctr++]  = mat->rr[i][MULT];
    }
    trace->ts[ld].rld  = ctr;
}

static void add_lms_to_trace(
        trace_t *trace,
        const bs_t * const bs,
        const len_t np
        )
{
    len_t i;

    const len_t ld      = trace->ltd;
    trace->td[ld].nlms  = realloc(trace->td[ld].nlms,
            (unsigned long)np * sizeof(hm_t));

    for (i = 0; i < np; ++i) {
        trace->td[ld].nlms[i]  = bs->hm[bs->ld + i][OFFSET];
    }
    trace->td[ld].nlm = np;
}

#if 0
static void add_minimal_lmh_to_trace(
        trace_t *trace,
        const bs_t * const bs
        )
{
    len_t i;

    const len_t ld    = trace->lts;
    const len_t lml   = bs->lml;
    /* trace->ts[ld].lmh = realloc(trace->ts[ld].lmh,
            (unsigned long)lml * sizeof(hm_t));

    for (i = 0; i < lml; ++i) {
        trace->ts[ld].lmh[i]  = bs->hm[bs->lmps[i]][OFFSET];
    } */
    trace->ts[ld].lml = lml;
}
#endif

/* 
 * static inline val_t compare_and_swap(
 *         long *ptr,
 *         long old,
 *         long new
 *         )
 * {
 *     val_t prev;
 *
 * #if 0
 *     __asm__ __volatile__(
 *             "lock; cmpxchgl %2, %1" : "=a"(prev),
 *             "+m"(*ptr) : "r"(new), "0"(old) : "memory");
 *     [> on which systems do we need "cmpxchgq" instead of "cmpxchgl" ? <]
 * #else
 *     __asm__ __volatile__(
 *             "lock; cmpxchgq %2, %1" : "=a"(prev),
 *             "+m"(*ptr) : "r"(new), "0"(old) : "memory");
 * #endif
 *     return prev;
 * } */
