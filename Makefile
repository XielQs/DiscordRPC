NAME              = libdiscordrpc
TEST_NAME         = testlib

INC_DIR           = inc
LIB_DIR           = lib
SRC_DIR           = src
TEST_DIR          = test
BUILD_DIR         = build

JANSSON_SRC       = $(LIB_DIR)/jansson
JANSSON_LIB       = $(JANSSON_SRC)/build/lib/libjansson.a

CC                = clang
CFLAGS            = -Wall -Wextra -O3 -std=c11 -MMD -MP -fPIC \
                    -I$(INC_DIR) -I$(JANSSON_SRC)/src
LDFLAGS           = -L$(LIB_DIR) -L$(JANSSON_SRC)/build/src -ljansson
RM                = rm -rf

SRCS              = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS         = $(wildcard $(TEST_DIR)/*.c)
OBJS              = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TEST_OBJS         = $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))
DEPS              = $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

STATIC_LIB        = $(BUILD_DIR)/$(NAME).a
SHARED_LIB        = $(BUILD_DIR)/$(NAME).so

all: lib shared test

$(JANSSON_LIB):
	@echo "Building Jansson..."
	@cd $(JANSSON_SRC) && mkdir -p build && cd build && cmake .. -DJANSSON_BUILD_DOCS=OFF && make
	@echo "Jansson built successfully."

$(STATIC_LIB): $(OBJS)
	@ar rcs $@ $(OBJS)

$(SHARED_LIB): $(OBJS)
	@$(CC) $(CFLAGS) -shared -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(STATIC_LIB) $(JANSSON_LIB)
	@$(CC) $(CFLAGS) $(TEST_OBJS) $(STATIC_LIB) $(JANSSON_LIB) -o $(BUILD_DIR)/$(TEST_NAME)

clean:
	@$(RM) $(BUILD_DIR)
	@$(RM) $(JANSSON_SRC)/build

fclean: clean
	@$(RM) $(STATIC_LIB) $(SHARED_LIB)

re: fclean all

lib: $(STATIC_LIB)
shared: $(SHARED_LIB)

-include $(DEPS)

.PHONY: all clean fclean re test lib shared
