SATEST = SATest -BUILD -cpuarch knc
SRCDIR = $(realpath .)
OUTDIR = $(realpath .)/foo
CFG_FILES := $(wildcard *.cfg.xml)
S_FILES := $(addprefix ${OUTDIR}/,$(notdir ${CFG_FILES:.cfg.xml=.s}))

all: ${S_FILES}

${OUTDIR}/%.s: %.cfg.xml
	mkdir -p $(@D)
	satest_wrap.sh $(SRCDIR)/$< $(OUTDIR)
