export SLURP_HOME = .
export UNITY_HOME = $(HOME)/software/unity

CC = gcc
RAGEL = ragel
DOT = dot

INCLUDES = -I$(SLURP_HOME) -I$(UNITY_HOME)/src
DEFAULT_CFLAGS = -g -fpic
DEFAULT_LDFLAGS = -L$(SLURP_HOME)/slurp -lslurp

slurp_template = $(wildcard $(SLURP_HOME)/slurp/*.rl)
slurp_template_src = $(slurp_template:.rl=.c)
slurp_obj = $(slurp_template_src:.c=.o)
slurp_dot = $(slurp_template:.rl=.dot)
slurp_png = $(slurp_dot:.dot=.png)
slurp_lib = slurp/libslurp.so

app_src = $(wildcard $(SLURP_HOME)/app/*.c)
app_obj = $(app_src:.c=.o)
app_app = $(patsubst %.c, %, $(subst _,-,$(app_src)))
#app_app = $(patsubst %.c, %, $(app_src))

test_src = $(wildcard $(SLURP_HOME)/test/*_test.c)
test_runner_src = $(patsubst %_test.c, %_test_runner.c, $(test_src))
test_obj = $(test_src:.c=.o) $(test_runner_src:.c=.o)
test_app = $(patsubst %_test.c, %-test, $(test_src))

unity_src = $(wildcard $(UNITY_HOME)/src/*.c)
unity_obj = $(unity_src:.c=.o)

.PHONY: all slurp apps test-apps graphs
all : slurp apps test-apps graphs

slurp : $(slurp_lib)
apps : $(app_app)
test-apps : $(test_app)
graphs : $(slurp_png)

.SECONDARY:

%.c : %.rl
	$(RAGEL) -o $@ $<

%_test_runner.c : %_test.c
	ruby $(UNITY_HOME)/auto/generate_test_runner.rb $< $@

%.o : %.c
	$(CC) -c $(INCLUDES) $(DEFAULT_CFLAGS) $(CFLAGS) $< -o $@

$(slurp_lib) : $(slurp_obj)
	$(CC) -shared -o $@ $^

app/parse-% : app/parse_%.o $(slurp_lib)
	$(CC) $(DEFAULT_LDFLAGS) $(LDFLAGS) -o $@ $^

%-test : %_test.o %_test_runner.o $(unity_obj) $(slurp_lib)
	$(CC) $(DEFAULT_LDFLAGS) $(LDFLAGS) -o $@ $^

%.dot : %.rl
	$(RAGEL) -Vp -o $@ $<

%.png : %.dot
	$(DOT) -Tpng -o $@ $<

.PHONY: test
test : $(test_app)
	@(for app in $(test_app); do \
		$$app; \
	done)

.PHONY: clean
clean :
	rm -f $(slurp_template_src) $(slurp_obj) $(slurp_dot) $(slurp_png) $(slurp_lib)
	rm -f $(app_obj) $(app_app)
	rm -f $(test_runner_src) $(test_obj) $(test_app)
	rm -f $(unity_obj)
