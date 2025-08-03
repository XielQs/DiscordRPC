NAME              = libdiscordrpc
TEST_NAME         = testlib

INC_DIR           = inc
LIB_DIR           = lib
SRC_DIR           = src
TEST_DIR          = test
BUILD_DIR         = build

JANSSON_SRC       = $(LIB_DIR)/jansson
JANSSON_OBJS      = $(JANSSON_SRC)/build/CMakeFiles/jansson.dir/src/*.o

CC                = clang
CFLAGS            = -Wall -Wextra -std=c11 -MMD -MP -fPIC \
                    -I$(INC_DIR) -I$(JANSSON_SRC)/build/include
LDFLAGS           = -L$(LIB_DIR) -L$(JANSSON_SRC)/build/lib
RM                = rm -rf

SRCS              = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS         = $(wildcard $(TEST_DIR)/*.c)
OBJS              = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TEST_OBJS         = $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))
DEPS              = $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

STATIC_LIB        = $(BUILD_DIR)/$(NAME).a
SHARED_LIB        = $(BUILD_DIR)/$(NAME).so

ifeq ($(MAKECMDGOALS), test)
		CFLAGS += -O0 -g3 -ggdb3
else
		CFLAGS += -O3
endif

$(JANSSON_OBJS):
	@echo "Building Jansson objects..."
	@cd $(JANSSON_SRC) && mkdir -p build && cd build && \
		cmake .. -G "Unix Makefiles" -DJANSSON_BUILD_DOCS=OFF -DJANSSON_BUILD_SHARED_LIBS=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && \
		make
	@echo "Jansson objects ready."

$(STATIC_LIB): $(OBJS) $(JANSSON_OBJS)
	@ar rcs $@ $^

$(SHARED_LIB): $(OBJS) $(JANSSON_OBJS)
	@$(CC) $(CFLAGS) -shared -o $@ $(OBJS) $(LDFLAGS) -ljansson

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(STATIC_LIB)
	@$(CC) $(CFLAGS) $(TEST_OBJS) $(STATIC_LIB) -o $(BUILD_DIR)/$(TEST_NAME)

clean:
	@$(RM) $(BUILD_DIR)
	@$(RM) $(JANSSON_SRC)/build

lib: $(STATIC_LIB)
shared: $(SHARED_LIB)

-include $(DEPS)

.PHONY: clean test lib shared
