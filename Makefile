CC = gcc
TARGETS = ssu_cleanup tree arrange

all: $(TARGETS)

ssu_cleanup: src/ssu_cleanup.c
	$(CC) -o $@ $^ -g

tree: src/tree.c
	$(CC) -o $@ $^ -g

arrange: src/arrange.c
	$(CC) -o $@ $^ -g

clean:
	rm -f $(TARGETS)
