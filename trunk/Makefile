CC = gcc
TARGET = MilyFX
RM = rm -rf
SOURCE = milyfx.c mygetopt.c utf8.c
OBJS = milyfx.o mygetopt.o utf8.o
INCLUDE = -I./libfetion
LIBFETION = ./lib/libfetion_32.a
SHARED_LIBCURL = -lcurl -lgssapi_krb5
STATIC_LIBCURL = /usr/lib/libcurl.a -lgssapi_krb5 -lidn -lssl -lcrypto -lldap \
	-L/usr/lib -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lssl -lcrypto -lz

%.o : %.c
	$(CC) -c $(INCLUDE) $*.c -o $*.o
all:$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBFETION) $(SHARED_LIBCURL) -lpthread -lstdc++
	$(RM) $(OBJS)
static:$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBFETION) $(STATIC_LIBCURL) -lpthread -lstdc++
	$(RM) $(OBJS)
clean:
	$(RM) $(TARGET) $(OBJS)
