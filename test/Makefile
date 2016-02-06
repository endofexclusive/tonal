CINCLUDE=-I../include -Ivtest/include -I..
CFLAGS := -O2 -std=c99 -Wall -Wextra -pedantic $(CINCLUDE)

test_tonal: tonal.o vtest.o test_tonal.c

tonal.o: ../tonal.c ../tonal_priv.h ../include/tonal.h
	$(CC) $(CFLAGS) -c ../tonal.c -o $@

vtest.o: vtest/vtest.c vtest/include/vtest.h
	$(CC) $(CFLAGS) -c vtest/vtest.c -o $@

.PHONY: clean
clean:
	rm -f tonal.o vtest.o test_tonal

