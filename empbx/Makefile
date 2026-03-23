PREFIX ?= /opt/empbx

UNAME=$(shell uname -s)
BIN_OUT=empbxd

#OPENSSL_INC=-I../openssl/include
#OPENSSL_LIB=../openssl/libssl.a ../openssl/libcrypto.a
OPENSSL_LIB=-lssl -lcrypto -lz

PCRE_INC=-I../libs/pcre
PCRE_LIB=../libs/pcre/.libs/libpcre.a

LIBRE_INC=-I../libs/re/dist/include
LIBRE_LIB=../libs/re/dist/lib/libre.a

BARESIP_INC=-I../libs/baresip/dist/include
BARESIP_LIB=../libs/baresip/dist/lib/libbaresip.a


CFLAGS=-g -O2 -DEMPBX_DEBUG
CFLAGS+=-I./src -I./include ${LIBRE_INC} ${BARESIP_INC} ${PCRE_INC} ${OPENSSL_INC}
LIBS+=${PCRE_LIB}
LIBS+=${BARESIP_LIB}
LIBS+=${LIBRE_LIB}
LIBS+=-lc -lm -lpthread ${OPENSSL_LIB}

BIN_SRC=src/empbx-main.c \
	src/empbx-config.c src/empbx-log.c src/empbx-utils-re.c src/empbx-utils-pid.c src/empbx-utils-common.c src/empbx-utils-file.c src/empbx-utils-thread.c src/empbx-utils-list.c src/empbx-utils-htable.c src/empbx-utils-sip-reply.c src/empbx-utils-time.c \
	src/empbx-utils-queue.c src/empbx-utils-regex.c src/empbx-utils-str.c \
	src/empbx-service-sip.c src/empbx-service-sip-pvt.c src/empbx-service-sip-pvt-gateways.c src/empbx-service-sip-pvt-users.c src/empbx-service-sip-pvt-sessions.c src/empbx-service-sip-pvt-dialplan.c src/empbx-service-sip-pvt-auth.c src/empbx-service-sip-pvt-apps.c \
	src/empbx-app-bridge.c \
	src/empbx-service-mgmt.c \
	src/bs-mod-g729.c \
	src/ezxml.c src/cJSON.c src/cJSON_Utils.c

BIN_OBJ=$(BIN_SRC:.c=.o)

ifeq ($(UNAME),FreeBSD)
    CXX=cc
    CC=cc
    LD=cc
    STRIP=strip
    LIBS+=-lexecinfo
else
    CXX=g++
    CC=gcc
    LD=gcc
    STRIP=strip
endif


all: $(BIN_SRC) $(BIN_OUT)

$(BIN_OUT): $(BIN_OBJ)
	$(LD) -o $@ $(BIN_OBJ) $(LIBS) 
#	$(STRIP) $(BIN_OUT)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

install:
	mkdir -p $(PREFIX)/bin/
	mkdir -p $(PREFIX)/configs/
	mkdir -p $(PREFIX)/sounds/
	cp $(BIN_OUT) $(PREFIX)/bin/
	cp misc/empbxd-conf.xml $(PREFIX)/configs/

clean:
	@rm -f src/*.o
	@rm -f $(BIN_OUT)

clean-all:
	@rm -f src/*.o
	@rm -f $(BIN_OUT)
