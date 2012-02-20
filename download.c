#include "util.h"

#include <curl/curl.h>
#include <unistd.h>             /* unlink */
#include <string.h>
#include <errno.h>

struct dcontext {
    char *d_url;                  /* the to be downloaded URL. Owned by this context */
    char *d_savefile;             /* filename of the file to which we save content */
    char *d_tofile;               /* filename of the final destination */

    /* private part */
    CURL *d_handle;
    char  d_ebuffer[CURL_ERROR_SIZE];

    FILE *d_savefp;

    char *d_cdfile;             /* content-disposition filename */
    
    curl_progress_callback d_progress_cb;
};


struct dcontext *
dcontext_new()
{
    return zmalloc(sizeof(struct dcontext));
}

void
dcontext_steal_string(char **dst, char *src)
{
    xfree(*dst);
    *dst = src;
}

void
dcontext_set_string(char **dst, const char *src)
{
    xfree(*dst);
    *dst = xstrdup(src);
}

void
dcontext_steal_url(struct dcontext *dctx, char *url)
{
    dcontext_steal_string(&dctx->d_url, url);
}

void
dcontext_set_url(struct dcontext *dctx, const char *url)
{
    dcontext_set_string(&dctx->d_url, url);
}

void
dcontext_steal_savefile(struct dcontext *dctx, char *savefile)
{
    dcontext_steal_string(&dctx->d_savefile, savefile);
}

void
dcontext_set_savefile(struct dcontext *dctx, const char *savefile)
{
    dcontext_set_string(&dctx->d_savefile, savefile);
}

void
dcontex_steal_tofile(struct dcontext *dctx, char *tofile)
{
    dcontext_steal_string(&dctx->d_tofile, tofile);
}

void
dcontext_set_tofile(struct dcontext *dctx, const char *tofile)
{
    dcontext_set_string(&dctx->d_tofile, tofile);
}

int
dcontext_prepare(struct dcontext *dctx)
{
    sassert(dctx);
    sassert(dctx->d_savefile);
    
    CURL *c = dctx->d_handle = curl_easy_init();

    curl_easy_reset(c);

    curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(c, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_LIMIT, 1024L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 10L);
    curl_easy_setopt(c, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);    

    
    curl_easy_setopt(c, CURLOPT_URL, dctx->d_url);

    curl_easy_setopt(c, CURLOPT_ERRORBUFFER, &dctx->d_ebuffer[0]);


    /* no progress for now */
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 1L);
#if 0
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, dcontext_progress_func);
    curl_easy_setopt(c, CURLOPT_PROGRESSDATA, dctx);
#endif

    /* no header function for now */
#if 0
    curl_easy_setopt(c, CURLOPT_HEADERFUNCTION, dcontext_header_func);
    curl_easy_setopt(c, CURLOPT_WRITEHEADER, dctx);
#endif
    
    if ((dctx->d_savefp = fopen(dctx->d_savefile, "wb")) == NULL) {
        goto error;
    }

    curl_easy_setopt(c, CURLOPT_WRITEDATA, dctx->d_savefp);

    return 0;
    
error:
    fprintf(stderr, "failed: %s\n", strerror(errno));
    return -1;
}

int
dcontext_download(struct dcontext *dctx)
{
    CURL *c = dctx->d_handle;
    CURLcode curl_error = curl_easy_perform(c);

    long response_code = 0;
    long remote_time = -1;
    double remote_size;
    double download_size;

    char *effective_url;
    
    switch (curl_error) {
        case CURLE_OK:
            curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code >= 400) {
                goto error;
            }

            break;
        default:
            goto error;
    }

    curl_easy_getinfo(c, CURLINFO_FILETIME, &remote_time);
    curl_easy_getinfo(c, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &remote_size);
    curl_easy_getinfo(c, CURLINFO_SIZE_DOWNLOAD, &download_size);
    curl_easy_getinfo(c, CURLINFO_EFFECTIVE_URL, &effective_url);

    if (!double_eq(remote_size, -1) &&
        !double_eq(download_size, -1) &&
        !double_eq(remote_size, download_size)) {
        goto error;
    }

#if 0
    if (dctx->d_cdfile) {
        dcontext_set_tofile(dctx, dctx->d_cdfile);
    } else {
        char *effective_filename = filename_from_url(effective_url);
        dcontext_set_tofile(dctx, effective_filename);
    }
#endif
    return 0;

error:
    return -1;
}


int
dcontext_commit(struct  dcontext *dctx)
{
    if (dctx->d_tofile) {
        if (rename(dctx->d_savefile, dctx->d_tofile) == -1) {
            fprintf(stderr, "dcontext_commit: failed to rename %s to %s\n",
                    dctx->d_savefile, dctx->d_tofile);
            goto error;
        }
    }

    return 0;
error:
    unlink(dctx->d_savefile);
    return -1;
}


int download(const char *url, const char *tofile)
{
    struct dcontext *dctx = dcontext_new();

    dcontext_set_url(dctx, url);
    dcontext_set_savefile(dctx, tofile);
    dcontext_set_tofile(dctx, tofile);

    if (dcontext_prepare(dctx)) {
        fprintf(stderr, "dcontext_prepare failed\n");
        return -1;
    }

    if (dcontext_download(dctx)) {
        fprintf(stderr, "dcontext_download failed\n");
        return -1;
    }

    if (dcontext_commit(dctx)) {
        fprintf(stderr, "dcontext_commit failed\n");
        return -1;
    }

    return 0;
}


