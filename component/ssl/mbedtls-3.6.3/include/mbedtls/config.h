#include "platform_opts.h"

/* RTL_CRYPTO_FRAGMENT should be 16bytes-aligned */
#if defined(CONFIG_PLATFORM_8735B)
#define RTL_CRYPTO_FRAGMENT               65536 // 64k bytes
#else
#define RTL_CRYPTO_FRAGMENT               15360
#endif

#if defined(CONFIG_PLATFORM_8735B)
#define CONFIG_SSL_RESUME                 1
#endif

#include "mbedtls/mbedtls_config.h"
