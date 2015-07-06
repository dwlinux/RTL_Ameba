#define CONFIG_SSL_RSA	1
#define CONFIG_SSL_SRP	0
#define POLARSSL_PEM_PARSE_C 1
#define POLARSSL_BASE64_C 1

#include "platform_stdlib.h"

#if CONFIG_SSL_RSA
#include "polarssl/config_rsa.h"
#elif CONFIG_SSL_SRP
#include "polarssl/config_srp.h"
#else
#include "polarssl/config_all.h"
#endif
