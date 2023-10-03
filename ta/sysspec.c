
/*
** sysspec.c
** System-specific routines.
**
** BYTEmark (tm)
** BYTE's Native Mode Benchmarks
** Rick Grehan, BYTE Magazine
**
** Creation:
** Revision: 3/95;10/95
**
** DISCLAIMER
** The source, executable, and documentation files that comprise
** the BYTEmark benchmarks are made available on an "as is" basis.
** This means that we at BYTE Magazine have made every reasonable
** effort to verify that the there are no errors in the source and
** executable code.  We cannot, however, guarantee that the programs
** are error-free.  Consequently, McGraw-HIll and BYTE Magazine make
** no claims in regard to the fitness of the source code, executable
** code, and documentation of the BYTEmark.
**  Furthermore, BYTE Magazine, McGraw-Hill, and all employees
** of McGraw-Hill cannot be held responsible for any damages resulting
** from the use of this code or the results obtained from using
** this code.
*/

/***********************************
**    SYSTEM-SPECIFIC ROUTINES    **
************************************
**
** These are the routines that provide functions that are
** system-specific.  If the benchmarks are to be ported
** to new hardware/new O.S., this is the first place to
** start.
*/
#include "sysspec.h"
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#ifdef DOS16
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#endif
/*********************************
**  MEMORY MANAGEMENT ROUTINES  **
*********************************/


/****************************
** AllocateMemory
** This routine returns a void pointer to a memory
** block.  The size of the memory block is given in bytes
** as the first argument.  This routine also returns an
** error code in the second argument.
** 10/95 Update:
**  Added an associative array for memory alignment reasons.
**  mem_array[2][MEM_ARRAY_SIZE]
**   mem_array[0][n] = Actual address (from malloc)
**   mem_array[1][n] = Aligned address
** Currently, mem_array[][] is only used if you use malloc;
**  it is not used for the 16-bit DOS and MAC versions.
*/
farvoid *AllocateMemory(unsigned long nbytes,   /* # of bytes to alloc */
		int *errorcode)                 /* Returned error code */
{
#ifdef MALLOCMEM
/*
** Everyone else, its pretty straightforward, given
** that you use a 32-bit compiler which treats size_t as
** a 4-byte entity.
*/
farvoid *returnval;             /* Return value */
ulong true_addr;		/* True address */
ulong adj_addr;			/* Adjusted address */

returnval=(farvoid *)TEE_Malloc((size_t)(nbytes+2L*(long)global_align), 0);
if(returnval==(farvoid *)NULL)
	*errorcode=ERROR_MEMORY;
else
	*errorcode=0;

/*
** Check for alignment
*/
adj_addr=true_addr=(ulong)returnval;
if(global_align==0)
{	
	if(AddMemArray(true_addr, adj_addr))
		*errorcode=ERROR_MEMARRAY_FULL;
	return(returnval);
}

if(global_align==1)
{	
        if(true_addr%2==0) adj_addr++;
}
else
{	
	while(adj_addr%global_align!=0) ++adj_addr;
	if(adj_addr%(global_align*2)==0) adj_addr+=global_align;
}
returnval=(void *)adj_addr;
if(AddMemArray(true_addr,adj_addr))
	*errorcode=ERROR_MEMARRAY_FULL;
return(returnval);
#endif

}


/****************************
** FreeMemory
** This is the reverse of AllocateMemory.  The memory
** block passed in is freed.  Should an error occur,
** that error is returned in errorcode.
*/
void FreeMemory(farvoid *mempointer,    /* Pointer to memory block */
		int *errorcode)
{
#ifdef MALLOCMEM
ulong adj_addr, true_addr;

/* Locate item in memory array */
adj_addr=(ulong)mempointer;
if(RemoveMemArray(adj_addr, &true_addr))
{	*errorcode=ERROR_MEMARRAY_NFOUND;
	return;
}
mempointer=(void *)true_addr;
TEE_Free(mempointer);
*errorcode=0;
return;
#endif
}

/****************************
** MoveMemory
** Moves n bytes from a to b.  Handles overlap.
** In most cases, this is just a memmove operation.
** But, not in DOS....noooo....
*/
void MoveMemory( farvoid *destination,  /* Destination address */
		farvoid *source,        /* Source address */
		unsigned long nbytes)
{

/* +++16-bit DOS VERSION+++ */
#ifdef DOS16MEM

	FarDOSmemmove( destination, source, nbytes);

#else

memmove(destination, source, nbytes);

#endif
}

/***********************************
** MEMORY ARRAY HANDLING ROUTINES **
***********************************/
/****************************
** InitMemArray
** Initialize the memory array.  This simply amounts to
** setting mem_array_ents to zero, indicating that there
** isn't anything in the memory array.
*/
void InitMemArray(void)
{
mem_array_ents=0;
return;
}

/***************************
** AddMemArray
** Add a pair of items to the memory array.
**  true_addr is the true address (mem_array[0][n])
**  adj_addr is the adjusted address (mem_array[0][n])
** Returns 0 if ok
** -1 if not enough room
*/
int AddMemArray(ulong true_addr,
		ulong adj_addr)
{
if(mem_array_ents>=MEM_ARRAY_SIZE)
	return(-1);

mem_array[0][mem_array_ents]=true_addr;
mem_array[1][mem_array_ents]=adj_addr;
mem_array_ents++;
return(0);
}

