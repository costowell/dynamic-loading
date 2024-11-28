CC = gcc

SRCD = src
MODULED = $(SRCD)/modules
OBJD = obj
OUTD = out
MODULE_OUTD = $(OUTD)/modules

SRC = $(wildcard $(SRCD)/*.c)
OBJ = $(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRC))

EXE = $(OUTD)/dyno
MODULES = simple

CFLAGS = -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wswitch-default -Wswitch-enum -Wconversion -DMODULEDIR=\"$(PWD)/$(MODULE_OUTD)\"
LDFLAGS =


debug: CFLAGS += -g3
debug: all

release: CFLAGS += -O3
release: all

all: $(OUTD) $(OBJD) $(MODULE_OUTD) $(EXE) $(MODULES)

$(MODULES):
	@mkdir -p $(OBJD)/$@
	$(MAKE) -C $(MODULED)/$@ "MODULE_OBJD=$(PWD)/$(OBJD)/$@" "MODULE_OUTD=$(PWD)/$(MODULE_OUTD)"

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(LDFLAGS) $^

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) -c $(CFLAGS) -o $@ $^

$(OBJD) $(OUTD) $(MODULE_OUTD):
	@mkdir -p $@

clean:
	-rm -r $(OUTD) $(OBJD)
