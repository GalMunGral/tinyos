#include "virtio_blk.h"
#include "mem.h"
#include "vm.h"
#include "kprintf.h"

// QEMU virt: virtio-mmio-bus.0 at PA 0x10001000
#define VIRTIO_MMIO_BASE  0x10001000UL

// Modern virtio MMIO register offsets (spec 4.2.2, version 2)
#define REG_MAGIC                0x000
#define REG_VERSION              0x004
#define REG_DEVICE_ID            0x008
#define REG_DEVICE_FEATURES      0x010
#define REG_DEVICE_FEATURES_SEL  0x014
#define REG_DRIVER_FEATURES      0x020
#define REG_DRIVER_FEATURES_SEL  0x024
#define REG_QUEUE_SEL            0x030
#define REG_QUEUE_MAX            0x034
#define REG_QUEUE_NUM            0x038
#define REG_QUEUE_READY          0x044
#define REG_QUEUE_NOTIFY         0x050
#define REG_INTR_STATUS          0x060
#define REG_INTR_ACK             0x064
#define REG_STATUS               0x070
#define REG_QUEUE_DESC_LOW       0x080
#define REG_QUEUE_DESC_HIGH      0x084
#define REG_QUEUE_DRIVER_LOW     0x090
#define REG_QUEUE_DRIVER_HIGH    0x094
#define REG_QUEUE_DEVICE_LOW     0x0A0
#define REG_QUEUE_DEVICE_HIGH    0x0A4

#define STAT_ACK          (1 << 0)
#define STAT_DRIVER       (1 << 1)
#define STAT_DRIVER_OK    (1 << 2)
#define STAT_FEATURES_OK  (1 << 3)

#define QUEUE_SIZE  8

#define DESC_F_NEXT   1
#define DESC_F_WRITE  2

struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[QUEUE_SIZE];
};

struct virtq_used_elem { uint32_t id; uint32_t len; };

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[QUEUE_SIZE];
};

static uint8_t virtq_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

static struct virtq_desc          *descs;
static struct virtq_avail         *avail;
static volatile struct virtq_used *used;

static volatile uint32_t *mmio;

static uint32_t reg_r(uint32_t off) { return mmio[off >> 2]; }
static void     reg_w(uint32_t off, uint32_t val) { mmio[off >> 2] = val; }

void virtio_blk_init(void) {
    mmio = (volatile uint32_t *)pa2kva(VIRTIO_MMIO_BASE);

    if (reg_r(REG_MAGIC)     != 0x74726976) kpanic("virtio_blk: bad magic\n");
    if (reg_r(REG_VERSION)   != 2)          kpanic("virtio_blk: need version 2\n");
    if (reg_r(REG_DEVICE_ID) != 2)          kpanic("virtio_blk: not a block device\n");

    reg_w(REG_STATUS, 0);
    reg_w(REG_STATUS, STAT_ACK);
    reg_w(REG_STATUS, STAT_ACK | STAT_DRIVER);

    reg_w(REG_DEVICE_FEATURES_SEL, 0); reg_w(REG_DRIVER_FEATURES_SEL, 0); reg_w(REG_DRIVER_FEATURES, 0);
    reg_w(REG_DEVICE_FEATURES_SEL, 1); reg_w(REG_DRIVER_FEATURES_SEL, 1); reg_w(REG_DRIVER_FEATURES, 0);

    reg_w(REG_STATUS, STAT_ACK | STAT_DRIVER | STAT_FEATURES_OK);
    if (!(reg_r(REG_STATUS) & STAT_FEATURES_OK))
        kpanic("virtio_blk: FEATURES_OK rejected\n");

    reg_w(REG_QUEUE_SEL, 0);
    if (reg_r(REG_QUEUE_MAX) < QUEUE_SIZE)
        kpanic("virtio_blk: queue too small\n");
    reg_w(REG_QUEUE_NUM, QUEUE_SIZE);

    // Layout inside virtq_page: descs | avail | used
    descs = (struct virtq_desc *)virtq_page;
    avail = (struct virtq_avail *)(descs + QUEUE_SIZE);
    used  = (volatile struct virtq_used *)(avail + 1);

    uint64_t desc_pa  = kva2pa(descs);
    uint64_t avail_pa = kva2pa(avail);
    uint64_t used_pa  = kva2pa(avail + 1);

    reg_w(REG_QUEUE_DESC_LOW,    (uint32_t) desc_pa);
    reg_w(REG_QUEUE_DESC_HIGH,   (uint32_t)(desc_pa  >> 32));
    reg_w(REG_QUEUE_DRIVER_LOW,  (uint32_t) avail_pa);
    reg_w(REG_QUEUE_DRIVER_HIGH, (uint32_t)(avail_pa >> 32));
    reg_w(REG_QUEUE_DEVICE_LOW,  (uint32_t) used_pa);
    reg_w(REG_QUEUE_DEVICE_HIGH, (uint32_t)(used_pa  >> 32));
    reg_w(REG_QUEUE_READY, 1);

    reg_w(REG_STATUS, STAT_ACK | STAT_DRIVER | STAT_FEATURES_OK | STAT_DRIVER_OK);
    kprintf("virtio_blk: ok\n");
}

struct virtio_blk_req {
    uint32_t type;
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

    descs[0].addr  = kva2pa(&blk_req);
    descs[0].len   = sizeof(blk_req);
    descs[0].flags = DESC_F_NEXT;
    descs[0].next  = 1;

    descs[1].addr  = kva2pa(buf);
    descs[1].len   = nsectors * SECTOR_SIZE;
    descs[1].flags = DESC_F_WRITE | DESC_F_NEXT;
    descs[1].next  = 2;

    descs[2].addr  = kva2pa((void *)&blk_status);
    descs[2].len   = 1;
    descs[2].flags = DESC_F_WRITE;
    descs[2].next  = 0;

    avail->ring[avail->idx % QUEUE_SIZE] = 0;
    __sync_synchronize();
    avail->idx++;
    __sync_synchronize();

    reg_w(REG_QUEUE_NOTIFY, 0);

    while (used->idx != avail->idx)
        __sync_synchronize();

    uint32_t isr = reg_r(REG_INTR_STATUS);
    reg_w(REG_INTR_ACK, isr);

    if (blk_status != 0)
        kpanic("virtio_blk_read: device error %d\n", (int)blk_status);
}
