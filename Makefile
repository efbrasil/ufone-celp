# Main makefile

CC			= gcc
LD			= $(CC)
AR			= ar

CFLAGS			= -Wall -ggdb
LDFLAGS			= -Wall -lm
ARFLAGS			= rc

ALL_SRCS		= Makefile

# Alsa
UFONE_ASERVER_ADAPT_OBJS= ufone_aserver_adapt.o ufone_sound.o libcelp.a wave.o \
                          ufone_fifo.o utils.o
UFONE_ASERVER_ADAPT_BINS= ufone_aserver_adapt
UFONE_ASERVER_OBJS	= ufone_aserver.o ufone_sound.o libcelp.a wave.o utils.o
UFONE_ASERVER_BINS	= ufone_aserver
UFONE_ACLIENT_OBJS	= ufone_aclient.o ufone_sound.o libcelp.a wave.o utils.o
UFONE_ACLIENT_BINS	= ufone_aclient

UFONE_SERVER_OBJS	= ufone_server.o libcelp.a wave.o utils.o
UFONE_SERVER_BINS	= ufone_server
UFONE_CLIENT_OBJS	= ufone_client.o libcelp.a wave.o utils.o
UFONE_CLIENT_BINS	= ufone_client
UFONE_PHONE_OBJS	= ufone_phone.o libcelp.a wave.o utils.o
UFONE_PHONE_BINS	= ufone_phone


LIBCELP_OBJS		= lpc.o filter.o codebook.o ccoder.o cdecoder.o pack.o
CELP_OBJS		= celp.o utils.o wave.o libcelp.a
TEST_ECHO_OBJS		= test_echo.o libcelp.a
TEST_GAIN_OBJS		= test_gain.o libcelp.a
TEST_LSF_OBJS		= test_lsf.o libcelp.a
TEST_ENERGY_OBJS	= test_energy.o libcelp.a
TEST_TORRES_OBJS	= test_torres.o libcelp.a
TEST_DETORRES_OBJS	= test_detorres.o libcelp.a
TEST_CCODER_OBJS	= test_ccoder.o libcelp.a
TEST_CDECODER_OBJS	= test_cdecoder.o libcelp.a

LIBCELP_BINS		= libcelp.so libcelp.a
CELP_BINS		= celp
TEST_ENERGY_BINS	= test_energy
TEST_GAIN_BINS		= test_gain
TEST_LSF_BINS		= test_lsf
TEST_ECHO_BINS		= test_echo
TEST_TORRES_BINS	= test_torres
TEST_DETORRES_BINS	= test_detorres
TEST_CCODER_BINS	= test_ccoder
TEST_CDECODER_BINS	= test_cdecoder

ALL_OBJS		= $(TEST_LPC_OBJS) $(TEST_CCODER_OBJS)       \
                          $(TEST_CDECODER_OBJS) $(TEST_ECHO_OBJS)    \
			  $(TEST_TORRES_OBJS) $(TEST_DETORRES_OBJS)  \
			  $(LIBCELP_OBJS) $(CELP_OBJS)               \
			  $(TEST_ENERGY_OBJS) $(TEST_LSF_OBJS)       \
			  $(TEST_GAIN_OBJS) $(UFONE_SERVER_OBJS)     \
			  $(UFONE_CLIENT_OBJS) $(UFONE_PHONE_OBJS)   \
			  $(UFONE_ACLIENT_OBJS) $(UFONE_ASERVER_OBJS)\
                          $(UFONE_ASERVER_ADAPT_OBJS)

ALL_BINS		= $(TEST_LPC_BINS) $(TEST_CCODER_BINS)       \
                          $(TEST_CDECODER_BINS) $(TEST_ECHO_BINS)    \
			  $(TEST_TORRES_BINS) $(TEST_DETORRES_BINS)  \
			  $(LIBCELP_BINS) $(CELP_BINS)               \
			  $(TEST_ENERGY_BINS) $(TEST_LSF_BINS)       \
			  $(TEST_GAIN_BINS) $(UFONE_SERVER_BINS)     \
			  $(UFONE_CLIENT_BINS) $(UFONE_PHONE_BINS)   \
			  $(UFONE_ACLIENT_BINS) $(UFONE_ASERVER_BINS)\
                          $(UFONE_ASERVER_ADAPT_BINS)

all: celp ufone

.c.o:
	$(CC) $(CFLAGS) -c $<

libcelp.a: $(LIBCELP_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBCELP_OBJS)

libcelp.so: $(LIBCELP_OBJS)
	$(LD) $(LDFLAGS) -shared -o $@ $(LIBCELP_OBJS)

celp: $(CELP_OBJS) 
	$(LD) $(LDFLAGS) -o $@ $(CELP_OBJS)

ufone: ufone_server ufone_client ufone_phone ufone_aserver ufone_aserver_adapt

ufone_aclient: $(UFONE_ACLIENT_OBJS) 
	$(LD) $(LDFLAGS) -lpthread -lasound -o $@ $(UFONE_ACLIENT_OBJS)

ufone_aserver_adapt: $(UFONE_ASERVER_ADAPT_OBJS) libadapt.a
	$(LD) $(LDFLAGS) -lpthread -lasound -lgsl -lgslcblas -o $@ $(UFONE_ASERVER_ADAPT_OBJS) libadapt.a

ufone_aserver: $(UFONE_ASERVER_OBJS) 
	$(LD) $(LDFLAGS) -lpthread -lasound -o $@ $(UFONE_ASERVER_OBJS)

ufone_client: $(UFONE_CLIENT_OBJS) 
	$(LD) $(LDFLAGS) -lpthread -o $@ $(UFONE_CLIENT_OBJS)

ufone_server: $(UFONE_SERVER_OBJS) 
	$(LD) $(LDFLAGS) -lpthread -o $@ $(UFONE_SERVER_OBJS)

ufone_phone: $(UFONE_PHONE_OBJS) 
	$(LD) $(LDFLAGS) -lpthread -o $@ $(UFONE_PHONE_OBJS)

test_energy: $(TEST_ENERGY_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_ENERGY_OBJS)

test_echo: $(TEST_ECHO_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_ECHO_OBJS)

test_gain: $(TEST_GAIN_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_GAIN_OBJS)

test_lsf: $(TEST_LSF_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_LSF_OBJS)

test_detorres: $(TEST_DETORRES_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_DETORRES_OBJS)

test_torres: $(TEST_TORRES_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_TORRES_OBJS)

test_ccoder: $(TEST_CCODER_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_CCODER_OBJS)

test_cdecoder: $(TEST_CDECODER_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(TEST_CDECODER_OBJS)

clean:
	rm -f $(ALL_OBJS) $(ALL_BINS)
	rm -rf core
