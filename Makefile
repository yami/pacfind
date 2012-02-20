CFLAGS = -Wall -Werror -g -D_GNU_SOURCE -std=gnu99 -g
LIBS = -lcurl -larchive -lcdb -lpcre
INCS = -I .

pacfind: bindb.o filedb.o util.o pacfind.o slist.o ttemp.o update.o query.o download.o
	gcc $(LIBS) bindb.o filedb.o util.o pacfind.o slist.o ttemp.o query.o update.o download.o -o pacfind

bindb_test: bindb.o filedb.o util.o
	gcc $(LIBS) bindb.o filedb.o util.o -o bindb_test

archive_test: filedb.o
	gcc $(LIBS) filedb.o -o archive_test


download_test: download.o util.o
	gcc $(LIBS) download.o util.o -o download_test

pacfind.o: bindb.h util.h slist.h pacfind.c
	gcc $(CFLAGS) $(INCS) -c pacfind.c -o pacfind.o

query.o: bindb.h query.h query.c util.h
	gcc $(CFLAGS) $(INCS) -c query.c -o query.o

update.o: bindb.h ttemp.h update.h util.h download.h
	gcc $(CFLAGS) $(INCS) -c update.c -o update.o

slist.o: slist.h slist.c util.h
	gcc $(CFLAGS) $(INCS) -c slist.c -o slist.o

bindb.o: bindb.c filedb.h util.h logging.h slist.h
	gcc $(CFLAGS) $(INCS) -c bindb.c -o bindb.o

download.o: download.c util.h download.h
	gcc $(CFLAGS) $(INCS) -c download.c -o download.o

util.o: util.c util.h
	gcc $(CFLAGS) $(INCS) -c util.c -o util.o

filedb.o: filedb.c filedb.h util.h
	gcc $(CFLAGS) $(INCS) -c filedb.c -o filedb.o

ttemp.o: ttemp.h util.h ttemp.c
	gcc $(CFLAGS) $(INCS) -c ttemp.c -o ttemp.o

clean:
	rm -rf *.o
	rm pacfind bindb_test archive_test download_test