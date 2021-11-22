// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

int g_nCoreID_HasMem = -1;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++)
  {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  push_off();
  g_nCoreID_HasMem = cpuid();
  pop_off();

  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    void* pa = p;

    struct run *r;

    if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
      panic("kfree");

    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;

    acquire(&kmem[g_nCoreID_HasMem].lock);
    r->next = kmem[g_nCoreID_HasMem].freelist;
    kmem[g_nCoreID_HasMem].freelist = r;
    release(&kmem[g_nCoreID_HasMem].lock);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  push_off();
  int nCoreNumber = cpuid();
  pop_off();

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[nCoreNumber].lock);
  r->next = kmem[nCoreNumber].freelist;
  kmem[nCoreNumber].freelist = r;
  release(&kmem[nCoreNumber].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cpu = cpuid();
  pop_off();

  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if(r)
    kmem[cpu].freelist = r->next;
  else // steal page from other CPU
  {
    struct run* tmp;
    for (int i = 0; i < NCPU; ++i)
    {
      if (i == cpu) continue;
      acquire(&kmem[i].lock);
      tmp = kmem[i].freelist;
      if (tmp == 0) {
        release(&kmem[i].lock);
        continue;
      } else {
        for (int j = 0; j < 1024; j++) {
          // steal 1024 pages
          if (tmp->next)
            tmp = tmp->next;
          else 
            break;
        }
        kmem[cpu].freelist = kmem[i].freelist;
        kmem[i].freelist = tmp->next;
        tmp->next = 0;
        release(&kmem[i].lock);
        break;
      }
    }
    r = kmem[cpu].freelist;
    if (r) 
      kmem[cpu].freelist = r->next;
  }
  release(&kmem[cpu].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
