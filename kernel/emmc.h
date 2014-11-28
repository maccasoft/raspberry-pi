/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EMMC_H_
#define EMMC_H_

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct block_device
{
    char *driver_name;
    char *device_name;
    uint8_t *device_id;
    size_t dev_id_len;

    int supports_multiple_block_read;
    int supports_multiple_block_write;

    int (*read)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);
    int (*write)(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_num);
    size_t block_size;
    size_t num_blocks;

    struct fs *fs;
};

int sd_card_init(struct block_device **dev);

size_t sd_get_block_size();
size_t sd_get_num_blocks();

size_t sd_read(uint8_t *buf, size_t buf_size, uint32_t block_no);
size_t sd_write(uint8_t *buf, size_t buf_size, uint32_t block_no);

#if defined(__cplusplus)
}
#endif

#endif // EMMC_H_