/*************************
** RemoveMemArray
** Given an adjusted address value (mem_array[1][n]), locate
** the entry and remove it from the mem_array.
** Also returns the associated true address.
** Returns 0 if ok
** -1 if not found.
*/
int RemoveMemArray(ulong adj_addr,ulong *true_addr)
{
int i,j;

/* Locate the item in the array. */
for(i=0;i<mem_array_ents;i++)
	if(mem_array[1][i]==adj_addr)
	{       /* Found it..bubble stuff down */
		*true_addr=mem_array[0][i];
		j=i;
		while(j+1<mem_array_ents)
		{      
			mem_array[0][j]=mem_array[0][j+1];
			mem_array[1][j]=mem_array[1][j+1];
			j++;
		}
		mem_array_ents--;
		return(0);      /* Return if found */
	}

/* If we made it here...something's wrong...show error */
return(-1);
}


/********************************
**   ERROR HANDLING ROUTINES   **
********************************/

/****************************
** ReportError
** Report error message condition.
*/
void ReportError(char *errorcontext,    /* Error context string */
		int errorcode)          /* Error code number */
{

/*
** Display error context
*/
printf("ERROR CONDITION\nContext: %s\n",errorcontext);

/*
** Display code
*/
printf("Code: %d",errorcode);

return;
}

/****************************
** ErrorExit
** Peforms an exit from an error condition.
*/
void ErrorExit()
{

/*
** For profiling on the Mac with MetroWerks -- 11/17/94 RG
** Have to do this to turn off profiler.
*/
#ifdef MACCWPROF
#if __profile__
ProfilerTerm();
#endif
#endif

/*
** FOR NOW...SIMPLE EXIT
*/
#ifdef JUNO
exit(1);
#endif
}

/*****************************
**    STOPWATCH ROUTINES    **
*****************************/
static inline uint32_t tee_time_to_ticks()
{
	TEE_Time t = { };
	TEE_GetSystemTime(&t);
	return (t.seconds * 1000 + t.millis) * 1000;
}

#ifdef ARMv8PMU

static inline unsigned long arch_counter_get_cntpct(void)
{
    unsigned long cval = 0;
	
    isb();
    asm volatile("mrs %0, PMCCNTR_EL0" : "+r"(cval));
    return cval;
}

#endif


/****************************
** StartStopwatch
** Starts a software stopwatch.  Returns the first value of
** the stopwatch in ticks.
*/
unsigned long StartStopwatch()
{
#ifdef MACTIMEMGR
/*
** For Mac code warrior, use timer. In this case, what we return is really
** a dummy value.
*/
InsTime((QElemPtr)&myTMTask);
PrimeTime((QElemPtr)&myTMTask,-MacHSTdelay);
return((unsigned long)1);
#else
#ifdef WIN31TIMER
/*
** Win 3.x timer returns a DWORD, which we coax into a long.
*/
_Call16(lpfn,"p",&win31tinfo);
return((unsigned long)win31tinfo.dwmsSinceStart);
#else
return((unsigned long)tee_time_to_ticks());
#endif
#endif
}

/****************************
** StopStopwatch
** Stops the software stopwatch.  Expects as an input argument
** the stopwatch start time.
*/
unsigned long StopStopwatch(unsigned long startticks)
{
	
#ifdef MACTIMEMGR
/*
** For Mac code warrior...ignore startticks.  Return val. in microseconds
*/
RmvTime((QElemPtr)&myTMTask);
return((unsigned long)(MacHSTdelay+myTMTask.tmCount-MacHSTohead));
#else
#ifdef WIN31TIMER
_Call16(lpfn,"p",&win31tinfo);
return((unsigned long)win31tinfo.dwmsSinceStart-startticks);
#else
return((unsigned long)tee_time_to_ticks()-startticks);
#endif
#endif
}

/****************************
** TicksToSecs
** Converts ticks to seconds.  Converts ticks to integer
** seconds, discarding any fractional amount.
*/
unsigned long TicksToSecs(unsigned long tickamount)
{
#ifdef CLOCKWCT
return((unsigned long)(tickamount/CLK_TCK));
#endif

#ifdef MACTIMEMGR
/* +++ MAC time manager version (using timer in microseconds) +++ */
return((unsigned long)(tickamount/1000000));
#endif

#ifdef CLOCKWCPS
/* Everybody else */
return((unsigned long)(tickamount/CLOCKS_PER_SEC));
#endif

#ifdef WIN31TIMER
/* Each tick is 840 nanoseconds */
return((unsigned long)(tickamount/1000L));
#endif

}

/****************************
** TicksToFracSecs
** Converts ticks to fractional seconds.  In other words,
** this returns the exact conversion from ticks to
** seconds.
*/
double TicksToFracSecs(unsigned long tickamount)
{
#ifdef CLOCKWCT
return((double)tickamount/(double)CLK_TCK);
#endif

#ifdef MACTIMEMGR
/* +++ MAC time manager version +++ */
return((double)tickamount/(double)1000000);
#endif

#ifdef CLOCKWCPS
/* Everybody else */
return((double)tickamount/(double)CLOCKS_PER_SEC);
#endif

#ifdef WIN31TIMER
/* Using 840 nanosecond ticks */
return((double)tickamount/(double)1000);
#endif
}

