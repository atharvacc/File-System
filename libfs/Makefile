# Target library
lib := libfs.a
objs := disk.o fs.o

CC = gcc
CFLAGS = -Wall -Werror
CFLAGS += -g


ifneq ($(V), 1)
	Q = @
endif

all: $(lib)

#Dep tracking *must* be below the 'all' rule
deps := $(patsubst %.o,%.d,$(objs))		# will be expanded into other .d's
-include $(deps)											# prefix '-' ignores errors
DEPFLAGS = -MMD -MF $(@:.o=.d)

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)
