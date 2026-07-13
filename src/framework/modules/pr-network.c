#include <stdio.h>
#include "../../framework/framework.h"

// Guard compiler gate: Allow disabling curl if a platform lacks the dev packages
#define ENABLE_LIBCURL_BACKEND

#ifdef ENABLE_LIBCURL_BACKEND
#include <curl/curl.h>

// Standard libcurl callback matrix to stream network bytes straight to a local file
static size_t PR_CurlWriteCallback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}
#endif

bool PR_Network_DownloadFile(const char *url, const char *destPath) {
    if (url == NULL || destPath == NULL) return false;

#ifdef ENABLE_LIBCURL_BACKEND
    CURL *curlCtx = curl_easy_init();
    if (!curlCtx) {
        PR_PushKernelWarning("Network: Failed to initialize libcurl context.");
        return false;
    }

    // Open the local target destination file in binary write mode safely
    FILE *fp = fopen(destPath, "wb");
    if (!fp) {
        PR_PushKernelWarning("Network: Cannot open destination file for writing.");
        curl_easy_cleanup(curlCtx);
        return false;
    }

    // Configure the connection parameters for the request session
    curl_easy_setopt(curlCtx, CURLOPT_URL, url);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEFUNCTION, PR_CurlWriteCallback);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEDATA, fp);
    
    // CRITICAL FOR GITHUB: Instruct curl to follow 301/302 raw redirections automatically
    curl_easy_setopt(curlCtx, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Optional timeout fallback shield (e.g., abort if download hangs for 15 seconds)
    curl_easy_setopt(curlCtx, CURLOPT_TIMEOUT, 15L);

    // Execute the network transaction sequence synchronously
    CURLcode res = curl_easy_perform(curlCtx);
    
    // Safe resource cleanup closure pass
    fclose(fp);
    curl_easy_cleanup(curlCtx);

    if (res != CURLE_OK) {
        char errBuf[MAX_TERMINAL_ROW_LEN];
        PR_StrFormat(errBuf, sizeof(errBuf), "Network error: %s", curl_easy_strerror(res));
        PR_PushKernelWarning(errBuf);
        return false;
    }

    return true; // Transfer completed flawlessly
#else
    PR_PushKernelWarning("Network: Remote operations are disabled in this build configuration.");
    return false;
#endif
}
