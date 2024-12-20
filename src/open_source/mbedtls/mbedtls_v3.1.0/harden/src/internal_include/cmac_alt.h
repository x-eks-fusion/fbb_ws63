/* cmac_alt.h with dummy types for MBEDTLS_CMAC_ALT */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef CMAC_ALT_H
#define CMAC_ALT_H

struct mbedtls_cmac_context_t
{
    /** The internal state of the CMAC algorithm.  */
    unsigned char       MBEDTLS_PRIVATE(state)[MBEDTLS_CIPHER_BLKSIZE_MAX];

    /** Unprocessed data - either data that was not block aligned and is still
     *  pending processing, or the final block. */
    unsigned char       MBEDTLS_PRIVATE(unprocessed_block)[MBEDTLS_CIPHER_BLKSIZE_MAX];

    /** The length of data pending processing. */
    size_t              MBEDTLS_PRIVATE(unprocessed_len);
};

#endif /* cmac_alt.h */
