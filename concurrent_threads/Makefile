CCFLAGS := -pthread

B_TARGET1 := s_lock_trie.c
B_TARGET2 := rw_lock_trie.c
B_TARGET3 := hoh_lock_trie.c

TARGET1 := s_lock
TARGET2 := rw_lock
TARGET3 := hoh_lock

all:
	gcc -o $(TARGET1) $(B_TARGET1) $(CCFLAGS)
	gcc -o $(TARGET2) $(B_TARGET2) $(CCFLAGS)
	gcc -o $(TARGET3) $(B_TARGET3) $(CCFLAGS)
	./s_lock
	./rw_lock
	./hoh_lock
	python3 plot.py

workload:
	python3 workload.py

clean:
	rm $(TARGET1) $(TARGET2) $(TARGET3)