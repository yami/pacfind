CFLAGS = -Wall -Werror -g -D_GNU_SOURCE -std=gnu99 -g
LIBS = -lcurl -larchive -lcdb -lpcre -ljansson
INCS = -I .

C_HEADERS = bindh.h download.h logging.h slist.h update.h conf.h filedb.h query.h ttemp.h util.h
OBJECTS = bindb.o filedb.o util.o pacfind.o slist.o ttemp.o update.o query.o download.o conf.o

pacfind: $(OBJECTS)
	gcc $(LIBS) $(OBJECTS) -o pacfind

%.o: %.c $(C_HEADERS)
	gcc $(CFLAGS) $(INCS) -c $^ -o $@

clean:
	rm -rf pacfind
	rm -rf *.o

install:
	\cp config_template /etc/pacfind.conf
	\cp pacfind /usr/bin/pacfind
	\cp command-not-found.sh /etc/profile.d/
	\mkdir -p /var/pacfind
	\pacfind -u