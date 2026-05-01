#include "virtio_blk.h"
#include "mm.h"
#include "vm.h"
#include "kprintf.h"

// QEMU virt: virtio-mmio-bus.0 lives at PA 0x10001000
#define VIRTIO_MMIO_BASE  0x10001000UL

// Legacy (version 1) MMIO register offsets
#define REG_MAGIC         0x000  // must read 0x74726976 ("virt")
#define REG_VERSION       0x004  // must be 1 for legacy
#define REG_DEVICE_ID     0x008  // 2 = block device
#define REG_HOST_FEAT     0x010  // device-offered features (R)
#define REG_GUEST_FEAT    0x020  // driver-accepted features (W)
#define REG_GUEST_PAGE    0x028  // guest page size in bytes (W)
#define REG_QUEUE_SEL     0x030  // select which virtqueue (W)
#define REG_QUEUE_MAX     0x034  // max queue size for selected queue (R)
#define REG_QUEUE_NUM     0x038  // actual queue size we'll use (W)
#define REG_QUEUE_ALIGN   0x03C  // byte alignment between avail and used rings (W)
#define REG_QUEUE_PFN     0x040  // queue base PA >> PAGE_SHIFT (W)
#define REG_QUEUE_NOTIFY  0x050  // write queue index to kick device (W)
#define REG_INTR_STATUS   0x060  // interrupt reason bitmap (R)
#define REG_INTR_ACK      0x064  // write back to clear interrupt (W)
#define REG_STATUS        0x070  // device status register (R/W)

// Status register bits — written in accumulating order during init
#define STAT_ACK        (1 << 0)
#define STAT_DRIVER     (1 << 1)
#define STAT_DRIVER_OK  (1 << 2)

#define QUEUE_SIZE    8

#define DESC_F_NEXT   1   // descriptor chains to desc[next]
#define DESC_F_WRITE  2   // device writes into this buffer (not driver)

// Virtqueue structures — the shared memory interface between driver and device.
// Descriptors describe buffers; the driver posts work via the avail ring;
// the device reports completion via the used ring.
struct virtq_desc {
    uint64_t addr;   // buffer physical address
    uint32_t len;
    uint16_t flags;
    uint16_t next;   // index of next descriptor if DESC_F_NEXT is set
};

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;                // next slot driver will write
    uint16_t ring[QUEUE_SIZE];   // descriptor head indices
    uint16_t used_event;
};

struct virtq_used_elem { uint32_t id; uint32_t len; };

struct virtq_used {
    uint16_t flags;
    uint16_t idx;                // next slot device will write
    struct virtq_used_elem ring[QUEUE_SIZE];
    uint16_t avail_event;
};

// Legacy virtio layout with QueueAlign = PAGE_SIZE:
//   page 0 offset 0:           descriptor table  (QUEUE_SIZE * 16 bytes)
//   page 0 after descriptors:  avail ring
//   page 1 offset 0:           used ring
// Two pages must be physically contiguous — static BSS guarantees this.
static uint8_t virtq_pages[2 * PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

static struct virtq_desc          *descs;
static struct virtq_avail         *avail;
static volatile struct virtq_used *used;   // volatile: device writes this via DMA

static volatile uint32_t *mmio;

static uint32_t reg_r(uint32_t off) { return mmio[off >> 2]; }
static void     reg_w(uint32_t off, uint32_t val) { mmio[off >> 2] = val; }

void virtio_blk_init(void) {
    mmio = (volatile uint32_t *)pa2kva(VIRTIO_MMIO_BASE);

    if (reg_r(REG_MAGIC)     != 0x74726976) kpanic("virtio_blk: bad magic\n");
    if (reg_r(REG_VERSION)   != 1)          kpanic("virtio_blk: need legacy (version 1)\n");
    if (reg_r(REG_DEVICE_ID) != 2)          kpanic("virtio_blk: not a block device\n");

    reg_w(REG_STATUS, 0);                           // reset
    reg_w(REG_STATUS, STAT_ACK);
    reg_w(REG_STATUS, STAT_ACK | STAT_DRIVER);

    reg_w(REG_GUEST_FEAT, 0);        // accept no optional features
    reg_w(REG_GUEST_PAGE, PAGE_SIZE);

    reg_w(REG_QUEUE_SEL, 0);
    if (reg_r(REG_QUEUE_MAX) < QUEUE_SIZE)
        kpanic("virtio_blk: queue too small\n");
    reg_w(REG_QUEUE_NUM,   QUEUE_SIZE);
    reg_w(REG_QUEUE_ALIGN, PAGE_SIZE);
    reg_w(REG_QUEUE_PFN,   (uint32_t)(kva2pa(virtq_pages) >> 12));

    descs = (struct virtq_desc  *) virtq_pages;
    avail = (struct virtq_avail *)(virtq_pages + QUEUE_SIZE * sizeof(struct virtq_desc));
    used  = (struct virtq_used  *)(virtq_pages + PAGE_SIZE);

    reg_w(REG_STATUS, STAT_ACK | STAT_DRIVER | STAT_DRIVER_OK);
    kprintf("virtio_blk: ok\n");
}

// Request header and status live here between submission and completion.
// Only one request in flight at a time, so static storage is fine.
struct virtio_blk_req {
    uint32_t type;      // BLK_T_IN = 0 (read), BLK_T_OUT = 1 (write)
    uint32_t reserved;
    uint64_t sector;
};

#define BLK_T_IN  0

static struct virtio_blk_req  blk_req    __attribute__((aligned(16)));
static volatile uint8_t       blk_status;

void virtio_blk_read(uint64_t sector, void *buf, uint32_t nsectors) {
    blk_req.type     = BLK_T_IN;
    blk_req.reserved = 0;
    blk_req.sector   = sector;
    blk_status       = 0xFF;

    // desc[0]: request header — driver fills, device reads
    descs[0].addr  = kva2pa(&blk_req);
    descs[0].len   = sizeof(blk_req);
    descs[0].flags = DESC_F_NEXT;
    descs[0].next  = 1;

    // desc[1]: data buffer — device writes nsectors*512 bytes into buf
    descs[1].addr  = kva2pa(buf);
    descs[1].len   = nsectors * SECTOR_SIZE;
    descs[1].flags = DESC_F_WRITE | DESC_F_NEXT;
    descs[1].next  = 2;

    // desc[2]: status byte — device writes 0 on success
    descs[2].addr  = kva2pa((void *)&blk_status);
    descs[2].len   = 1;
    descs[2].flags = DESC_F_WRITE;
    descs[2].next  = 0;

    // Post head descriptor to avail ring, then bump avail->idx
    avail->ring[avail->idx % QUEUE_SIZE] = 0;
    __sync_synchronize();
    avail->idx++;
    __sync_synchronize();

    reg_w(REG_QUEUE_NOTIFY, 0);  // kick device: queue 0 has new work

    // Poll until device increments used->idx to match our avail->idx
    while (used->idx != avail->idx)
        __sync_synchronize();

    uint32_t isr = reg_r(REG_INTR_STATUS);
    reg_w(REG_INTR_ACK, isr);

    if (blk_status != 0)
        kpanic("virtio_blk_read: device error %d\n", (int)blk_status);
}
