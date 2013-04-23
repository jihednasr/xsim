#ifndef _PERFORMANCE_EVALUATION_H_
#define _PERFORMANCE_EVALUATION_H_

//#define _POSIX_C_SOURCE 199309L

#include <xsim_perf_eval.h>


extern xsim_specific_perf_counter_t xsim_specific_counter[number_of_specific_area];
extern xsim_global_perf_counter_t   global_counter[number_of_global_area];
extern int                          msg_type_counter[xsim_need_time_info];



void			xsim_perf_init();
void			xsim_perf_fini();

#if defined(PERFORMANCE_EVALUATION) && defined(BENCHMARK)
void xsim_perf_benchmark(xsim_node_t *node);
#else
void xsim_perf_benchmark(xsim_node_t *node __attribute__((__unused__)));
#endif

void xsim_compute_the_results_of_the_measures();


static inline void Serialize () {
   __asm__ __volatile__ ( "xorl %%eax, %%eax \n cpuid " : : : "%eax","%ebx","%ecx","%edx" );
}

static inline uint64_t Readtsc() {
	uint32_t hb = 0;
	uint32_t lb = 0;
	__asm__ __volatile__ ( "rdtsc\n\t" 
			"movl %%eax, %0\n\t" 
			"movl %%edx, %1\n\t"
			: "=r"(lb), "=r" (hb) : : "%edx", "%eax");    

	return (uint64_t)( ((uint64_t)hb << 32) | lb );
}

static inline uint64_t Readtscp(uint32_t *cpu) {
	uint32_t hb = 0;
	uint32_t lb = 0;
	__asm__ __volatile__ ( "rdtscp\n\t" 
			"movl %%eax, %0\n\t" 
			"movl %%edx, %1\n\t"
            "movl %%ecx, %2\n\t"
			: "=r"(lb), "=r" (hb), "=r" (*cpu) : : "%edx", "%eax", "%ecx");    

	return (uint64_t)( ((uint64_t)hb << 32) | lb );
}


static inline void xsim_perf_begin_measure(xsim_specific_area_to_measure_t area)
{
#ifdef OUTPUT_ALL_MEASURES
	/* alloc a new cellule for the next measure */ 
    xsim_specific_measure_t *tmp_measure  = malloc(sizeof(xsim_specific_measure_t)); 
    memset(tmp_measure, 0, sizeof(xsim_specific_measure_t));
	tmp_measure->next                     = xsim_specific_counter[area].list_measure; 
	xsim_specific_counter[area].list_measure   = tmp_measure; 
#endif

	xsim_specific_counter[area].nb_of_passage++; 
#ifndef OUTPUT_ALL_MEASURES
    if (xsim_specific_counter[area].length < 0) {
        fprintf(stderr, "End measure %lld not done in area %d\n", 
                (unsigned long long)(xsim_specific_counter[area].nb_of_passage), area);
    }
#endif
    
	Serialize(); 
    
#ifdef OUTPUT_ALL_MEASURES

#ifdef RDTSCP
	xsim_specific_counter[area].list_measure->begin = Readtscp(&(xsim_specific_counter[area].list_measure->cpu));
#else
#ifdef AFFINITY
    xsim_specific_counter[area].list_measure->cpu   = sched_getcpu(); 
#endif
	xsim_specific_counter[area].list_measure->begin = Readtsc();
#endif

#else 

#ifdef RDTSCP
	xsim_specific_counter[area].length = -Readtscp(&(xsim_specific_counter[area].cpu));
#else
#ifdef AFFINITY
	xsim_specific_counter[area].cpu   = sched_getcpu(); 
#endif
	xsim_specific_counter[area].length = -Readtsc();
#endif

#endif
}


static inline void xsim_perf_end_measure(xsim_specific_area_to_measure_t area) 
{
	Serialize(); 
#ifdef OUTPUT_ALL_MEASURES

#ifdef RDTSCP
    xsim_specific_counter[area].list_measure->end    = Readtscp(&(xsim_specific_counter[area].list_measure->cpu_end)); 
#else
    if (xsim_specific_counter[area].end != 0) {
        fprintf(stderr, "End measure %"PRIu64" already done in area %d\n", 
                xsim_specific_counter[area].nb_of_passage, area);
    }
    xsim_specific_counter[area].list_measure->end     = Readtsc(); 
#ifdef AFFINITY
	xsim_specific_counter[area].list_measure->cpu_end = sched_getcpu();
#endif
#endif

#else

#ifdef RDTSCP
	xsim_specific_counter[area].length += Readtscp(&(xsim_specific_counter[area].cpu_end)); 
#else
    xsim_specific_counter[area].length += Readtsc(); 
#ifdef AFFINITY
	xsim_specific_counter[area].cpu_end = sched_getcpu();
#endif
#endif
    if (xsim_specific_counter[area].length > 0) {
        xsim_specific_counter[area].sum += xsim_specific_counter[area].length;
    } else {
        fprintf(stderr, "Error: get a negative measure.\n");
    }

#endif
}



#ifdef PERFORMANCE_EVALUATION 
static inline void	xsim_perf_begin_global_measure(xsim_global_area_to_measure_t area, clockid_t clk)
{
	/* alloc a new cellule for the next measure */
	xsim_global_measure_t* tmp = (xsim_global_measure_t*) malloc(sizeof(xsim_global_measure_t));
	tmp->next                  = global_counter[area].list_measure;
	global_counter[area].list_measure   = tmp;
	global_counter[area].nb_of_passage++;

#ifdef AFFINITY
	global_counter[area].list_measure->cpu   = sched_getcpu(); 
#endif
	clock_gettime(clk, &(global_counter[area].list_measure->begin));
}

static inline void 	xsim_perf_end_global_measure(xsim_global_area_to_measure_t area, clockid_t clk)
{
	clock_gettime(clk, &(global_counter[area].list_measure->end));
#ifdef AFFINITY
	if (global_counter[area].list_measure->cpu != sched_getcpu()) {
		fprintf(stdout, "*************************The CPU has changed during the measure in area %d\n", (int)area);
	}
#endif
}



#else

    
    
static inline void	xsim_perf_begin_global_measure(
        xsim_global_area_to_measure_t area __attribute__((__unused__)), 
        clockid_t clk __attribute__((__unused__)))
{
}

static inline void 	xsim_perf_end_global_measure(
        xsim_global_area_to_measure_t area __attribute__((__unused__)), 
        clockid_t clk __attribute__((__unused__)))
{
}
#endif



#endif
