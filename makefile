rx320: rx320.c consts.h stations.c stations.h utils.c utils.h com.c com.h
	cc rx320.c com.c utils.c stations.c -o rx320 -lm
clean:
	rm channels.db rx320

