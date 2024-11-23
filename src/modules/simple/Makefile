LIB = $(MODULE_OUTD)/dyno_simple.so
SRCD = src

SRC = $(wildcard $(SRCD)/*.c)
OBJ = $(patsubst $(SRCD)/%.c,$(MODULE_OBJD)/%.o,$(SRC))


$(LIB): $(OBJ)
	$(CC) -o $@ -shared -fPIC $^

$(MODULE_OBJD)/%.o: $(SRCD)/%.c
	$(CC) -c -o $@ -fPIC $^
