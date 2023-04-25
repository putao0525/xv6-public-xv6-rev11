struct buf {
    int flags;//用于表示缓冲区的状态或属性
    uint dev;//表示设备号，用于标识该缓冲区所在的设备
    uint blockno;//用于标识该缓冲区所在的磁盘块
    struct sleeplock lock;
    uint refcnt;//表示引用计数，用于记录该缓冲区被使用的次数。
    struct buf *prev; // LRU cache list
    struct buf *next;
    struct buf *qnext; // disk queue
    uchar data[BSIZE];
};
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

