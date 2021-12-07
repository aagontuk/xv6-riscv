#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
  int n;
  int block[LOGSIZE_INDIV];
};

struct log {
  int start;
  int size;
  int outstanding; // how many FS sys calls are executing.
  int committing;  // in commit(), please wait.
  int dev;
  struct logheader lh;
};

struct log logs[NLOGS];

int curlog = 0;  // current log we are using
int curcommit = 0;  // log we are commiting next

struct spinlock lock;

static void recover_from_log(void);
static void commit();

void
initlog(int dev, struct superblock *sb)
{
  if (sizeof(struct logheader) >= BSIZE)
    panic("initlog: too big logheader");
    
  initlock(&lock, "log");

  for (int i = 0; i < NLOGS; i++) {
    logs[i].start = sb->logstart + (i * LOGSIZE_INDIV);
    logs[i].size = sb->nlog / NLOGS;
    logs[i].dev = dev;
  }

  recover_from_log();
}

// Copy committed blocks from log to their home location
static void
install_trans(int recovering)
{
  int tail;
  struct log *lg = &logs[curcommit];

  for (tail = 0; tail < lg->lh.n; tail++) {
    struct buf *lbuf = bread(lg->dev, lg->start+tail+1); // read log block
    struct buf *dbuf = bread(lg->dev, lg->lh.block[tail]); // read dst
    memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
    bwrite(dbuf);  // write dst to disk
    if(recovering == 0)
      bunpin(dbuf);
    brelse(lbuf);
    brelse(dbuf);
  }
}

// Read the log header from disk into the in-memory log header
static void
read_head(void)
{
  struct log *lg = &logs[curcommit];

  struct buf *buf = bread(lg->dev, lg->start);
  struct logheader *lh = (struct logheader *) (buf->data);
  int i;
  lg->lh.n = lh->n;
  for (i = 0; i < lg->lh.n; i++) {
    lg->lh.block[i] = lh->block[i];
  }
  brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
static void
write_head(void)
{
  struct log *lg = &logs[curcommit];
  struct buf *buf = bread(lg->dev, lg->start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = lg->lh.n;
  for (i = 0; i < lg->lh.n; i++) {
    hb->block[i] = lg->lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

static void
recover_from_log(void)
{
  for (curcommit = 0; curcommit < NLOGS; curcommit++){
    read_head();
    install_trans(1); // if committed, copy from log to disk
    logs[curcommit].lh.n = 0;
    write_head(); // clear the log
  }

  curcommit = 0; // All commits done! reset.
}

// called at the start of each FS system call.
void
begin_op(void)
{
  acquire(&lock);
  while(1){
    int sleeping = 1;
    if(logs[curlog].committing){
      // Work with new log
      // sleep if there is none
      if(curlog < NLOGS) {
        curlog++;  
        sleeping = 0;
      }
      else {
        sleep(&logs, &lock);
      }

    } else if(logs[curlog].lh.n + 
        (logs[curlog].outstanding+1)*MAXOPBLOCKS > LOGSIZE_INDIV){
      
      // this op might exhaust log space; wait for commit.
      // work with new log
      // sleep if there is none
      if(curlog < NLOGS) {
        curlog++;  
        sleeping = 0;
      }
      else {
        sleep(&logs, &lock);
      }

    }

    if(!sleeping) {
      logs[curlog].outstanding += 1;
      release(&lock);
      break;
    }
  }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
//
// Protocol:
//  * Decrement first log
//  * If it reaches zero commit
//  * Decrement 2nd log, if zero commit
//  * Same for other logs
//  * Only one decrement per end_op
//  * if it is the last log set curlog back to zero
void
end_op(void)
{
  int do_commit = 0;

  acquire(&lock);
  logs[curcommit].outstanding -= 1;
  if(logs[curcommit].committing)
    panic("log.committing");
  if(logs[curcommit].outstanding == 0){
    do_commit = 1;
    logs[curcommit].committing = 1;
  } else {
    // FIXME: Think about modification for four log
    // begin_op() may be waiting for log space,
    // and decrementing log.outstanding has decreased
    // the amount of reserved space.
    wakeup(&logs);
  }
  release(&lock);

  if(do_commit){
    // call commit w/o holding locks, since not allowed
    // to sleep with locks.
    commit();
    acquire(&lock);
    logs[curcommit].committing = 0;
    curcommit++;

    // All commit done! Reset.
    if (curcommit == NLOGS) {
      curcommit = 0; 
      curlog = 0;
    }

    wakeup(&logs);
    release(&lock);
  }
}

// Copy modified blocks from cache to log.
// 
// For all the modified blocks
//  1. Copy modified block to in-memory log block
//  2. Write the log block to disk.
static void
write_log(void)
{
  int tail;
  struct log *lg = &logs[curcommit];

  for (tail = 0; tail < lg->lh.n; tail++) {
    struct buf *to = bread(lg->dev, lg->start+tail+1); // log block
    struct buf *from = bread(lg->dev, lg->lh.block[tail]); // cache block
    memmove(to->data, from->data, BSIZE);
    bwrite(to);  // write the log
    brelse(from);
    brelse(to);
  }
}

// Commit a log
//  1. Write modified blocks to log in-mem blocks
//     and in-mem log blocks to disk log blocks.
//  2. Write current state of log header to disk
//  3. Copy log blocks to their destination blocks
//  4. Reset in-mem log header to zero.
//  5. Reset disk log header to zero by writing
//     log header to disk.

static void
commit()
{
  if (logs[curcommit].lh.n > 0) {
    write_log();     // Modified blocks from cache -> log -> home disk blocks
    write_head();    // Write header to disk -- the real commit
    install_trans(0); // Now install writes to home locations
    logs[curcommit].lh.n = 0;
    write_head();    // Erase the transaction from the log
  }
}

// Add new block to in memory log header if it's not there already
//
// Caller has modified b->data and is done with the buffer.
// Record the block number and pin in the cache by increasing refcnt.
// commit()/write_log() will do the disk write.
//
// log_write() replaces bwrite(); a typical use is:
//   bp = bread(...)
//   modify bp->data[]
//   log_write(bp)
//   brelse(bp)
void
log_write(struct buf *b)
{
  int i;
  struct log *lg = &logs[curlog];

  acquire(&lock);
  if (lg->lh.n >= LOGSIZE_INDIV || lg->lh.n >= (lg->size / 4) - 1)
    panic("too big a transaction");
  if (lg->outstanding < 1)
    panic("log_write outside of trans");

  for (i = 0; i < lg->lh.n; i++) {
    if (lg->lh.block[i] == b->blockno)   // log absorbtion
      break;
  }
  lg->lh.block[i] = b->blockno;
  if (i == lg->lh.n) {  // Add new block to log?
    bpin(b);
    lg->lh.n++;
  }
  release(&lock);
}

