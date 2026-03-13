/*************************************************************************
    > File Name: cxl-coherence-demo.c
  > Author:
  > Mail:
  > Created Time: 2026年03月13日 星期五 17时31分37秒
 ************************************************************************/

#include <pthread.h>
#include <numa.h>
#include <stdio.h>
#include <stdatomic.h>
#include <emmintrin.h>

long loop_count = 10;  // 默认循环10次,压力测试建议10000000次以上
int numa_node = 0;  // 默认使用NUMA节点0

atomic_long *p1;
atomic_long *p2;

void *worker(void *arg)
{
	for (long i = 0; i < loop_count; i++)
		atomic_fetch_add(p1, 1);

	for (long i = 0; i < loop_count; i++)
		atomic_fetch_add(p2, 1);

	return NULL;
}

int main(int argc, char *argv[])
{
	// 解析命令行参数
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--loop") == 0 && i + 1 < argc) {
			loop_count = atol(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--numa") == 0 && i + 1 < argc) {
			numa_node = atoi(argv[i + 1]);
			i++;
		} else {
			printf("Usage: %s [--loop N] [--numa NODE]\n", argv[0]);
			return 1;
		}
	}

	p1 = numa_alloc_onnode(sizeof(atomic_long), numa_node);
	p2 = numa_alloc_onnode(sizeof(atomic_long), numa_node);

	atomic_init(p1, 0);
	atomic_init(p2, 0);

	pthread_t t1, t2, t3;

	pthread_create(&t1, NULL, worker, NULL);
	pthread_create(&t2, NULL, worker, NULL);
	pthread_create(&t3, NULL, worker, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	printf("p1 add: %ld\n", atomic_load(p1));
	printf("p2 add: %ld\n", atomic_load(p1));

	printf("flush cache line to mem\n");
	_mm_clflush(p1);  // flush p 对应的 cache line 到内存
	_mm_sfence();    // 确保 flush 完成
	_mm_clflush(p2);  // flush p 对应的 cache line 到内存
	_mm_sfence();    // 确保 flush 完成
	printf("p1 add: %ld\n", atomic_load(p1));
	printf("p2 add: %ld\n", atomic_load(p1));

	if (atomic_load(p1) != 3 * loop_count || atomic_load(p2) != 3 * loop_count) {
		printf("Data inconsistency detected! p1: %ld, p2: %ld\n", atomic_load(p1), atomic_load(p2));
	} else {
		printf("Data is consistent. p1: %ld, p2: %ld\n", atomic_load(p1), atomic_load(p2));
	}

	// 三个线程，每个线程对 p1 和 p2 分别加LOOP_COUNT次，所以最终结果应该是 3 * loop_count
	// 以qemu cxl为例，由于qemu并不模拟cpu的cache子系统
	// 这里实验时若numa node设定为cxl node
	// 当loop count次数较少时如1, 2, 3, 表现正常，否则会出现不等于3*loop_count的情况
	// 当实验时numa node设定为mem node时，无论loop count次数多少，表现正常
}

